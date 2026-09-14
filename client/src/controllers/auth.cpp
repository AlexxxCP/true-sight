#include "controllers/auth.hpp"
#include "crypto/crypto.hpp"
#include "impl/connect.h"

#include <iostream>
#include <QJsonObject>

Q_INVOKABLE void AuthController::hello() {
    std::cout << "hello world" << '\n';

    http_.get(
       QUrl{"http://localhost:8888/health-check"},
        [](const QJsonObject& json) {
            std::cout
                << json["status"].toString().toStdString()
                << '\n';
        },
        [](const QString& error) {
            std::cerr
                << error.toStdString()
                << '\n';
        }
    );

    std::cout.flush();
}

QCoro::Task<> AuthController::authAsync(
    QString username,
    QUrl private_key_path
) {
    try {
        app_settings_.setUsername(username);

        QJsonObject get_challenge_body{
            {"user_iid", username}
        };

        auto json = co_await http_.post(
            app_settings_.backendUrl() + "/get-challenge",
            get_challenge_body
        );

        QString challenge = json["challenge"].toString();
        QString challege_id = json["challenge_id"].toString();

        QByteArray challenge_bytes = challenge.toUtf8();

        std::span<const unsigned char> challenge_span{
            reinterpret_cast<const unsigned char*>(challenge_bytes.constData()),
            static_cast<size_t>(challenge_bytes.size())
        };

        std::filesystem::path path{
            private_key_path.toLocalFile().toStdString()
        };

        auto signature
            = crypto::sign_ed25519(path, challenge_span);

        auto encoded_sig
            = crypto::base64url_encode(signature);

        QJsonObject answer_challenge_body {
            {"challenge_id", challege_id},
            {"signed_challenge", QString::fromStdString(encoded_sig)}
        };

        auto response = co_await http_.post(
            app_settings_.backendUrl() +  "/validate-challenge",
            answer_challenge_body
        );

        app_settings_.setAccessToken(response["access_token"].toString());
        std::cout << app_settings_.accessToken().toStdString() << '\n';

        emit authFinished(true);
    } catch (const std::exception& e) {
        emit authFailed(QString::fromUtf8(e.what()));
    }
};

Q_INVOKABLE void AuthController::auth(
    QString username,
    QUrl private_key_path
) {
    QCoro::connect(
        authAsync(
            std::move(username),
            std::move(private_key_path)
        ),
        this,
        []{}
    );
};
