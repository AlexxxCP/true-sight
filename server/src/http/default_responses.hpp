#pragma once

#include "shared.hpp"
#include "boost/json.hpp"

namespace responses {
    Response json(RequestContext& context, http::status status, const boost::json::value& body);
    Response method_not_allowed(RequestContext& context);
    Response bad_request(RequestContext& context, const std::string&);
    Response not_found(RequestContext& context, std::string message = "Not Found");
    Response internal_server_error(RequestContext& context, std::string message = "Internal server error");
    Response unauthorized( RequestContext& context, std::string message = "Unauthorized");
};
