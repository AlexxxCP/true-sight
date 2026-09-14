#include "crypto/crypto.hpp"

#include <filesystem>
#include <stdexcept>

#include <openssl/pem.h>

namespace crypto {

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
    const std::filesystem::path& private_key_path,
    std::span<const unsigned char> msg
) {
    FILE* file = std::fopen(private_key_path.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("failed to open private key");
    }

    EVP_PKEY* key = PEM_read_PrivateKey(
        file,
        nullptr,
        nullptr,
        nullptr
    );

    std::fclose(file);

    if (!key) {
        throw std::runtime_error("failed to read private key");
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        EVP_PKEY_free(key);
        throw std::runtime_error("EVP_MD_CTX_new failed");
    }

    if (EVP_DigestSignInit(
        ctx,
        nullptr,
        nullptr,
        nullptr,
        key
    ) != 1) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(key);
        throw std::runtime_error("EVP_DigestSignInit failed");
    }

    size_t signature_size = 0;

    if (EVP_DigestSign(
        ctx,
        nullptr,
        &signature_size,
        msg.data(),
        msg.size()
    ) <= 0) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(key);
        throw std::runtime_error("failed to get signature size");
    }

    std::vector<unsigned char> signature(signature_size);

    if (EVP_DigestSign(
        ctx,
        signature.data(),
        &signature_size,
        msg.data(),
        msg.size()
    ) != 1) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(key);
        throw std::runtime_error("signing failed");
    }

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(key);

    signature.resize(signature_size);
    return signature;
}

};
