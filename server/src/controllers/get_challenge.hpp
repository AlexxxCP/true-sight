#pragma once

#include "http/controller.hpp"
#include "db/database.hpp"
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


