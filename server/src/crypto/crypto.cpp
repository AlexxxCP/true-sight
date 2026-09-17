#include "crypto/crypto.hpp"

#include "shared.hpp"

#include <openssl/core_names.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/pem.h>

namespace crypto {

std::string base64url_encode(const std::string& input) {
    size_t encoded_size = 4 * ((input.size() + 2) / 3);

    // EVP_EncodeBlock also writes a terminating NUL.
    std::string result(encoded_size + 1, '\0');

    int size = EVP_EncodeBlock(
        reinterpret_cast<unsigned char*>(result.data()),
        reinterpret_cast<const unsigned char*>(input.data()),
        static_cast<int>(input.size())
    );

    result.resize(size);

    std::replace(result.begin(), result.end(), '+', '-');
    std::replace(result.begin(), result.end(), '/', '_');

    while (!result.empty() && result.back() == '=') {
        result.pop_back();
    }

    return result;
}

std::vector<u8> base64url_decode(std::string_view input) {
    std::string base64(input);

    std::replace(base64.begin(), base64.end(), '-', '+');
    std::replace(base64.begin(), base64.end(), '_', '/');

    while (base64.size() % 4 != 0) {
        base64.push_back('=');
    }

    std::vector<u8> result(
        3 * (base64.size() / 4)
    );

    int size = EVP_DecodeBlock(
        result.data(),
        reinterpret_cast<const unsigned char*>(base64.data()),
        static_cast<int>(base64.size())
    );

    if (size < 0) {
        return {};
    }

    if (!base64.empty() && base64.back() == '=') {
        --size;
    }

    if (base64.size() >= 2 && base64[base64.size() - 2] == '=') {
        --size;
    }

    result.resize(size);

    return result;
}

std::vector<unsigned char> hmac_sha256(
    const std::string& secret,
    const std::string& message
) {
    EVP_MAC* mac = EVP_MAC_fetch(nullptr, "HMAC", nullptr);

    if (mac == nullptr) {
        throw std::runtime_error("Failed to fetch HMAC");
    }

    EVP_MAC_CTX* ctx = EVP_MAC_CTX_new(mac);

    if (ctx == nullptr) {
        EVP_MAC_free(mac);
        throw std::runtime_error("Failed to create HMAC context");
    }

    char digest[] = "SHA256";

    OSSL_PARAM params[] = {
        OSSL_PARAM_construct_utf8_string(
            OSSL_MAC_PARAM_DIGEST,
            digest,
            0
        ),
        OSSL_PARAM_construct_end()
    };

    if (EVP_MAC_init(
        ctx,
        reinterpret_cast<const unsigned char*>(secret.data()),
        secret.size(),
        params
    ) != 1) {
        EVP_MAC_CTX_free(ctx);
        EVP_MAC_free(mac);
        throw std::runtime_error("Failed to initialize HMAC");
    }

    if (EVP_MAC_update(
        ctx,
        reinterpret_cast<const unsigned char*>(message.data()),
        message.size()
    ) != 1) {
        EVP_MAC_CTX_free(ctx);
        EVP_MAC_free(mac);
        throw std::runtime_error("Failed to update HMAC");
    }

    std::vector<unsigned char> result(32);
    size_t result_size = 0;

    if (EVP_MAC_final(
        ctx,
        result.data(),
        &result_size,
        result.size()
    ) != 1) {
        EVP_MAC_CTX_free(ctx);
        EVP_MAC_free(mac);
        throw std::runtime_error("Failed to finalize HMAC");
    }

    result.resize(result_size);

    EVP_MAC_CTX_free(ctx);
    EVP_MAC_free(mac);

    return result;
}

std::vector<u8> rsa_encrypt(
    std::span<const u8> plaintext,
    EVP_PKEY* public_key
) {
    EVP_PKEY_CTX* ctx =
        EVP_PKEY_CTX_new_from_pkey(nullptr, public_key, nullptr);

    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        throw std::runtime_error("EVP_PKEY_CTX_new_from_pkey failed");
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        throw std::runtime_error("EVP_PKEY_encrypt_init failed");
    }

    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0) {
       throw std::runtime_error("Failed to set RSA OAEP padding");
    }

    if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) <= 0) {
        throw std::runtime_error("Failed to set RSA OAEP digest");
    }

    std::size_t ciphertext_size = 0;

    if (EVP_PKEY_encrypt(
        ctx,
        nullptr,
        &ciphertext_size,
        plaintext.data(),
        plaintext.size()
    ) <= 0) {
       throw std::runtime_error("Failed to determine RSA ciphertext size");
    }

    std::vector<unsigned char> ciphertext(ciphertext_size);
    if (EVP_PKEY_encrypt(
        ctx,
        ciphertext.data(),
        &ciphertext_size,
        plaintext.data(),
        plaintext.size()
    ) <= 0) {
        throw std::runtime_error("RSA encryption failed");
    }

    ciphertext.resize(ciphertext_size);

    return ciphertext;
}

std::string random_hex(std::size_t bytes) {
    std::string buf(bytes, '\0');

    if (RAND_bytes(
        reinterpret_cast<unsigned char*>(buf.data()),
        static_cast<int>(bytes)
    ) != 1) {
        throw std::runtime_error("RAND_bytes failed");
    }

    static constexpr char hex[] = "0123456789abcdef";

    std::string out;
    out.resize(bytes * 2);

    for (std::size_t i = 0; i < bytes; ++i) {
        unsigned char x = static_cast<unsigned char>(buf[i]);
        out[2 * i] = hex[x >> 4];
        out[2 * i + 1] = hex[x & 0x0f];
    }

    return out;
}


PKeyPtr load_public_key(std::string_view pem) {
    BIO* bio = BIO_new_mem_buf(pem.data(), pem.size());

    if (!bio) {
        return {nullptr, EVP_PKEY_free};
    }

    EVP_PKEY* key = PEM_read_bio_PUBKEY(
        bio,
        nullptr,
        nullptr,
        nullptr
    );

    BIO_free(bio);

    return {key, EVP_PKEY_free};
}


bool ed25519_verify(
    std::span<const std::uint8_t> message,
    std::span<const std::uint8_t> signature,
    EVP_PKEY *public_key
) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();

    if (!ctx) {
        return false;
    }

    if (EVP_DigestVerifyInit(
        ctx, nullptr, nullptr, nullptr, public_key
    ) != 1) {
        EVP_MD_CTX_free(ctx);
        return false;
    }

    int result = EVP_DigestVerify(
        ctx,
        signature.data(),
        signature.size(),
        message.data(),
        message.size()
    );

    EVP_MD_CTX_free(ctx);

    return result == 1;
}

}
