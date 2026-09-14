#include "controllers/messages.hpp"
#include "crypto/crypto.hpp"
#include "shared.hpp"
#include "http/default_responses.hpp"
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/http/status.hpp>

// {
//      "to": "Alice",
//      "nonce": <base64encoded>,
//      "ciphertext": <base64encoded>,
//      "authtag": <base64encoded>,
//      "protocol_version": 1,
//      "message_counter": 2
// }

constexpr std::array message_create{
    Field{"to", boost::json::kind::string},
    Field{"nonce", boost::json::kind::string},
    Field{"authtag", boost::json::kind::string},
    Field{"protocol_version", boost::json::kind::int64},
    Field{"message_counter", boost::json::kind::int64}
};

AsyncResponse MessagesController::POST(RequestContext& ctx) {
    boost::json::value body;

    try {
        body = boost::json::parse(ctx.request.body());
    } catch (const boost::system::system_error&) {
        co_return responses::bad_request(ctx, "Invalid JSON");
    }

    if (!body.is_object()) {
        co_return responses::bad_request(ctx, "The requst must be a JSON object");
    }

    if (!ctx.authenticated_iid.has_value()) {
        co_return responses::unauthorized(ctx, "The JWT token did not contain the user iid");
    }

    boost::json::object& obj = body.as_object();

    auto invalid_fields_res = responses::fields_required(
        ctx,
        obj,
        message_create
    );

    if (invalid_fields_res.has_value()) {
        co_return std::move(*invalid_fields_res);
    }

    auto from = ctx.authenticated_iid.value();
    auto to = obj["to"].as_string().c_str();
    auto protocol_version = obj["protocol_version"].as_int64();
    auto message_counter = obj["message_counter"].as_int64();

    auto nonce = crypto::base64url_decode(obj["nonce"].as_string());
    auto ciphertext = crypto::base64url_decode(obj["ciphertext"].as_string());
    auto authtag = crypto::base64url_decode(obj["authtag"].as_string());

    if (nonce.size() != 12) {
        co_return responses::bad_request(ctx, "Ivalid nonce");
    }

    if (authtag.size() != 16) {
        co_return responses::bad_request(ctx, "Ivalid auth tag");
    }

    if (ciphertext.empty()) {
        co_return responses::bad_request(ctx, "Invalid ciphertext");
    }

    if (protocol_version != 1) {
        co_return responses::bad_request(ctx, "Unsupported protocol version");
    }

    if (message_counter < 0) {
        co_return responses::bad_request(ctx, "Invalid message counter");
    }

    auto nonce_bytes = std::span<std::byte>{
        reinterpret_cast<std::byte*>(nonce.data()),
        nonce.size()
    };

    auto ciphertext_bytes = std::span<std::byte>{
        reinterpret_cast<std::byte*>(ciphertext.data()),
        ciphertext.size()
    };

    auto authtag_bytes = std::span<std::byte>{
        reinterpret_cast<std::byte*>(authtag.data()),
        authtag.size()
    };

    co_await db_.exec_async(
        "INSERT INTO messages "
        "(sender_iid, receiver_iid, nonce, ciphertext, auth_tag, protocol_version, message_counter) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7);",
        {from, to, nonce_bytes, ciphertext_bytes, authtag_bytes, protocol_version, message_counter},
        asio::use_awaitable
    );

    co_return responses::json(ctx, http::status::ok, {{ "status", "ok" }});
}

