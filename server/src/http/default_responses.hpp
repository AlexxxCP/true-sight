#pragma once

#include "shared.hpp"
#include "boost/json.hpp"

namespace responses {
    Response json(RequestContext& context, http::status status, const boost::json::value& body);
    Response method_not_allowed(RequestContext& context);
    Response not_found(RequestContext& context);
};
