#pragma once

#include "qcorotask.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QNetworkAccessManager>

#include <functional>

class HttpClient : public QObject {
public:
    explicit HttpClient(QObject* parent = nullptr);

    void get(
        const QUrl& url,
        std::function<void(QJsonObject)> on_success,
        std::function<void(QString)> on_error
    );

    QCoro::Task<QJsonObject> post(
        const QUrl& url,
        const QJsonObject& body
    );

private:
    QNetworkAccessManager network_;
};
