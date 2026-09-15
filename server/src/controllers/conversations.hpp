#pragma once

#include "http/controller.hpp"
#include "db/database.hpp"

class ConversationsController : public Controller {
    public:
        ConversationsController(Database& db) :
            db_{db} {}

        AsyncResponse GET(RequestContext& request) override;

    private:
        Database& db_;
};
