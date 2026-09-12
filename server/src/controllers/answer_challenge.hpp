#pragma once
#include "db/database.hpp"
#include "http/controller.hpp"

#include "shared.hpp"

class AnswerChallengeController : Controller {
public:
    AnswerChallengeController(Database& db)
       : db_{db} {};

    AsyncResponse GET(RequestContext& request) override;
private:

    Database& db_;
};
