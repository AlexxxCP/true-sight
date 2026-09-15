#pragma once

#include "qcorotask.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QNetworkAccessManager>

class HttpClient : public QObject {
public:
    explicit HttpClient(QObject* parent = nullptr);

    QCoro::Task<QJsonObject> get(
        const QUrl& url,
        const QString& jwt_token = {}
    );

    QCoro::Task<QJsonObject> post(
        const QUrl& url,
        const QJsonObject& body,
        const QString& jwt_token = {}
    );

private:
    QNetworkAccessManager network_;
};
