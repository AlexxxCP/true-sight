#pragma once

#include <QObject>
#include <QUrl>
#include <QWebSocket>
#include <cstdint>

#include <qcorowebsockets.h>

class WebSocketClient : public QObject {
    Q_OBJECT

public:
    explicit WebSocketClient(
        QUrl url,
        QObject* parent = nullptr
    );

    void run(const QString& token);
    void stop();

signals:
    void connected();
    void disconnected();
    void connectionFailed(const QString& error);
    void messageReceived(const QString& peer);

private:
    QCoro::Task<> runAsync(QString token, std::uint64_t generation);
    void handleMessage(const QString& message);

    QUrl url_;
    QWebSocket socket_;
    bool running_ = false;
    std::uint64_t generation_ = 0;
    QString pending_token_;
};
