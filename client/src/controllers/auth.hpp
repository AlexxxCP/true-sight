#pragma once

#include "app_settings.hpp"
#include "http/http_client.hpp"
#include "qcorotask.h"
#include "stores/identity_keys.hpp"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <cstdint>

class AuthController : public QObject {
    Q_OBJECT

public:
    explicit AuthController(
        AppSettings& app_settings,
        IdentityKeys& identity_keys,
        QObject* parent = nullptr
    )
        : QObject(parent),
          app_settings_(app_settings),
          identity_keys_(identity_keys)
    {}

    Q_INVOKABLE void auth(
        QString username,
        QUrl auth_file
    );
    Q_INVOKABLE void autoLogin();
    Q_INVOKABLE QString logout();

private:
    QCoro::Task<> authAsync(
        QString username,
        std::vector<uint8_t> private_key,
        QUrl private_key_path,
        std::uint64_t generation
    );

    HttpClient http_;
    AppSettings& app_settings_;
    IdentityKeys& identity_keys_;
    std::uint64_t generation_ = 0;

signals:
    void authFinished(bool success);
    void authFailed(const QString& error);
    void loggedOut();
};
