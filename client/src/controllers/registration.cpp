#include "controllers/registration.hpp"
#include "crypto/crypto.hpp"
#include "crypto/share.hpp"

#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QCoroCore>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/crypto.h>

#include <memory>
#include <stdexcept>

namespace {

using Key = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using Bio = std::unique_ptr<BIO, decltype(&BIO_free)>;

Key generate_key(int type) {
    std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> context(
        EVP_PKEY_CTX_new_id(type, nullptr), &EVP_PKEY_CTX_free
    );
    if (!context || EVP_PKEY_keygen_init(context.get()) != 1) {
        throw std::runtime_error("Could not initialize key generation");
    }
    EVP_PKEY* raw = nullptr;
    if (EVP_PKEY_keygen(context.get(), &raw) != 1) {
        throw std::runtime_error("Could not generate keys");
    }
    return Key(raw, &EVP_PKEY_free);
}

QByteArray pem_for(EVP_PKEY* key, bool private_key) {
    Bio bio(BIO_new(BIO_s_mem()), &BIO_free);
    if (!bio || (private_key
            ? PEM_write_bio_PrivateKey(bio.get(), key, nullptr, nullptr, 0, nullptr, nullptr)
            : PEM_write_bio_PUBKEY(bio.get(), key)) != 1) {
        throw std::runtime_error("Could not encode key as PEM");
    }
    char* data = nullptr;
    const auto length = BIO_get_mem_data(bio.get(), &data);
    if (length <= 0) {
        throw std::runtime_error("Empty PEM key");
    }
    return QByteArray(data, static_cast<qsizetype>(length));
}

std::vector<uint8_t> raw_key(EVP_PKEY* key, bool private_key) {
    size_t size = 0;
    const auto extract = private_key ? EVP_PKEY_get_raw_private_key : EVP_PKEY_get_raw_public_key;
    if (extract(key, nullptr, &size) != 1 || size != 32) {
        throw std::runtime_error("Invalid generated key size");
    }
    std::vector<uint8_t> raw(size);
    if (extract(key, raw.data(), &size) != 1 || size != 32) {
        throw std::runtime_error("Could not extract generated key");
    }
    return raw;
}

QString encoded(std::span<const uint8_t> bytes) {
    return QString::fromLatin1(QByteArray(
        reinterpret_cast<const char*>(bytes.data()), bytes.size()
    ).toBase64());
}

} // namespace

RegistrationController::RegistrationController(AppSettings& settings, QObject* parent)
    : QObject(parent), settings_(settings) {}

QString RegistrationController::beginRegistration() {
    if (registered_ && (!share_saved_ || !private_key_saved_)) {
        return "Save both files before starting another registration";
    }
    try {
        const auto ed_key = generate_key(EVP_PKEY_ED25519);
        const auto x_key = generate_key(EVP_PKEY_X25519);
        const auto ed_public = pem_for(ed_key.get(), false);
        const auto ed_private = pem_for(ed_key.get(), true);
        const auto x_public = pem_for(x_key.get(), false);
        const auto x_private = pem_for(x_key.get(), true);
        auto ed_private_raw = raw_key(ed_key.get(), true);

        const QJsonObject keys{
            {"ed25519_pk", QString::fromUtf8(ed_public)},
            {"ed25519_sk", QString::fromUtf8(ed_private)},
            {"x25519_pk", QString::fromUtf8(x_public)},
            {"x25519_sk", QString::fromUtf8(x_private)}
        };
        const QJsonObject bundle{
            {"format", "truesight-key-bundle"}, {"version", 1}, {"keys", keys}
        };

        ed_public_pem_ = ed_public;
        x_public_pem_ = x_public;
        if (!ed_private_raw_.empty()) {
            OPENSSL_cleanse(ed_private_raw_.data(), ed_private_raw_.size());
        }
        ed_private_raw_ = std::move(ed_private_raw);
        private_bundle_ = QJsonDocument(bundle).toJson(QJsonDocument::Indented);
        share_file_.clear();
        registered_ = false;
        share_saved_ = false;
        private_key_saved_ = false;
        emit downloadsChanged();
        return {};
    } catch (const std::exception& error) {
        return QString::fromUtf8(error.what());
    }
}

void RegistrationController::registerUsername(QString username) {
    username = username.trimmed();
    if (registering_) {
        return;
    }
    if (registered_) {
        emit registrationFailed("This key bundle is already registered");
        return;
    }
    if (private_bundle_.isEmpty() || username.isEmpty() || username.size() > 128) {
        emit registrationFailed("Enter a username (up to 128 characters)");
        return;
    }
    registering_ = true;
    QCoro::connect(registerUsernameAsync(std::move(username)), this, [] {});
}

QCoro::Task<> RegistrationController::registerUsernameAsync(QString username) {
    try {
        const QJsonObject request{
            {"user_iid", username},
            {"ed25519_pk", QString::fromUtf8(ed_public_pem_)}
        };
        const auto response = co_await http_.post(
            settings_.backendUrl() + "/register", request
        );
        if (response.value("status").toString() != "ok") {
            throw std::runtime_error("Registration failed");
        }

        auto public_bytes = [](const QByteArray& pem, int expected_type) {
            Bio bio(BIO_new_mem_buf(pem.constData(), pem.size()), &BIO_free);
            if (!bio) throw std::runtime_error("Could not read public key");
            Key key(PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr), &EVP_PKEY_free);
            if (!key || EVP_PKEY_base_id(key.get()) != expected_type) {
                throw std::runtime_error("Invalid public key type");
            }
            return raw_key(key.get(), false);
        };
        const auto x_public = public_bytes(x_public_pem_, EVP_PKEY_X25519);
        const auto ed_public = public_bytes(ed_public_pem_, EVP_PKEY_ED25519);
        const auto sign_key = [this, &username](const char* type,
                                                std::span<const uint8_t> key) {
            const auto payload = crypto::share::signed_payload(username, type, key);
            return crypto::sign_ed25519(ed_private_raw_, {
                reinterpret_cast<const uint8_t*>(payload.constData()),
                static_cast<size_t>(payload.size())
            });
        };
        const QJsonObject share{
            {"v", 1},
            {"iid", username},
            {"x25519_pk", encoded(x_public)},
            {"x25519_sig", encoded(sign_key("x25519", x_public))},
            {"ed25519_pk", encoded(ed_public)},
            {"ed25519_sig", encoded(sign_key("ed25519", ed_public))}
        };
        share_file_ = QJsonDocument(share).toJson(QJsonDocument::Indented);
        registered_ = true;
        emit registrationSucceeded(username);
    } catch (const std::exception& error) {
        emit registrationFailed(QString::fromUtf8(error.what()));
    }
    registering_ = false;
    co_return;
}

QString RegistrationController::saveFile(
    const QUrl& destination, const QByteArray& content, const QString& suffix
) {
    if (!registered_ || content.isEmpty()) {
        return "Register a username before saving files";
    }
    if (!destination.isLocalFile() ||
        QFileInfo(destination.toLocalFile()).suffix().compare(suffix, Qt::CaseInsensitive) != 0) {
        return "Choose a local ." + suffix + " file";
    }

    QSaveFile file(destination.toLocalFile());
    if (!file.open(QIODevice::WriteOnly)) {
        return file.errorString();
    }
    if (suffix == "tskey" &&
        !file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner)) {
        file.cancelWriting();
        return "Could not restrict private key file permissions";
    }
    if (file.write(content) != content.size() || !file.commit()) {
        return "Could not save file: " + file.errorString();
    }
    return {};
}

QString RegistrationController::saveShare(const QUrl& destination) {
    const auto error = saveFile(destination, share_file_, "share");
    if (error.isEmpty()) {
        share_saved_ = true;
        emit downloadsChanged();
    }
    return error;
}

QString RegistrationController::savePrivateKey(const QUrl& destination) {
    const auto error = saveFile(destination, private_bundle_, "tskey");
    if (error.isEmpty()) {
        private_key_saved_ = true;
        emit downloadsChanged();
    }
    return error;
}

void RegistrationController::finishRegistration() {
    if (!share_saved_ || !private_key_saved_) {
        return;
    }
    ed_public_pem_.clear();
    x_public_pem_.clear();
    if (!ed_private_raw_.empty()) {
        OPENSSL_cleanse(ed_private_raw_.data(), ed_private_raw_.size());
        ed_private_raw_.clear();
    }
    private_bundle_.fill('\0');
    private_bundle_.clear();
    share_file_.clear();
}
