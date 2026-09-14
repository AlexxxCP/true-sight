#pragma once

#include "db/database.hpp"
#include "http/controller.hpp"
#include "shared.hpp"

class MessagesController : public Controller {
    public:
        MessagesController(Database& db)
            : db_{db} {};

        AsyncResponse GET(RequestContext& request) override;
        AsyncResponse POST(RequestContext& request) override;

    private:
        Database& db_;
};
