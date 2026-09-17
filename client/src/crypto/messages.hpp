#pragma once

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace crypto::messages {

// Version 2 signs the complete transport envelope. Length prefixes make the
// encoding unambiguous even when user IDs contain delimiters.
std::vector<std::uint8_t> signed_envelope(
    std::string_view sender,
    std::string_view receiver,
    std::uint64_t clock,
    std::span<const std::uint8_t> nonce,
    std::span<const std::uint8_t> ciphertext,
    std::span<const std::uint8_t> tag
);

} // namespace crypto::messages
