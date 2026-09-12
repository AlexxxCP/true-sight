#include "crypto/jwt.hpp"

std::string base64url_encode(const std::string& input) {
    size_t encoded_size = 4 * ((input.size() + 2) / 3);

    std::string result(encoded_size, '\0');

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

std::string JWT::create(boost::json::object body) {
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

    std::string encoded_body = base64url_encode(body_json);
    std::string encoded_header = base64url_encode(header_json);

    std::string signing_input =
        encoded_header + "." + encoded_body;

    auto signature = hmac_sha256(secret, signing_input);

    std::string signature_str(
        reinterpret_cast<const char*>(signature.data()),
        signature.size()
    );

    return signing_input + "." + base64url_encode(signature_str);
}

bool JWT::validate(const std::string& token) {
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

    auto raw_sig = hmac_sha256(raw, signing_input);
    std::string sig_str(
        reinterpret_cast<const char*>(raw_sig.data()),
        raw_sig.size()
    );

    std::string expected = base64url_encode(sig_str);

    if (signature.size() != expected.size()) {
        return false;
    }

    return CRYPTO_memcmp(
        signature.data(),
        expected.data(),
        expected.size()
    ) == 0;
}
