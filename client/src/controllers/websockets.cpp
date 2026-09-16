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
        return;
    }
    running_ = true;

    QCoro::connect(
        runAsync(token),
        this,
        [this] {
            running_ = false;
        }
    );
}

void WebSocketClient::stop() {
    socket_.close();
}

QCoro::Task<> WebSocketClient::runAsync(QString token)
{
    QNetworkRequest request{url_};

    request.setRawHeader(
        "Authorization",
        QByteArrayLiteral("Bearer ") + token.toUtf8()
    );

    const bool success = co_await qCoro(socket_).open(
        request
    );

    if (!success) {
        emit connectionFailed(socket_.errorString());
        co_return;
    }

    emit connected();

    QCORO_FOREACH(
        const QString& message,
        qCoro(socket_).textMessages()
    ) {
        handleMessage(message);
    }

    emit disconnected();
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
