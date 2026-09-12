#pragma once

#include "db/database.hpp"
#include "http/controller.hpp"

class HealthCheckController: public Controller {

public:
    HealthCheckController(Database& db)
        : db_{db} {};

    AsyncResponse GET(RequestContext& request) override;

private:
    Database& db_;

};
