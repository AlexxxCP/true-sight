#pragma once

#include "shared.hpp"
#include "boost/json.hpp"

struct Field {
    std::string_view name;
    boost::json::kind type;
};

namespace responses {
    Response json(RequestContext& context, http::status status, const boost::json::value& body);
    Response method_not_allowed(RequestContext& context);
    Response bad_request(RequestContext& context, const std::string&);
    Response not_found(RequestContext& context, std::string message = "Not Found");
    Response internal_server_error(RequestContext& context, std::string message = "Internal server error");
    Response unauthorized(RequestContext& context, std::string message = "Unauthorized");

    template<std::size_t N>
    std::optional<Response> fields_required(
        RequestContext& ctx,
        const boost::json::object& obj,
        const std::array<Field, N>& fields
    ) {
        for (const auto& field : fields) {
            auto it = obj.find(field.name);

            if (it == obj.end()) {
                return bad_request(
                    ctx,
                    "missing required field: " + std::string(field.name)
                );
            }

            if (it->value().kind() != field.type) {
                return bad_request(
                    ctx,
                    "invalid type for field: " + std::string(field.name)
                );
            }
        }

        return std::nullopt;
    }
};
