#include "controllers/websockets.hpp"
#include "impl/connect.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>

#include <QCoroAsyncGenerator>
#include <QCoroWebSocket>

#include <utility>

WebSocketClient::WebSocketClient(
    QUrl url,
    QObject* parent
)
    : QObject(parent),
      url_(std::move(url))
{}

void WebSocketClient::run(const QString& token) {
    if (running_) {
        pending_token_ = token;
        ++generation_;
        socket_.close();
        return;
    }
    running_ = true;
    const auto generation = ++generation_;

    QCoro::connect(
        runAsync(token, generation),
        this,
        [this] {
            running_ = false;
            if (!pending_token_.isEmpty()) {
                const auto token = std::move(pending_token_);
                pending_token_.clear();
                run(token);
            }
        }
    );
}

void WebSocketClient::stop() {
    ++generation_;
    pending_token_.clear();
    socket_.close();
}

QCoro::Task<> WebSocketClient::runAsync(QString token, std::uint64_t generation)
{
    QNetworkRequest request{url_};

    request.setRawHeader(
        "Authorization",
        QByteArrayLiteral("Bearer ") + token.toUtf8()
    );

    const bool success = co_await qCoro(socket_).open(
        request
    );

    if (generation != generation_) {
        socket_.close();
        co_return;
    }

    if (!success) {
        emit connectionFailed(socket_.errorString());
        co_return;
    }

    emit connected();

    QCORO_FOREACH(
        const QString& message,
        qCoro(socket_).textMessages()
    ) {
        if (generation != generation_) {
            co_return;
        }
        handleMessage(message);
    }

    if (generation == generation_) {
        emit disconnected();
    }
}

void WebSocketClient::handleMessage(
    const QString& message
) {
    QJsonParseError error;

    const auto document = QJsonDocument::fromJson(
        message.toUtf8(),
        &error
    );

    if (error.error != QJsonParseError::NoError) {
        return;
    }

    if (!document.isObject()) {
        return;
    }

    const QJsonObject object = document.object();
    const QString event = object.value("event").toString();

    if (event == "message_created") {
        const QString peer = object.value("sender_iid").toString();

        if (!peer.isEmpty()) {
            emit messageReceived(peer);
        }
    }
}
