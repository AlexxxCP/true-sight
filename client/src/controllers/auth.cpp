#include "controllers/auth.hpp"
#include "crypto/crypto.hpp"

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

Q_INVOKABLE bool AuthController::auth(
    const QString& username,
    const QUrl& private_key_path
) {
    app_settings_.setUsername(username);

    QJsonObject get_challenge_body{
        {"user_iid", username}
    };

    http_.post(
        app_settings_.backendUrl(),
        get_challenge_body,
        [this, private_key_path](const QJsonObject& json) {
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

            auto signature = crypto::sign_ed25519(path, challenge_span);
            auto encoded_sig = crypto::base64url_encode(signature);

            QJsonObject answer_challenge_body {
                {"challege_id", challege_id},
                {"signed_challenge", QString::fromStdString(encoded_sig)}
            };

            http_.post(
                app_settings_.backendUrl(),
                answer_challenge_body,
                [](const QJsonObject& json){

                },
                [](const QString& error){

                }
            );
        },
        [](const QString& error) {
            std::cerr
                << error.toStdString()
                << '\n';
        }
    );
};
