#pragma once

#include <vector>
#include <span>
#include <openssl/evp.h>
#include <filesystem>

namespace crypto {

std::vector<unsigned char> sign_ed25519(
    const std::filesystem::path& private_key_path,
    std::span<const unsigned char> msg
);

std::string base64url_encode(std::span<const unsigned char> input);

};
