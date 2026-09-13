#include <iostream>
#include <memory>
#include <string>

#include "controllers/healthcheck.hpp"
#include "controllers/get_challenge.hpp"
#include "controllers/validate_challenge.hpp"

#include "db/database.hpp"
#include "http/router.hpp"
#include "shared.hpp"

const std::size_t PORT = 8888;
const std::string CONNECTION_STRING =
    "host=127.0.0.1 "
    "port=5432 "
    "dbname=true_sight "
    "user=true_sight "
    "password=true_sight";

asio::awaitable<void> session(tcp::socket socket, Router& router) {
    beast::flat_buffer buffer;

    for (;;) {
        http::request<http::string_body> request;

        beast::error_code ec;

        co_await http::async_read(
            socket,
            buffer,
            request,
            asio::redirect_error(asio::use_awaitable, ec)
        );

        if (ec == http::error::end_of_stream) {
            break;
        }

        if (ec) {
            co_return;
        }

        auto response = co_await router.route(request);
        bool keep_alive = response.keep_alive();

        co_await beast::async_write(
            socket,
            std::move(response),
            asio::redirect_error(asio::use_awaitable, ec)
        );

        if (ec) {
            co_return;
        }

        if (!keep_alive) {
            break;
        }
    }

    beast::error_code ec;
    auto _ = socket.shutdown(tcp::socket::shutdown_send, ec);
}

asio::awaitable<void> listen(asio::io_context& io, Router& context) {

    tcp::acceptor acceptor(
        io,
        tcp::endpoint(tcp::v4(), PORT)
    );

    std::cout << "Listening on http://localhost:" << PORT << '\n';

    for (;;) {
        tcp::socket socket =
            co_await acceptor.async_accept(asio::use_awaitable);

        asio::co_spawn(
            io,
            session(std::move(socket), context),
            asio::detached
        );
    }
}

int main() {
    asio::io_context io;

    Database db{
        CONNECTION_STRING,
        4
    };

    Router router{};

    router.register_path("/health-check", std::make_unique<HealthCheckController>(db));
    router.register_path("/get-challenge", std::make_unique<GetChallengeController>(db));
    router.register_path("/validate-challenge", std::make_unique<ValidateChallengeController>(db));

    asio::co_spawn(
        io,
        listen(io, router),
        asio::detached
    );

    io.run();
}
