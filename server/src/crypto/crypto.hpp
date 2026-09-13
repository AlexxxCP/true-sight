#pragma once

#include <string>
#include <vector>
#include <span>
#include <openssl/evp.h>

#include "shared.hpp"

namespace crypto {
using PKeyPtr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;

std::string base64url_encode(const std::string& input);
std::vector<u8> base64url_decode(std::string_view input);

std::vector<unsigned char> hmac_sha256(
    const std::string& secret,
    const std::string& message
);

std::vector<std::uint8_t> rsa_encrypt(
    std::span<const std::uint8_t> plaintext,
    EVP_PKEY* public_key
);

std::string random_hex(std::size_t bytes);

PKeyPtr load_public_key(std::string_view pem);

bool ed25519_verify(
    std::span<const std::uint8_t> message,
    std::span<const std::uint8_t> signature,
    EVP_PKEY* public_key
);

};
