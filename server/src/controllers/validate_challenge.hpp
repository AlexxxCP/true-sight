#pragma once
#include "crypto/crypto.hpp"
#include "crypto/jwt.hpp"

#include "db/database.hpp"

#include "http/controller.hpp"
#include "http/default_responses.hpp"

#include "shared.hpp"
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/http/status.hpp>

class ValidateChallengeController : public Controller {
public:
    ValidateChallengeController(Database& db)
       : db_{db} {};

    AsyncResponse POST(RequestContext& ctx) override;
private:

    Database& db_;
};


AsyncResponse ValidateChallengeController::POST(RequestContext& ctx) {
    boost::json::value body = boost::json::parse(ctx.request.body());

    if (!body.is_object()) {
        co_return responses::bad_request(ctx, "The requst must be a JSON object");
    }

    boost::json::object& obj = body.as_object();
    auto it_challenge_id = obj.find("challenge_id");
    auto it_signed_challenge = obj.find("signed_challenge");

    if (it_challenge_id == obj.end() || !it_challenge_id->value().is_string()) {
        co_return responses::bad_request(ctx, "challenge_id is required");
    }

    if (it_signed_challenge == obj.end() || !it_signed_challenge->value().is_string()) {
        co_return responses::bad_request(ctx, "signed_challenge is required");
    }

    std::string challenge_id = it_challenge_id->value().as_string().c_str();
    std::string signed_challenge_str = it_signed_challenge->value().as_string().c_str();


    auto res = co_await db_.exec_async(
        "SELECT user_iid, challenge FROM auth_challenge WHERE id = $1 AND IS_USED = false AND NOW() < expires_at;",
        pqxx::params{challenge_id},
        asio::use_awaitable
    );

    if (res.empty()) {
        co_return responses::not_found(ctx, "Challenge not found, expired, or already used");
    }

    std::string challenge = res[0]["challenge"].as<std::string>();
    std::string user_iid = res[0]["user_iid"].as<std::string>();

    auto user_pk_res = co_await db_.exec_async(
        "SELECT ed25519_pk FROM user_to_pk WHERE user_iid = $1",
        pqxx::params{user_iid},
        asio::use_awaitable
    );

    if (user_pk_res.empty()) {
        co_return responses::not_found(ctx, "User not found");
    }

    std::string ed25519_pk = user_pk_res[0]["ed25519_pk"].as<std::string>();

    std::span<const u8> msg{
        reinterpret_cast<const u8*>(challenge.data()),
        challenge.size()
    };

    std::vector<u8> signature = crypto::base64url_decode(signed_challenge_str);
    if (signature.size() != 64) {
        co_return responses::bad_request(ctx, "Invalid Ed25519 signature");
    }

    auto public_key = crypto::load_public_key(ed25519_pk);
    if (!public_key) {
        co_return responses::internal_server_error(
            ctx,
            "Invalid stored public key"
        );
    }

    bool valid = crypto::ed25519_verify(msg, signature, public_key.get());

    if (!valid) {
        co_return responses::unauthorized(ctx, "Invalid signature");
    }

    auto [access_token, claims] = crypto::jwt::create_access_token(user_iid);

    auto update_res = co_await db_.exec_async(
        "UPDATE auth_challenge SET is_used = TRUE "
        "WHERE id = $1 AND is_used = FALSE AND NOW() < expires_at "
        "RETURNING id;",
        pqxx::params{challenge_id},
        asio::use_awaitable
    );

    if (update_res.empty()) {
        co_return responses::not_found(ctx, "Challenge not found, expired, or already used");
    }

    co_return responses::json(
        ctx,
        http::status::ok,
        {
            {"status", "ok"},
            {"access_token", access_token},
            {"expires_at", claims.expires_at}
        }
    );
};
