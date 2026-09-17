#pragma once

#include "db/database.hpp"
#include "http/controller.hpp"

class RegisterController final : public Controller {
public:
    explicit RegisterController(Database& db) : db_{db} {}

    AsyncResponse POST(RequestContext& ctx) override;

private:
    Database& db_;
};
