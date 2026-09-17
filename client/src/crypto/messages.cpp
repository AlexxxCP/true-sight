#include "crypto/messages.hpp"

#include <stdexcept>

namespace crypto::messages {
namespace {

void append_u64(std::vector<std::uint8_t>& out, std::uint64_t value) {
    for (int shift = 56; shift >= 0; shift -= 8) {
        out.push_back(static_cast<std::uint8_t>(value >> shift));
    }
}

void append_bytes(std::vector<std::uint8_t>& out, std::span<const std::uint8_t> bytes) {
    append_u64(out, bytes.size());
    out.insert(out.end(), bytes.begin(), bytes.end());
}

} // namespace

std::vector<std::uint8_t> signed_envelope(
    std::string_view sender,
    std::string_view receiver,
    std::uint64_t clock,
    std::span<const std::uint8_t> nonce,
    std::span<const std::uint8_t> ciphertext,
    std::span<const std::uint8_t> tag
) {
    if (clock == 0) {
        throw std::invalid_argument("Lamport clock must be positive");
    }

    std::vector<std::uint8_t> out;
    constexpr std::string_view domain = "true-sight/message-envelope/v2";
    append_bytes(out, {reinterpret_cast<const std::uint8_t*>(domain.data()), domain.size()});
    append_u64(out, 2);
    append_bytes(out, {reinterpret_cast<const std::uint8_t*>(sender.data()), sender.size()});
    append_bytes(out, {reinterpret_cast<const std::uint8_t*>(receiver.data()), receiver.size()});
    append_u64(out, clock);
    append_bytes(out, nonce);
    append_bytes(out, ciphertext);
    append_bytes(out, tag);
    return out;
}

} // namespace crypto::messages
