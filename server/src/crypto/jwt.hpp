#pragma once

#include <boost/json.hpp>
#include <cstdlib>
#include <string>

#include <openssl/hmac.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>

class JWT {
public:
    JWT();

    static std::string create(boost::json::object body);
    static bool validate(const std::string&);
};


