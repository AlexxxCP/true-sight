#include "controllers/auth.hpp"
#include "crypto/crypto.hpp"

#include <QJsonObject>

QCoro::Task<> AuthController::authAsync(
    QString username,
    std::vector<uint8_t> ed25519_sk,
    QUrl private_key_path,
    std::uint64_t generation
) {
    try {
        QJsonObject get_challenge_body{
            {"user_iid", username}
        };

        auto json = co_await http_.post(
            app_settings_.backendUrl() + "/get-challenge",
            get_challenge_body
        );
        if (generation != generation_) {
            co_return;
        }

        QString challenge = json["challenge"].toString();
        QString challege_id = json["challenge_id"].toString();

        QByteArray challenge_bytes = challenge.toUtf8();

        std::span<const unsigned char> challenge_span{
            reinterpret_cast<const unsigned char*>(challenge_bytes.constData()),
            static_cast<size_t>(challenge_bytes.size())
        };

        auto signature
            = crypto::sign_ed25519(ed25519_sk, challenge_span);

        auto encoded_sig
            = crypto::base64url_encode(signature);

        QJsonObject answer_challenge_body {
            {"challenge_id", challege_id},
            {"signed_challenge", QString::fromStdString(encoded_sig)}
        };

        auto response = co_await http_.post(
            app_settings_.backendUrl() +  "/validate-challenge",
            answer_challenge_body
        );
        if (generation != generation_) {
            co_return;
        }

        const QString token = response.value("access_token").toString();
        if (token.isEmpty()) {
            throw std::runtime_error("Server returned no access token");
        }
        identity_keys_.load_saved_peers();
        app_settings_.saveLogin(username, private_key_path);
        app_settings_.setUsername(username);
        app_settings_.setAccessToken(token);

        emit authFinished(true);
    } catch (const std::exception& e) {
        if (generation == generation_) {
            identity_keys_.clear();
            app_settings_.setAccessToken({});
            app_settings_.setUsername({});
            emit authFailed(QString::fromUtf8(e.what()));
        }
    }
    co_return;
};

void AuthController::auth(
    QString username,
    QUrl private_key_path
) {
    const auto generation = ++generation_;
    try {
        if (username.trimmed().isEmpty() || !private_key_path.isLocalFile()) {
            throw std::runtime_error("Select a username and local .tskey file");
        }
        identity_keys_.load_from_bundle(private_key_path);
        identity_keys_.set_current_username(username.toStdString());

        QCoro::connect(
            authAsync(username, identity_keys_.ed25519_sk(), private_key_path, generation),
            this,
            []{}
        );
    } catch (const std::exception& error) {
        identity_keys_.clear();
        emit authFailed(QString::fromUtf8(error.what()));
    }
};

void AuthController::autoLogin() {
    const QString username = app_settings_.savedUsername();
    const QUrl path = app_settings_.savedPrivateKeyPath();
    if (!username.isEmpty()) {
        auth(username, path);
    }
}

QString AuthController::logout() {
    ++generation_;
    QString error;
    try {
        identity_keys_.forget_saved_peers();
    } catch (const std::exception& e) {
        error = QString::fromUtf8(e.what());
    }
    try {
        app_settings_.clearSavedLogin();
    } catch (const std::exception& e) {
        if (error.isEmpty()) {
            error = QString::fromUtf8(e.what());
        }
    }
    identity_keys_.clear();
    app_settings_.setAccessToken({});
    app_settings_.setUsername({});
    emit loggedOut();
    return error;
}
