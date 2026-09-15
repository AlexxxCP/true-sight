#include "controllers/auth.hpp"
#include "crypto/crypto.hpp"

#include <iostream>
#include <QJsonObject>

QCoro::Task<> AuthController::authAsync(
    QString username,
    std::vector<uint8_t> ed25519_sk
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

        auto signature
            = crypto::sign_ed25519(ed25519_sk, challenge_span);

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
    identity_keys_.load_from_bundle(private_key_path);
    identity_keys_.set_current_username(username.toStdString());

    QCoro::connect(
        authAsync(
            std::move(username),
            identity_keys_.ed25519_sk()
        ),
        this,
        []{}
    );
};
