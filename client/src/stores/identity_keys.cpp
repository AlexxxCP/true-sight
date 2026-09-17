#include "stores/identity_keys.hpp"
#include "crypto/crypto.hpp"
#include "crypto/share.hpp"

#include <QFile>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QSettings>
#include <QCryptographicHash>

#include <openssl/evp.h>
#include <openssl/pem.h>

namespace {

std::vector<uint8_t> parse_share_field(const QJsonObject& object,
                                       const char* field, int expected_size) {
  const auto value = object.value(field);
  if (!value.isString() || value.toString().isEmpty()) {
      throw std::runtime_error(std::string("Missing share field: ") + field);
  }
  const auto decoded = QByteArray::fromBase64Encoding(
      value.toString().toLatin1(),
      QByteArray::Base64Encoding | QByteArray::AbortOnBase64DecodingErrors
  );
  if (!decoded || decoded.decoded.size() != expected_size ||
      decoded.decoded.toBase64() != value.toString().toLatin1()) {
      throw std::runtime_error(std::string("Invalid share field: ") + field);
  }
  return {decoded.decoded.begin(), decoded.decoded.end()};
}

} // namespace

QString IdentityKeys::peers_settings_key() const {
  QByteArray identity = QByteArray::fromStdString(current_username_);
  identity.append('\0');
  identity.append(reinterpret_cast<const char*>(ed25519_pk_.data()), ed25519_pk_.size());
  return "trusted_peers/" + QString::fromLatin1(
      QCryptographicHash::hash(identity, QCryptographicHash::Sha256).toHex()
  );
}

void IdentityKeys::save_peers() const {
  QJsonArray rows;
  for (const auto& [user, keys] : other_users_) {
      const QByteArray signing(
          reinterpret_cast<const char*>(keys.ed25519_pk.data()), keys.ed25519_pk.size()
      );
      const QByteArray exchange(
          reinterpret_cast<const char*>(keys.x25519_pk.data()), keys.x25519_pk.size()
      );
      rows.append(QJsonObject{
          {"user_iid", QString::fromStdString(user)},
          {"ed25519_pk", QString::fromLatin1(signing.toBase64(QByteArray::Base64UrlEncoding))},
          {"x25519_pk", QString::fromLatin1(exchange.toBase64(QByteArray::Base64UrlEncoding))}
      });
  }
  QSettings settings{"TrueSight", "TrueSightClient"};
  settings.setValue(peers_settings_key(), QJsonDocument(rows).toJson(QJsonDocument::Compact));
  settings.sync();
  if (settings.status() != QSettings::NoError) {
      throw std::runtime_error("Could not save peer public keys");
  }
}

void IdentityKeys::load_saved_peers() {
  QSettings settings{"TrueSight", "TrueSightClient"};
  const QByteArray saved = settings.value(peers_settings_key()).toByteArray();
  if (saved.isEmpty()) {
      return;
  }
  QJsonParseError error;
  const auto document = QJsonDocument::fromJson(saved, &error);
  if (error.error != QJsonParseError::NoError || !document.isArray()) {
      throw std::runtime_error("Saved peer keys are invalid");
  }
  std::unordered_map<std::string, Keys> restored;
  for (const auto& value : document.array()) {
      if (!value.isObject()) {
          throw std::runtime_error("Saved peer keys are invalid");
      }
      const auto row = value.toObject();
      const auto user = row.value("user_iid");
      const auto signing = row.value("ed25519_pk");
      const auto exchange = row.value("x25519_pk");
      if (!user.isString() || user.toString().isEmpty() ||
          !signing.isString() || !exchange.isString()) {
          throw std::runtime_error("Saved peer keys are invalid");
      }
      const auto signing_bytes = QByteArray::fromBase64Encoding(
          signing.toString().toLatin1(),
          QByteArray::Base64UrlEncoding | QByteArray::AbortOnBase64DecodingErrors
      );
      const auto exchange_bytes = QByteArray::fromBase64Encoding(
          exchange.toString().toLatin1(),
          QByteArray::Base64UrlEncoding | QByteArray::AbortOnBase64DecodingErrors
      );
      if (!signing_bytes || !exchange_bytes ||
          signing_bytes.decoded.size() != 32 || exchange_bytes.decoded.size() != 32) {
          throw std::runtime_error("Saved peer keys are invalid");
      }
      Keys keys{
          .ed25519_pk = {signing_bytes.decoded.begin(), signing_bytes.decoded.end()},
          .x25519_pk = {exchange_bytes.decoded.begin(), exchange_bytes.decoded.end()}
      };
      if (!restored.emplace(user.toString().toStdString(), std::move(keys)).second) {
          throw std::runtime_error("Duplicate saved peer key");
      }
  }
  other_users_ = std::move(restored);
}

void IdentityKeys::forget_saved_peers() {
  QSettings settings{"TrueSight", "TrueSightClient"};
  settings.remove(peers_settings_key());
  settings.sync();
  if (settings.status() != QSettings::NoError) {
      throw std::runtime_error("Could not clear saved peer keys");
  }
  other_users_.clear();
}

std::vector<std::string> IdentityKeys::peer_usernames() const {
  std::vector<std::string> users;
  users.reserve(other_users_.size());
  for (const auto& [user, keys] : other_users_) {
      users.push_back(user);
  }
  return users;
}

std::string IdentityKeys::import_share(const QUrl& url)
{
  if (!url.isLocalFile()) {
      throw std::runtime_error("Select a local .share file");
  }
  const QString path = url.toLocalFile();
  if (QFileInfo(path).suffix().compare("share", Qt::CaseInsensitive) != 0) {
      throw std::runtime_error("Only .share files are accepted");
  }

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
      throw std::runtime_error(("Cannot open share file: " + file.errorString()).toStdString());
  }
  if (file.size() > 64 * 1024) {
      throw std::runtime_error("Share file is unexpectedly large");
  }

  QJsonParseError parse_error;
  const auto document = QJsonDocument::fromJson(file.readAll(), &parse_error);
  if (parse_error.error != QJsonParseError::NoError || !document.isObject()) {
      throw std::runtime_error("Invalid share file JSON");
  }
  const auto object = document.object();
  if (object.value("v").toInt() != 1) {
      throw std::runtime_error("Unsupported share file format or version");
  }
  const auto user_value = object.value("iid");
  if (!user_value.isString() || user_value.toString().trimmed().isEmpty()) {
      throw std::runtime_error("Share file has no user ID");
  }
  const std::string user = user_value.toString().toStdString();
  if (user == current_username_) {
      throw std::runtime_error("Cannot add your own key as a conversation");
  }

  Keys imported{
      .ed25519_pk = parse_share_field(object, "ed25519_pk", 32),
      .x25519_pk = parse_share_field(object, "x25519_pk", 32)
  };
  const auto ed_signature = parse_share_field(object, "ed25519_sig", 64);
  const auto x_signature = parse_share_field(object, "x25519_sig", 64);
  const auto verify_key = [&user_value, &imported](const char* type,
                                                   const std::vector<uint8_t>& key,
                                                   const std::vector<uint8_t>& signature) {
      const auto payload = crypto::share::signed_payload(user_value.toString(), type, key);
      if (!crypto::verify_ed25519(imported.ed25519_pk,
              {reinterpret_cast<const uint8_t*>(payload.constData()),
               static_cast<size_t>(payload.size())}, signature)) {
          throw std::runtime_error(std::string("Invalid share signature: ") + type);
      }
  };
  verify_key("ed25519", imported.ed25519_pk, ed_signature);
  verify_key("x25519", imported.x25519_pk, x_signature);

  const auto existing = other_users_.find(user);
  if (existing != other_users_.end()) {
      if (existing->second.ed25519_pk != imported.ed25519_pk ||
          existing->second.x25519_pk != imported.x25519_pk) {
          throw std::runtime_error("Public keys for this user differ from the imported keys");
      }
      save_peers();
      return user;
  }

  other_users_.emplace(user, std::move(imported));
  save_peers();
  return user;
}

void IdentityKeys::load_from_bundle(const QUrl& url)
{
  const QString path = url.isLocalFile()
      ? url.toLocalFile()
      : url.toString();

  QFile file(path);

  if (!file.open(QIODevice::ReadOnly)) {
      throw std::runtime_error(
          ("Cannot open key bundle: " + file.errorString()).toStdString()
      );
  }

  if (file.size() > 64 * 1024) {
      throw std::runtime_error("Key bundle is unexpectedly large");
  }

  QJsonParseError parse_error;
  const QJsonDocument document =
      QJsonDocument::fromJson(file.readAll(), &parse_error);

  if (parse_error.error != QJsonParseError::NoError) {
      throw std::runtime_error(
          ("Invalid key bundle JSON: " + parse_error.errorString())
              .toStdString()
      );
  }

  if (!document.isObject()) {
      throw std::runtime_error("Key bundle root must be an object");
  }

  const QJsonObject root = document.object();

  if (root.value("format").toString() != "truesight-key-bundle") {
      throw std::runtime_error("Unsupported key bundle format");
  }

  if (root.value("version").toInt() != 1) {
      throw std::runtime_error("Unsupported key bundle version");
  }

  if (!root.value("keys").isObject()) {
      throw std::runtime_error("Key bundle has no keys object");
  }

  const QJsonObject keys = root.value("keys").toObject();

  auto pem_value = [&keys](const char* name) {
      const QJsonValue value = keys.value(name);

      if (!value.isString() || value.toString().isEmpty()) {
          throw std::runtime_error(
              std::string("Missing key: ") + name
          );
      }

      return value.toString().toUtf8();
  };

  using PKey = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;

  auto parse_pem = [](
      const QByteArray& pem,
      bool public_key,
      int expected_type,
      const char* name
  ) -> PKey {
      std::unique_ptr<BIO, decltype(&BIO_free)> bio(
          BIO_new_mem_buf(pem.constData(), pem.size()),
          &BIO_free
      );

      if (!bio) {
          throw std::runtime_error("BIO_new_mem_buf failed");
      }

      EVP_PKEY* raw_key = public_key
          ? PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr)
          : PEM_read_bio_PrivateKey(bio.get(), nullptr, nullptr, nullptr);

      PKey key(raw_key, &EVP_PKEY_free);

      if (!key) {
          throw std::runtime_error(
              std::string("Invalid PEM key: ") + name
          );
      }

      if (EVP_PKEY_base_id(key.get()) != expected_type) {
          throw std::runtime_error(
              std::string("Wrong key type for: ") + name
          );
      }

      return key;
  };

  auto raw_public = [](EVP_PKEY* key) {
      size_t size = 0;

      if (EVP_PKEY_get_raw_public_key(key, nullptr, &size) != 1 ||
          size != 32) {
          throw std::runtime_error("Invalid public key size");
      }

      std::vector<uint8_t> result(size);

      if (EVP_PKEY_get_raw_public_key(
              key,
              result.data(),
              &size
          ) != 1) {
          throw std::runtime_error("Cannot extract public key");
      }

      result.resize(size);
      return result;
  };

  auto raw_private = [](EVP_PKEY* key) {
      size_t size = 0;

      if (EVP_PKEY_get_raw_private_key(key, nullptr, &size) != 1 ||
          size != 32) {
          throw std::runtime_error("Invalid private key size");
      }

      std::vector<uint8_t> result(size);

      if (EVP_PKEY_get_raw_private_key(
              key,
              result.data(),
              &size
          ) != 1) {
          throw std::runtime_error("Cannot extract private key");
      }

      result.resize(size);
      return result;
  };

  const auto ed_public_key = parse_pem(
      pem_value("ed25519_pk"),
      true,
      EVP_PKEY_ED25519,
      "ed25519_pk"
  );

  const auto ed_private_key = parse_pem(
      pem_value("ed25519_sk"),
      false,
      EVP_PKEY_ED25519,
      "ed25519_sk"
  );

  const auto x_public_key = parse_pem(
      pem_value("x25519_pk"),
      true,
      EVP_PKEY_X25519,
      "x25519_pk"
  );

  const auto x_private_key = parse_pem(
      pem_value("x25519_sk"),
      false,
      EVP_PKEY_X25519,
      "x25519_sk"
  );

  auto ed_public = raw_public(ed_public_key.get());
  auto ed_derived_public = raw_public(ed_private_key.get());
  auto ed_private = raw_private(ed_private_key.get());

  auto x_public = raw_public(x_public_key.get());
  auto x_derived_public = raw_public(x_private_key.get());
  auto x_private = raw_private(x_private_key.get());

  if (CRYPTO_memcmp(
          ed_public.data(),
          ed_derived_public.data(),
          ed_public.size()
      ) != 0) {
      OPENSSL_cleanse(ed_private.data(), ed_private.size());
      OPENSSL_cleanse(x_private.data(), x_private.size());
      throw std::runtime_error("Ed25519 keypair does not match");
  }

  if (CRYPTO_memcmp(
          x_public.data(),
          x_derived_public.data(),
          x_public.size()
      ) != 0) {
      OPENSSL_cleanse(ed_private.data(), ed_private.size());
      OPENSSL_cleanse(x_private.data(), x_private.size());
      throw std::runtime_error("X25519 keypair does not match");
  }

  clear();

  ed25519_pk_ = std::move(ed_public);
  ed25519_sk_ = std::move(ed_private);
  x25519_pk_ = std::move(x_public);
  x25519_sk_ = std::move(x_private);
}
