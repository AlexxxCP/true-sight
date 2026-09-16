#include "crypto/crypto.hpp"

#include <filesystem>
#include <memory>
#include <stdexcept>

#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/core_names.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>

namespace crypto {

namespace {

using PKeyPtr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using MdContextPtr = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;

std::vector<unsigned char> sign_ed25519_with_key(
    EVP_PKEY* key,
    std::span<const unsigned char> msg
) {
    MdContextPtr ctx(EVP_MD_CTX_new(), &EVP_MD_CTX_free);
    if (!ctx) {
        throw std::runtime_error("EVP_MD_CTX_new failed");
    }

    if (EVP_DigestSignInit(
        ctx.get(),
        nullptr,
        nullptr,
        nullptr,
        key
    ) != 1) {
        throw std::runtime_error("EVP_DigestSignInit failed");
    }

    size_t signature_size = 0;

    if (EVP_DigestSign(
        ctx.get(),
        nullptr,
        &signature_size,
        msg.data(),
        msg.size()
    ) != 1) {
        throw std::runtime_error("failed to get signature size");
    }

    std::vector<unsigned char> signature(signature_size);

    if (EVP_DigestSign(
        ctx.get(),
        signature.data(),
        &signature_size,
        msg.data(),
        msg.size()
    ) != 1) {
        throw std::runtime_error("signing failed");
    }

    signature.resize(signature_size);
    return signature;
}

} // namespace

std::string base64url_encode(std::span<const unsigned char> input) {
    size_t encoded_size = 4 * ((input.size() + 2) / 3);

    std::string result(encoded_size + 1, '\0');

    int size = EVP_EncodeBlock(
        reinterpret_cast<unsigned char*>(result.data()),
        input.data(),
        static_cast<int>(input.size())
    );

    if (size < 0) {
        throw std::runtime_error("EVP_EncodeBlock failed");
    }

    result.resize(static_cast<size_t>(size));

    std::replace(result.begin(), result.end(), '+', '-');
    std::replace(result.begin(), result.end(), '/', '_');

    while (!result.empty() && result.back() == '=') {
        result.pop_back();
    }

    return result;
}

std::vector<unsigned char> sign_ed25519(
    const std::vector<uint8_t>& private_key,
    std::span<const unsigned char> msg
) {
    constexpr size_t ed25519_private_key_size = 32;

    if (private_key.size() != ed25519_private_key_size) {
        throw std::invalid_argument(
            "Ed25519 private key must be exactly 32 bytes"
        );
    }

    PKeyPtr key(
        EVP_PKEY_new_raw_private_key(
            EVP_PKEY_ED25519,
            nullptr,
            private_key.data(),
            private_key.size()
        ),
        &EVP_PKEY_free
    );

    if (!key) {
        throw std::runtime_error("failed to create Ed25519 private key");
    }

    return sign_ed25519_with_key(key.get(), msg);
}

std::array<uint8_t, 32> x25519_shared_secret(
    std::span<const uint8_t, 32> sk,
    std::span<const uint8_t, 32> pk
) {
    EVP_PKEY* private_key = EVP_PKEY_new_raw_private_key(
        EVP_PKEY_X25519,
        nullptr,
        sk.data(),
        sk.size()
    );

    if (!private_key) {
        throw std::runtime_error("failed to create X25519 private key");
    }

    EVP_PKEY* public_key = EVP_PKEY_new_raw_public_key(
        EVP_PKEY_X25519,
        nullptr,
        pk.data(),
        pk.size()
    );

    if (!public_key) {
        throw std::runtime_error("failed to create X25519 public key");
    }

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(private_key, nullptr);

    if (!ctx) {
        EVP_PKEY_free(private_key);
        EVP_PKEY_free(public_key);
        throw std::runtime_error("failed to create X25519 context");
    }

    if (EVP_PKEY_derive_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(public_key);
        EVP_PKEY_free(private_key);
        throw std::runtime_error("EVP_PKEY_derive_init failed");
    }

    if (EVP_PKEY_derive_set_peer(ctx, public_key) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(public_key);
        EVP_PKEY_free(private_key);
        throw std::runtime_error("EVP_PKEY_derive_set_peer failed");
    }

    std::array<uint8_t, 32> secret;
    size_t secret_size = secret.size();

     if (EVP_PKEY_derive(
        ctx,
        secret.data(),
        &secret_size
    ) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(public_key);
        EVP_PKEY_free(private_key);
        throw std::runtime_error("EVP_PKEY_derive failed");
    }

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(public_key);
    EVP_PKEY_free(private_key);

    if (secret_size != secret.size()) {
        throw std::runtime_error("unexpected X25519 shared secret size");
    }

    return secret;
}

std::array<uint8_t, 32> hkdf_sha256(
    std::span<const uint8_t> ikm,
    std::span<const uint8_t> salt,
    std::span<const uint8_t> info
) {
    EVP_KDF* kdf = EVP_KDF_fetch(nullptr, "HKDF", nullptr);

    if (!kdf) {
        throw std::runtime_error("EVP_KDF_fetch failed");
    }

    EVP_KDF_CTX* ctx = EVP_KDF_CTX_new(kdf);
    EVP_KDF_free(kdf);

    if (!ctx) {
        throw std::runtime_error("EVP_KDF_CTX_new failed");
    }

    OSSL_PARAM params[5];

    params[0] = OSSL_PARAM_construct_utf8_string(
        OSSL_KDF_PARAM_DIGEST,
        const_cast<char*>("SHA256"),
        0
    );

    params[1] = OSSL_PARAM_construct_octet_string(
        OSSL_KDF_PARAM_KEY,
        const_cast<uint8_t*>(ikm.data()),
        ikm.size()
    );

    params[2] = OSSL_PARAM_construct_octet_string(
        OSSL_KDF_PARAM_SALT,
        const_cast<uint8_t*>(salt.data()),
        salt.size()
    );

    params[3] = OSSL_PARAM_construct_octet_string(
        OSSL_KDF_PARAM_INFO,
        const_cast<uint8_t*>(info.data()),
        info.size()
    );

    params[4] = OSSL_PARAM_construct_end();

    std::array<uint8_t, 32> out;

    if (EVP_KDF_derive(
        ctx,
        out.data(),
        out.size(),
        params
    ) <= 0) {
        EVP_KDF_CTX_free(ctx);
        throw std::runtime_error("EVP_KDF_derive failed");
    }

    EVP_KDF_CTX_free(ctx);

    return out;
}

EncryptedMessage aes_256_gcm_siv_encrypt(
    std::span<const u8, 32> key,
    std::span<const u8> plaintext,
    std::span<const u8> aad
) {
    EncryptedMessage result;
    result.ciphertext.resize(plaintext.size());

    if (RAND_bytes(result.nonce.data(), result.nonce.size()) != 1) {
        throw std::runtime_error("RAND_bytes failed");
    }

    EVP_CIPHER* cipher = EVP_CIPHER_fetch(
        nullptr,
        "AES-256-GCM-SIV",
        nullptr
    );

    if (!cipher) {
        throw std::runtime_error("EVP_CIPHER_fetch failed");
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (!ctx) {
        EVP_CIPHER_free(cipher);
        throw std::runtime_error("EVP_CIPHER_CTX_new failed");
    }

    if (EVP_EncryptInit_ex2(
        ctx,
        cipher,
        key.data(),
        result.nonce.data(),
        nullptr
    ) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        EVP_CIPHER_free(cipher);
        throw std::runtime_error("EVP_EncryptInit_ex2 failed");
    }

    int len = 0;

    if (!aad.empty()) {
        if (EVP_EncryptUpdate(
            ctx,
            nullptr,
            &len,
            aad.data(),
            static_cast<int>(aad.size())
        ) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            EVP_CIPHER_free(cipher);
            throw std::runtime_error("failed to process AAD");
        }
    }

    if (EVP_EncryptUpdate(
        ctx,
        result.ciphertext.data(),
        &len,
        plaintext.data(),
        static_cast<int>(plaintext.size())
    ) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        EVP_CIPHER_free(cipher);
        throw std::runtime_error("EVP_EncryptUpdate failed");
    }

    result.ciphertext.resize(len);

    int final_len = 0;

    if (EVP_EncryptFinal_ex(
        ctx,
        result.ciphertext.data() + result.ciphertext.size(),
        &final_len
    ) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        EVP_CIPHER_free(cipher);
        throw std::runtime_error("EVP_EncryptFinal_ex failed");
    }

    if (EVP_CIPHER_CTX_ctrl(
        ctx,
        EVP_CTRL_AEAD_GET_TAG,
        result.tag.size(),
        result.tag.data()
    ) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        EVP_CIPHER_free(cipher);
        throw std::runtime_error("failed to get authentication tag");
    }

    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_free(cipher);

    return result;
}

std::optional<std::vector<u8>> aes_256_gcm_siv_decrypt(
    std::span<const u8, 32> key,
    std::span<const u8, 12> nonce,
    std::span<const u8> ciphertext,
    std::span<const u8, 16> tag,
    std::span<const u8> aad
) {
    EVP_CIPHER* cipher = EVP_CIPHER_fetch(
        nullptr,
        "AES-256-GCM-SIV",
        nullptr
    );

    if (!cipher) {
        throw std::runtime_error("EVP_CIPHER_fetch failed");
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (!ctx) {
        EVP_CIPHER_free(cipher);
        throw std::runtime_error("EVP_CIPHER_CTX_new failed");
    }

    if (EVP_DecryptInit_ex2(
        ctx,
        cipher,
        key.data(),
        nonce.data(),
        nullptr
    ) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        EVP_CIPHER_free(cipher);
        throw std::runtime_error("EVP_DecryptInit_ex2 failed");
    }

    if (EVP_CIPHER_CTX_ctrl(
        ctx,
        EVP_CTRL_AEAD_SET_TAG,
        tag.size(),
        const_cast<u8*>(tag.data())
    ) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        EVP_CIPHER_free(cipher);
        throw std::runtime_error("failed to set authentication tag");
    }

    int len = 0;

    if (!aad.empty()) {
        if (EVP_DecryptUpdate(
            ctx,
            nullptr,
            &len,
            aad.data(),
            static_cast<int>(aad.size())
        ) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            EVP_CIPHER_free(cipher);
            throw std::runtime_error("failed to process AAD");
        }
    }

    std::vector<u8> plaintext(ciphertext.size());

    if (EVP_DecryptUpdate(
        ctx,
        plaintext.data(),
        &len,
        ciphertext.data(),
        static_cast<int>(ciphertext.size())
    ) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        EVP_CIPHER_free(cipher);
        return std::nullopt;
    }

    int plaintext_size = len;
    int final_len = 0;

    int ok = EVP_DecryptFinal_ex(
        ctx,
        plaintext.data() + plaintext_size,
        &final_len
    );

    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_free(cipher);

    if (ok != 1) {
        return std::nullopt;
    }

    plaintext.resize(
        static_cast<size_t>(plaintext_size + final_len)
    );

    return plaintext;
}

};
