#include "controllers/get_challenge.hpp"
#include "crypto/crypto.hpp"
#include "http/default_responses.hpp"

AsyncResponse GetChallengeController::POST(RequestContext& ctx) {
    boost::json::value body = boost::json::parse(ctx.request.body());

    if (!body.is_object()) {
        co_return responses::bad_request(ctx, "The requst must be a JSON object");
    }

    boost::json::object& obj = body.as_object();
    auto it = obj.find("user_iid");

    if (it == obj.end() || !it->value().is_string()) {
        co_return responses::bad_request(ctx, "user_iid is required");
    }

    std::string user_iid = it->value().as_string().c_str();

    auto res = co_await db_.exec_async(
        "SELECT 1 FROM user_to_pk WHERE user_iid = $1;",
        pqxx::params{user_iid},
        asio::use_awaitable
    );

    if (res.empty()) {
        co_return responses::not_found(ctx, "User not found");
    }

    auto existant_challenge = co_await db_.exec_async(
        "SELECT id, challenge FROM auth_challenge "
        "WHERE user_iid = $1 AND NOW() < expires_at AND is_used = false "
        "LIMIT 1;",
        pqxx::params{user_iid},
        asio::use_awaitable
    );

    if (!existant_challenge.empty()) {
        auto challenge = existant_challenge[0];

        co_return responses::json(
            ctx,
            http::status::ok,
            {
                { "challenge", challenge["challenge"].as<std::string>() },
                { "challenge_id", challenge["id"].as<std::string>() }
            }
        );
    }

    std::string challenge = crypto::random_hex(32);

    auto auth_challenge = co_await db_.exec_async(
        "INSERT INTO auth_challenge (user_iid, challenge, expires_at) "
        "VALUES ($1, $2, NOW() + INTERVAL '5 minutes') "
        "RETURNING id;",
        pqxx::params{user_iid, challenge},
        asio::use_awaitable
    );

    auto id = auth_challenge[0]["id"].as<std::string>();
    boost::json::object res_body = {
        {"challenge", challenge},
        {"challenge_id", id}
    };

    co_return responses::json(ctx, http::status::ok, res_body);
}
