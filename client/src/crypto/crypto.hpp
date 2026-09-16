#pragma once

#include <vector>
#include <span>
#include <openssl/evp.h>
#include <array>
#include <optional>

namespace crypto {

using u8 = uint8_t;

std::vector<unsigned char> sign_ed25519(
    const std::vector<uint8_t>& private_key,
    std::span<const unsigned char> msg
);

std::string base64url_encode(std::span<const unsigned char> input);

std::array<u8, 32> x25519_shared_secret(
    std::span<const u8, 32> sk,
    std::span<const u8, 32> pk
);

std::array<u8, 32> hkdf_sha256(
    std::span<const u8> ikm,
    std::span<const u8> salt,
    std::span<const u8> info
);

struct EncryptedMessage {
    std::vector<u8> ciphertext;
    std::array<u8, 12> nonce;
    std::array<u8, 16> tag;
};

EncryptedMessage aes_256_gcm_siv_encrypt(
    std::span<const u8, 32> key,
    std::span<const u8> plaintext,
    std::span<const u8> aad
);

std::optional<std::vector<u8>> aes_256_gcm_siv_decrypt(
    std::span<const u8, 32> key,
    std::span<const u8, 12> nonce,
    std::span<const u8> ciphertext,
    std::span<const u8, 16> tag,
    std::span<const u8> aad
);

};
