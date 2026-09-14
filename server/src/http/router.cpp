#include "http/router.hpp"
#include "http/default_responses.hpp"
#include <boost/url.hpp>

void Router::register_path(std::string path, Controller_p controller, Middlewares_p middleware) {
    paths_.insert_or_assign(
        std::move(path),
        Route{
            std::move(controller),
            middleware
        }
    );
};

AsyncResponse Router::route(const Request& req) {
    RequestContext context{
        .request = req
    };

    auto parsed = boost::urls::parse_origin_form(req.target());
    if (!parsed) {
        co_return responses::bad_request(context, "Invalid query params");
    }

    context.url = parsed.value();

    auto it = paths_.find(std::string(parsed->encoded_path()));
    if (it == paths_.end()) {
        co_return responses::not_found(context);
    }

    for (auto& middleware : it->second.middleware) {
        auto middlware_res = co_await middleware->handle(context);

        if (middlware_res.has_value()) {
            co_return std::move(*middlware_res);
        }
    }

    switch(context.request.method()) {
        case http::verb::get:
            co_return co_await it->second.controller->GET(context);

        case http::verb::post:
            co_return co_await it->second.controller->POST(context);

        case http::verb::put:
            co_return co_await it->second.controller->PUT(context);

        case http::verb::delete_:
            co_return co_await it->second.controller->DELETE(context);

        default:
            co_return responses::method_not_allowed(context);
    };
}
