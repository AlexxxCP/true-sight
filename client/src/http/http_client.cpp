#include "http/http_client.hpp"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QCoroNetworkReply>

HttpClient::HttpClient(QObject* parent)
    : QObject(parent) {}


void HttpClient::get(
    const QUrl& url,
    std::function<void(QJsonObject)> on_success,
    std::function<void(QString)> on_error
) {
    QNetworkRequest request{url};

    QNetworkReply* reply = network_.get(request);

    connect(
        reply,
        &QNetworkReply::finished,
        this,
        [reply, on_success = std::move(on_success), on_error = std::move(on_error)]() {
            if (reply->error() != QNetworkReply::NoError) {
                on_error(reply->errorString());
                reply->deleteLater();
                return;
            }

            QJsonParseError parse_error;

            QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parse_error);

            if (parse_error.error != QJsonParseError::NoError) {
                on_error(parse_error.errorString());
                reply->deleteLater();
                return;
            }

            if (!document.isObject()) {
                on_error("Expected JSON object");
                reply->deleteLater();
                return;
            }

            on_success(document.object());

            reply->deleteLater();
        }
    );
}

QCoro::Task<QJsonObject> HttpClient::post(
    const QUrl& url,
    const QJsonObject& body
) {
    QNetworkRequest request{url};

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
