#include "stores/identity_keys.hpp"

#include <QFile>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>

#include <openssl/evp.h>
#include <openssl/pem.h>

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

