#pragma once

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

    void post(
        const QUrl& url,
        const QJsonObject& body,
        std::function<void(QJsonObject)> on_success,
        std::function<void(QString)> on_error
    );

private:
    QNetworkAccessManager network_;
};
