#include "crypto/jwt.hpp"
#include "http/middleware.hpp"

#include "shared.hpp"
#include "middleware/auth_middleware.hpp"
#include "http/default_responses.hpp"

#include <boost/asio/awaitable.hpp>
#include <optional>

MiddlewareResponse AuthMiddleware::handle(RequestContext& ctx) {
    auto it = ctx.request.find(http::field::authorization);

    if (it == ctx.request.end()) {
        co_return responses::unauthorized(ctx, "Accessing this endpoint requres a Bearer token");
    }

    std::string value = it->value();

    constexpr std::string_view prefix = "Bearer ";

    if (!value.starts_with(prefix)) {
        co_return responses::unauthorized(ctx, "Authorization field value must start with 'Bearer '");
    }

    std::string token = value.substr(prefix.size());
    auto claims = crypto::jwt::validate_access_token(token);

    if (!claims.has_value()) {
        co_return responses::unauthorized(ctx, "JWT token is invalid");
    }

    ctx.authenticated_iid = claims.value().user_iid;
    co_return std::nullopt;
}

