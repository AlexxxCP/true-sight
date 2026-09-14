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
    RequestContext& context,
    std::string message
) {
    Response res = json(context, http::status::not_found, {{"status", message }});
    return res;
}

Response bad_request(
    RequestContext& ctx,
    const std::string& err
) {
    Response res = json(ctx, http::status::bad_request, {{ "status", err }});
    return res;
}

Response internal_server_error(
    RequestContext& context,
    std::string message
) {
    Response res = json(context, http::status::internal_server_error, {{ "status", message }});
    return res;
}

Response unauthorized(
    RequestContext& context,
    std::string message
) {
    Response res = json(context, http::status::unauthorized, {{ "status", message }});
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



