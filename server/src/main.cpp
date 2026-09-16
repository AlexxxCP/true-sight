#include <boost/asio/awaitable.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/beast/websocket/impl/rfc6455.hpp>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "controllers/conversations.hpp"
#include "controllers/healthcheck.hpp"
#include "controllers/get_challenge.hpp"
#include "controllers/validate_challenge.hpp"
#include "controllers/messages.hpp"

#include "db/database.hpp"
#include "http/router.hpp"
#include "middleware/auth_middleware.hpp"
#include "shared.hpp"
#include "ws/ws.hpp"

const std::size_t PORT = 8888;
const std::string CONNECTION_STRING =
    "host=127.0.0.1 "
    "port=5432 "
    "dbname=true_sight "
    "user=true_sight "
    "password=true_sight";


asio::awaitable<void> handle_ws_upgrade(
    tcp::socket socket,
    http::request<http::string_body> request,
    WebSocket& websocket_service
) {
    auto ctx = RequestContext{
        .request = request
    };

    AuthMiddleware auth_check;
    auto res = co_await auth_check.handle(ctx);

    if (res.has_value()) {
        beast::error_code ec;

        co_await beast::async_write(
            socket,
            std::move(*res),
            asio::redirect_error(asio::use_awaitable, ec)
        );

        co_return;
    }

    co_await websocket_service.accept(
        *ctx.authenticated_iid,
        std::move(socket),
        std::move(request)
    );
}

asio::awaitable<void> session(
    tcp::socket socket,
    Router& router,
    WebSocket& websocket_service
) {
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

        if (
            request.target() == "/ws" &&
            websocket::is_upgrade(request)
        ) {
            co_await handle_ws_upgrade(
                std::move(socket),
                std::move(request),
                websocket_service
            );

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

asio::awaitable<void> listen(
    asio::io_context& io,
    Router& router,
    WebSocket& websocket_service
) {
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
            session(std::move(socket), router, websocket_service),
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

    WebSocket websocket_service;

    std::vector<std::shared_ptr<Middleware>> auth_check;
    auth_check.push_back(std::make_shared<AuthMiddleware>());

    Router router{};
    router.register_path("/health-check", std::make_unique<HealthCheckController>(db));
    router.register_path("/get-challenge", std::make_unique<GetChallengeController>(db));
    router.register_path("/validate-challenge", std::make_unique<ValidateChallengeController>(db));
    router.register_path(
        "/messages",
        std::make_unique<MessagesController>(db),
        auth_check
    );

    router.register_path(
        "/conversations",
        std::make_unique<ConversationsController>(db),
        auth_check
    );

    asio::co_spawn(
        io,
        listen(io, router, websocket_service),
        asio::detached
    );

    io.run();
}
