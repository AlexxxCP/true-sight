#pragma once

#include "app_settings.hpp"
#include "http/http_client.hpp"
#include "qcorotask.h"
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

    Q_INVOKABLE void auth(
        QString username,
        QUrl private_key_path
    );

private:
    QCoro::Task<> authAsync(
        QString username,
        QUrl private_key_path
    );

    HttpClient http_;
    AppSettings& app_settings_;

signals:
    void authFinished(bool success);
    void authFailed(const QString& error);
};
