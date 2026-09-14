#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QJsonObject>

#include "app_settings.hpp"
#include "controllers/auth.hpp"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    AppSettings app_settings;

    AuthController auth(app_settings);

    engine.rootContext()->setContextProperty("authController", &auth);

    engine.loadFromModule("TrueSight", "Main");

    return app.exec();
}
