#pragma once

#include "app_settings.hpp"
#include "http/http_client.hpp"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>

class AuthController : public QObject {
    Q_OBJECT

public:
    explicit AuthController(
        AppSettings& app_settings,
        QObject* parent = nullptr
    )
        : QObject(parent),
          app_settings_(app_settings)
    {
    }

    Q_INVOKABLE void hello();

    Q_INVOKABLE bool auth(
        const QString& username,
        const QUrl& private_key_path
    );

private:
    HttpClient http_;
    AppSettings& app_settings_;
};
