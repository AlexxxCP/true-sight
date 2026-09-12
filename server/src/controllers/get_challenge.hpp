#pragma once

#include "http/controller.hpp"
#include "db/database.hpp"
#include "http/default_responses.hpp"
#include "shared.hpp"
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/http/status.hpp>

class GetChallengeController : public Controller {
public:
    GetChallengeController(Database& db)
        : db_{db} {}

    AsyncResponse POST(RequestContext& request) override;

private:

    Database& db_;
};

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
        "SELECT * FROM user_to_pk WHERE user_iid = $1;",
        pqxx::params{user_iid},
        asio::use_awaitable
    );



}
