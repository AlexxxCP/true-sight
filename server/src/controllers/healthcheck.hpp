#pragma once

#include "db/database.hpp"
#include "http/controller.hpp"
#include "ws/ws.hpp"

class HealthCheckController: public Controller {

public:
    HealthCheckController(Database& db, WebSocket& ws)
        : db_{db}, ws_{ws} {};

    AsyncResponse GET(RequestContext& request) override;

private:
    Database& db_;
    WebSocket& ws_;

};
