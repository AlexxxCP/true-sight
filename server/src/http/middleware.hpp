#pragma once

#include "shared.hpp"

#include <boost/asio/awaitable.hpp>

class Middleware {
public:
    virtual MiddlewareResponse handle(RequestContext& ctx) = 0;
    virtual ~Middleware() = default;
};
