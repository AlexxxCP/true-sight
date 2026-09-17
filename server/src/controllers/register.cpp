#include "controllers/register.hpp"

#include "crypto/crypto.hpp"
#include "http/default_responses.hpp"

#include <boost/asio/use_awaitable.hpp>
#include <openssl/evp.h>

AsyncResponse RegisterController::POST(RequestContext& ctx) {
    boost::system::error_code parse_error;
    const auto body = boost::json::parse(ctx.request.body(), parse_error);
    if (parse_error || !body.is_object()) {
        co_return responses::bad_request(ctx, "Expected a JSON object");
    }

    const auto& object = body.as_object();
    const auto iid_value = object.if_contains("user_iid");
    const auto key_value = object.if_contains("ed25519_pk");
    if (!iid_value || !iid_value->is_string() ||
        !key_value || !key_value->is_string()) {
        co_return responses::bad_request(ctx, "user_iid and ed25519_pk are required strings");
    }

    const auto& iid_json = iid_value->as_string();
    const auto& key_json = key_value->as_string();
    std::string user_iid(iid_json.data(), iid_json.size());
    std::string public_key_pem(key_json.data(), key_json.size());
    if (user_iid.empty() || user_iid.size() > 128 ||
        user_iid.find('\0') != std::string::npos) {
        co_return responses::bad_request(ctx, "Invalid user_iid");
    }
    if (public_key_pem.empty() || public_key_pem.size() > 4096) {
        co_return responses::bad_request(ctx, "Invalid Ed25519 public key PEM");
    }

    const auto public_key = crypto::load_public_key(public_key_pem);
    std::size_t raw_key_size = 0;
    if (!public_key || EVP_PKEY_base_id(public_key.get()) != EVP_PKEY_ED25519 ||
        EVP_PKEY_get_raw_public_key(public_key.get(), nullptr, &raw_key_size) != 1 ||
        raw_key_size != 32) {
        co_return responses::bad_request(ctx, "Invalid Ed25519 public key PEM");
    }

    // The unique constraint on user_iid makes the check and insert atomic.
    const auto inserted = co_await db_.exec_async(
        "INSERT INTO user_to_pk (user_iid, ed25519_pk) VALUES ($1, $2) "
        "ON CONFLICT (user_iid) DO NOTHING RETURNING id;",
        pqxx::params{user_iid, public_key_pem},
        asio::use_awaitable
    );
    if (inserted.empty()) {
        co_return responses::json(ctx, http::status::conflict,
                                {{"status", "user_iid already registered"}});
    }

    co_return responses::json(ctx, http::status::ok, {{"status", "ok"}});
}
