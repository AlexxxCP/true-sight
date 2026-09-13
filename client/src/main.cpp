#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QJsonObject>

#include "controllers/auth.hpp"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    AuthController auth;

    engine.rootContext()->setContextProperty("Auth", &auth);

    engine.loadFromModule("TrueSight", "Main");

    return app.exec();
}
