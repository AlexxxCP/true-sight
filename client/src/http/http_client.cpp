#include "http/http_client.hpp"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QCoroNetworkReply>

HttpClient::HttpClient(QObject* parent)
    : QObject(parent) {}


QCoro::Task<QJsonObject> HttpClient::get(
    const QUrl& url,
    const QString& jwt_token
) {
    QNetworkRequest request{url};
    if (!jwt_token.isEmpty()) {
        request.setRawHeader(
            "Authorization",
            QByteArray("Bearer ") + jwt_token.toUtf8()
        );
    }

    QNetworkReply* reply =
        co_await network_.get(request);

    if (reply->error() != QNetworkReply::NoError) {
        QString error = reply->errorString();
        reply->deleteLater();

        throw std::runtime_error(error.toStdString());
    }

    QJsonParseError parse_error;

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parse_error);

    reply->deleteLater();

    if (parse_error.error != QJsonParseError::NoError) {
        throw std::runtime_error(
            parse_error.errorString().toStdString()
        );
    }

    if (!document.isObject()) {
        throw std::runtime_error(
            "Expected JSON object"
        );
    }

    co_return document.object();
}

QCoro::Task<QJsonObject> HttpClient::post(
    const QUrl& url,
    const QJsonObject& body,
    const QString& jwt_token
) {
    QNetworkRequest request{url};
    if (!jwt_token.isEmpty()) {
        request.setRawHeader(
            "Authorization",
            QByteArray("Bearer ") + jwt_token.toUtf8()
        );
    }

    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/json"
    );

    QByteArray payload =
        QJsonDocument{body}.toJson(QJsonDocument::Compact);

    QNetworkReply* reply =
        co_await network_.post(request, payload);

    if (reply->error() != QNetworkReply::NoError) {
        QString error = reply->errorString();
        reply->deleteLater();

        throw std::runtime_error(error.toStdString());
    }

    QJsonParseError parse_error;

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parse_error);

    reply->deleteLater();

    if (parse_error.error != QJsonParseError::NoError) {
        throw std::runtime_error(
            parse_error.errorString().toStdString()
        );
    }

    if (!document.isObject()) {
        throw std::runtime_error(
            "Expected JSON object"
        );
    }

    co_return document.object();
}
