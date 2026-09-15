#include "controllers/messenger.hpp"
#include "crypto/crypto.hpp"
#include "stores/identity_keys.hpp"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QUrl>
#include <QUrlQuery>

#include <stdexcept>
#include <utility>

MessengerController::MessengerController(
    AppSettings& app_settings,
    IdentityKeys& identity_keys,
    QObject* parent
)
    : QObject(parent),
      app_settings_(app_settings),
      identity_keys_(identity_keys)
{
}

QVariantList MessengerController::conversations() const
{
    return conversations_;
}

QVariantList MessengerController::messages() const
{
    return messages_;
}

QString MessengerController::peer() const
{
    return peer_;
}

void MessengerController::loadConversations()
{
    QCoro::connect(
        loadConversationsAsync(),
        this,
        [] {}
    );
}

void MessengerController::openConversation(QString peer)
{
    if (peer.isEmpty()) {
        return;
    }

    if (peer_ != peer) {
        peer_ = peer;
        emit peerChanged();

        messages_.clear();
        emit messagesChanged();
    }

    QCoro::connect(
        loadMessagesAsync(std::move(peer)),
        this,
        [] {}
    );
}

QCoro::Task<> MessengerController::loadConversationsAsync()
{
    try {
        const auto json = co_await http_client_.get(
            app_settings_.backendUrl() + "/conversations",
            app_settings_.accessToken()
        );

        const QJsonValue value = json.value("conversations");
        if (!value.isArray()) {
            throw std::runtime_error("conversations is not an array");
        }

        conversations_ = value.toArray().toVariantList();
        emit conversationsChanged();
    } catch (const std::exception& error) {
        emit conversationsLoadFailed(QString::fromUtf8(error.what()));
    }

    co_return;
}

QCoro::Task<> MessengerController::loadMessagesAsync(QString peer)
{
    try {
        QUrl url{app_settings_.backendUrl() + "/messages"};
        QUrlQuery query;
        query.addQueryItem("from", peer);
        query.addQueryItem("limit", "100");
        url.setQuery(query);

        const auto json = co_await http_client_.get(
            url,
            app_settings_.accessToken()
        );

        const QJsonValue value = json.value("messages");
        if (!value.isArray()) {
            throw std::runtime_error("messages is not an array");
        }

        if (peer != peer_) {
            co_return;
        }

        messages_ = value.toArray().toVariantList();
        emit messagesChanged();
    } catch (const std::exception& error) {
        if (peer == peer_) {
            emit messagesLoadFailed(QString::fromUtf8(error.what()));
        }
    }

    co_return;
}

void MessengerController::sendMessage(QString message)
{
    QCoro::connect(
        sendMessageAsync(
            peer_,
            std::move(message)
        ),
        this,
        [] {}
    );
}

QCoro::Task<> MessengerController::sendMessageAsync(QString peer, QString message) {
    try {
        auto other_user_keys = identity_keys_.get_user_pk(peer.toStdString());
        if (!other_user_keys.has_value()) {
            throw std::runtime_error("peer not found");
        }

        auto other_user_pk = other_user_keys->x25519_pk;
        auto our_sk = identity_keys_.x25519_sk();

        const auto Z = crypto::x25519_shared_secret(
            std::span<const uint8_t, 32>(
              our_sk.data(),
              our_sk.size()
            ),
            std::span<const uint8_t, 32>(
              other_user_pk.data(),
              other_user_pk.size()
            )
        );

        std::string info = "true-sight-v1/messages/from"
            + identity_keys_.current_username() + "to" + peer.toStdString();

        const std::string salt = "true-sight-v1/x25519-hkdf";

        const auto encryption_key = crypto::hkdf_sha256(
            Z,
            std::span<const uint8_t>{
                reinterpret_cast<const uint8_t*>(salt.data()),
                salt.size()
            },
            std::span<const uint8_t>{
                reinterpret_cast<const uint8_t*>(info.data()),
                info.size()
            }
        );

        const QByteArray plaintext = message.toUtf8();
        auto encrypted_message = crypto::aes_256_gcm_siv_encrypt(
            encryption_key,
            std::span<const uint8_t> {
                reinterpret_cast<const uint8_t*>(plaintext.constData()),
                static_cast<size_t>(plaintext.size())
            },
            {}
        );

        // {
        //      "to": "Alice",
        //      "nonce": <base64encoded>,
        //      "ciphertext": <base64encoded>,
        //      "auth_tag": <base64encoded>,
        //      "protocol_version": 1,
        //      "message_counter": 2
        // }

        const std::string encoded_nonce =
            crypto::base64url_encode(encrypted_message.nonce);

        const std::string encoded_auth_tag =
            crypto::base64url_encode(encrypted_message.tag);

        const std::string encoded_ciphertext =
            crypto::base64url_encode(encrypted_message.ciphertext);

        const QString nonce =
            QString::fromStdString(encoded_nonce);

        const QString auth_tag =
            QString::fromStdString(encoded_auth_tag);

        const QString ciphertext =
            QString::fromStdString(encoded_ciphertext);

        QJsonObject message_data = {
            {"to", peer},
            {"nonce", nonce },
            {"ciphertext", ciphertext },
            {"auth_tag", auth_tag },
            {"protocol_version", 1},
            {"message_counter", 1}
        };

        QUrl url{app_settings_.backendUrl() + "/messages"};
        auto json = co_await http_client_.post(
            url,
            message_data,
            app_settings_.accessToken()
        );

        const QJsonValue status = json.value("status");
        if (!status.isString() || status.toString() != "ok") {
            throw std::runtime_error("sending message failed");
        }

        if (peer != peer_) {
            co_return;
        }

        co_await loadMessagesAsync(peer);

        emit messageSent();
    } catch (const std::exception& error) {
        if (peer == peer_) {
            emit messageSentFailed(QString::fromUtf8(error.what()));
        }
    }

    co_return;
}

