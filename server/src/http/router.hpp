#pragma once

#include "http/controller.hpp"
#include "http/middleware.hpp"
#include "shared.hpp"

#include <memory>
#include <unordered_map>

class Router {
    using Controller_p = std::unique_ptr<Controller>;
    using Middlewares_p = std::vector<std::shared_ptr<Middleware>>;

    struct Route {
        Controller_p controller;
        std::vector<std::shared_ptr<Middleware>> middleware;
    };

public:
    void register_path(
        std::string path,
        Controller_p controller,
        Middlewares_p middleware = {}
    );

    AsyncResponse route(
        const Request& request
    );

private:
    std::unordered_map<std::string, Route> paths_;
    Controller_p default_controller_;
};

