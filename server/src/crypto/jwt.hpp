#pragma once

#include <boost/json.hpp>
#include <cstdlib>
#include <string>

#include <openssl/hmac.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>

#include "shared.hpp"

namespace crypto::jwt {
    struct Claims {
        std::string user_iid;
        i64 issued_at;
        i64 expires_at;
        std::string token_id;
    };

    std::pair<std::string, Claims> create_access_token(std::string_view user_iid);
    std::optional<Claims> validate_access_token(const std::string &);
    std::string create(boost::json::object body);
    bool validate_signature(const std::string&);
};


