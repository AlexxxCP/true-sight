#include "controllers/healthcheck.hpp"
#include "http/default_responses.hpp"

AsyncResponse HealthCheckController::GET(RequestContext& request) {
    auto response = responses::json(request, http::status::ok, {{"status", "ok"}});

    co_return response;
}
