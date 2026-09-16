#pragma once

#include <QObject>
#include <QUrl>
#include <QWebSocket>

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
    QCoro::Task<> runAsync(QString token);
    void handleMessage(const QString& message);

    QUrl url_;
    QWebSocket socket_;
    bool running_ = false;
};
