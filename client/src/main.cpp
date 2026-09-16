#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QJsonObject>
#include <QWebSocket>

#include <qcorowebsockets.h>

#include "app_settings.hpp"
#include "controllers/auth.hpp"
#include "controllers/messenger.hpp"
#include "controllers/websockets.hpp"
#include "stores/identity_keys.hpp"

QCoro::Task<void> runWebSocket(
    const QString& token
) {
    QWebSocket socket;

    QNetworkRequest request{
        QUrl{"ws://localhost:8888/ws"}
    };

    request.setRawHeader(
        "Authorization",
        "Bearer " + token.toUtf8()
    );

    bool connected = co_await qCoro(socket).open(
        request
    );

    if (!connected) {
        qDebug() << "failed to connect";
        co_return;
    }

    qDebug() << "connected";

    QCORO_FOREACH(
        const QString& message,
        qCoro(socket).textMessages()
    ) {
        qDebug() << "received:" << message;
    }
}


int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    AppSettings app_settings;

    IdentityKeys identity_keys;

    WebSocketClient ws{QUrl{app_settings.wsUrl()}};

    AuthController auth(app_settings, identity_keys);
    MessengerController messenger(app_settings, identity_keys);

    QQmlApplicationEngine engine;

    QObject::connect(
        &auth,
        &AuthController::authFinished,
        &messenger,
        [&messenger,
        &ws = ws,
        &as = app_settings](bool success) {
            if (!success) {
                return;
            }

            messenger.loadConversations();
            ws.run(as.accessToken());
        }
    );

    QObject::connect(
        &ws,
        &WebSocketClient::messageReceived,
        &messenger,
        &MessengerController::onMessageReceived
    );

    engine.rootContext()->setContextProperty("authController", &auth);
    engine.rootContext()->setContextProperty("messengerController", &messenger);

    engine.loadFromModule("TrueSight", "Main");

    return app.exec();
}
