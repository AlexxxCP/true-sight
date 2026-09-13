#include "crypto/jwt.hpp"
#include "crypto/crypto.hpp"
#include <boost/json/object.hpp>
#include <boost/json/parse.hpp>

namespace crypto::jwt {

i64 now_unix() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

std::pair<std::string, Claims> create_access_token(std::string_view user_iid) {
    auto now = now_unix();

    std::string user_iid_str{
        user_iid.data(),
        user_iid.size()
    };

    Claims c{
        .user_iid = user_iid_str,
        .issued_at = now,
        .expires_at = now + 15 * 60,
        .token_id = random_hex(16),
    };

    boost::json::object obj = {
        {"sub", user_iid},
        {"iat", c.issued_at},
        {"exp", c.expires_at},
        {"jti", c.token_id},
        {"iss", "true-sight"},
        {"aud", "true-sight-api"},
    };

    return { create(obj), c };
}

std::optional<Claims> validate_access_token(const std::string& token) {
    if (!validate_signature(token)) {
        return {};
    }

    size_t first = token.find('.');
    if (first == std::string::npos) {
        return {};
    }

    size_t second = token.find('.', first + 1);
    if (second == std::string::npos) {
        return {};
    }

    std::string encoded_body = token.substr(first + 1, second - first - 1);
    std::string encoded_header = token.substr(0, first);

    auto decoded_body_bytes = crypto::base64url_decode(encoded_body);
    auto decoded_header_bytes = crypto::base64url_decode(encoded_header);

    std::string decoded_body_str{
        reinterpret_cast<const char*>(decoded_body_bytes.data()),
        decoded_body_bytes.size()
    };

    std::string decoded_header_str{
        reinterpret_cast<const char*>(decoded_header_bytes.data()),
        decoded_header_bytes.size()
    };

    boost::system::error_code ec_b;
    boost::json::value body = boost::json::parse(decoded_body_str, ec_b);

    boost::system::error_code ec_h;
    boost::json::value header = boost::json::parse(decoded_header_str, ec_h);

    if (ec_b || !body.is_object() || ec_h || !header.is_object()) {
        return {};
    }

    auto& obj = body.as_object();
    auto& objh = header.as_object();

    if (
        !obj.contains("sub") ||
        !obj.contains("iat") ||
        !obj.contains("exp") ||
        !obj.contains("jti") ||
        !obj.contains("iss") ||
        !obj.contains("aud") ||
        !objh.contains("alg") ||
        !objh.contains("typ") ||
        !obj["sub"].is_string() ||
        !obj["iat"].is_int64() ||
        !obj["exp"].is_int64() ||
        !obj["jti"].is_string() ||
        !obj["iss"].is_string() ||
        !obj["aud"].is_string() ||
        !objh["alg"].is_string() ||
        !objh["typ"].is_string()
    ) {
        return {};
    }

    if (
        obj["iss"].as_string() != "true-sight" ||
        obj["aud"].as_string() != "true-sight-api" ||
        objh["alg"].as_string() != "HS256" ||
        objh["typ"].as_string() != "JWT"
    ) {
        return {};
    }

    auto now = now_unix();
    auto iat = obj["iat"].as_int64();
    auto exp = obj["exp"].as_int64();

    if (
        exp <= now ||
        exp <= iat ||
        iat > now + 30
    ) {
        return {};
    }

    return Claims {
        .user_iid = obj["sub"].as_string().c_str(),
        .issued_at = iat,
        .expires_at = exp,
        .token_id = obj["jti"].as_string().c_str()
    };
};

std::string create(boost::json::object body) {
    const char* raw = std::getenv("JWT_SECRET");

    if (raw == nullptr) {
        throw std::runtime_error("JWT_SECRET is not set");
    }

    std::string secret = raw;

    boost::json::object header = {
        {"alg", "HS256"},
        {"typ", "JWT"}
    };

    std::string header_json = boost::json::serialize(header);
    std::string body_json = boost::json::serialize(body);

    std::string encoded_body = crypto::base64url_encode(body_json);
    std::string encoded_header = crypto::base64url_encode(header_json);

    std::string signing_input =
        encoded_header + "." + encoded_body;

    auto signature = crypto::hmac_sha256(secret, signing_input);

    std::string signature_str(
        reinterpret_cast<const char*>(signature.data()),
        signature.size()
    );

    return signing_input + "." + crypto::base64url_encode(signature_str);
}

bool validate_signature(const std::string& token) {
    const char* raw = std::getenv("JWT_SECRET");

    if (raw == nullptr) {
        throw std::runtime_error("JWT_SECRET is not set");
    }

    size_t first = token.find('.');
    if (first == std::string::npos) {
        return false;
    }

    size_t second = token.find('.', first + 1);

    if (second == std::string::npos) {
        return false;
    }

     if (token.find('.', second + 1) != std::string::npos) {
        return false;
    }

    std::string signing_input = token.substr(0, second);
    std::string signature = token.substr(second + 1);

    auto raw_sig = crypto::hmac_sha256(raw, signing_input);
    auto supplied_signature = crypto::base64url_decode(signature);

    if (supplied_signature.size() != raw_sig.size()) {
        return false;
    }

    return CRYPTO_memcmp(
        supplied_signature.data(),
        raw_sig.data(),
        raw_sig.size()
    ) == 0;
}

}
