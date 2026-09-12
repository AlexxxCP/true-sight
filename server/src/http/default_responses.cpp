#include "http/default_responses.hpp"
#include "shared.hpp"

namespace responses {

Response method_not_allowed(
    RequestContext& context
) {
    Response res = json(context, http::status::method_not_allowed, {{"status", "Method Not Allowed"}});
    return res;
}

Response not_found(
    RequestContext& context
) {
    Response res = json(context, http::status::not_found, {{"status", "Not Found"}});
    return res;
}

Response json(RequestContext& context, http::status status, const boost::json::value& body) {
    http::response<http::string_body> response {
        status,
        context.request.version()
    };

    response.set(
        http::field::content_type,
        "application/json"
    );

    response.keep_alive(context.request.keep_alive());
    response.body() = boost::json::serialize(body);
    response.prepare_payload();

    return response;
}

}

