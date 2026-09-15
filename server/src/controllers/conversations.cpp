#include "controllers/conversations.hpp"
#include "http/default_responses.hpp"
#include <boost/asio/use_awaitable.hpp>


AsyncResponse ConversationsController::GET(RequestContext& ctx) {
    auto to = ctx.authenticated_iid;

    if (!to.has_value()) {
        co_return responses::unauthorized(ctx, "The JWT doesn't contain the log in user iid");
    }

    auto rows = co_await db_.exec_async(
        "SELECT "
            "CASE "
                "WHEN sender_iid = $1 THEN receiver_iid "
                "ELSE sender_iid "
            "END AS peer_id, "
            "MAX(created_at) AS last_message_at "
        "FROM messages "
        "WHERE sender_iid = $1 "
            "OR receiver_iid = $1 "
        "GROUP BY peer_id "
        "ORDER BY last_message_at DESC; ",
        pqxx::params{*to},
        asio::use_awaitable
    );

    boost::json::array convos;

    for (const auto& row : rows) {
        convos.push_back({
            {"peer", row["peer_id"].as<std::string>()},
            {"last_message_at", row["last_message_at"].as<std::string>()}
        });
    }

    co_return responses::json(ctx, http::status::ok, {
        {"status", "ok"},
        {"conversations", convos}
    });
}
