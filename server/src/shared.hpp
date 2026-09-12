#pragma once

#include <boost/beast/http/string_body_fwd.hpp>
#include <stdint.h>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>

using u64 = uint64_t;
using i64 = int64_t;
using u32 = uint32_t;
using i32 = int32_t;

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;

using tcp = asio::ip::tcp;

using Request = http::request<http::string_body>;
using Response = http::response<http::string_body>;

struct RequestContext {
    const Request& request;
    std::optional<std::string> authenticated_iid;
};

using AsyncResponse = asio::awaitable<http::message_generator>;
