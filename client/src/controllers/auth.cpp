#include "controllers/auth.hpp"

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
        [](const QJsonObject& json) {
            QString challege = json["challenge"].toString();
            QString challege_id = json["challenge_id"].toString();


        },
        [](const QString& error) {
            std::cerr
                << error.toStdString()
                << '\n';
        }
    );


};
