#pragma once

#include "shared.hpp"

using MiddlewareResult =
    asio::awaitable<std::optional<http::message_generator>>;

class Middleware {
public:
    virtual MiddlewareResult handle(RequestContext& context) = 0;
    virtual ~Middleware() = default;
};

