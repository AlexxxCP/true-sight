#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QJsonObject>

#include "app_settings.hpp"
#include "controllers/auth.hpp"
#include "controllers/messenger.hpp"
#include "stores/identity_keys.hpp"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    AppSettings app_settings;

    IdentityKeys identity_keys;
    AuthController auth(app_settings, identity_keys);
    MessengerController messenger(app_settings, identity_keys);

    QQmlApplicationEngine engine;

    QObject::connect(
        &auth,
        &AuthController::authFinished,
        &messenger,
        [&messenger](bool success) {
            if (success) {
                messenger.loadConversations();
            }
        }
    );

    engine.rootContext()->setContextProperty("authController", &auth);
    engine.rootContext()->setContextProperty("messengerController", &messenger);

    engine.loadFromModule("TrueSight", "Main");

    return app.exec();
}
