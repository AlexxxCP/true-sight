#pragma once

#include "http/middleware.hpp"
#include "shared.hpp"

#include <boost/asio/awaitable.hpp>

class AuthMiddleware : public Middleware {

public:
    MiddlewareResponse handle(RequestContext& ctx);

};
