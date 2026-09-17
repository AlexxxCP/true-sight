#include "controllers/messenger.hpp"
#include "crypto/crypto.hpp"
#include "crypto/messages.hpp"
#include "stores/identity_keys.hpp"
#include "utils/dates.hpp"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QCryptographicHash>
#include <QSettings>
#include <QUrl>
#include <QUrlQuery>

#include <stdexcept>
#include <algorithm>
#include <limits>
#include <set>
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

void MessengerController::resetSession()
{
    ++session_generation_;
    ++load_generation_;
    conversations_.clear();
    messages_.clear();
    peer_.clear();
    imported_peers_.clear();
    emit conversationsChanged();
    emit messagesChanged();
    emit peerChanged();
}

void MessengerController::onAuthenticated()
{
    resetSession();
    for (const auto& user : identity_keys_.peer_usernames()) {
        const auto peer = QString::fromStdString(user);
        imported_peers_.insert(peer);
        QVariantMap row;
        row["peer"] = peer;
        conversations_.push_back(row);
    }
    emit conversationsChanged();
    loadConversations();
}

QString MessengerController::addConversationFromShare(const QUrl& file)
{
    try {
        const QString imported_peer = QString::fromStdString(identity_keys_.import_share(file));
        imported_peers_.insert(imported_peer);

        const auto found = std::any_of(conversations_.begin(), conversations_.end(),
            [&imported_peer](const QVariant& conversation) {
                return conversation.toMap().value("peer").toString() == imported_peer;
            });
        if (!found) {
            QVariantMap row;
            row["peer"] = imported_peer;
            conversations_.prepend(row);
            emit conversationsChanged();
        }
        openConversation(imported_peer);
        return {};
    } catch (const std::exception& error) {
        return QString::fromUtf8(error.what());
    }
}

namespace {
QByteArray clock_identity(const IdentityKeys& keys) {
    QByteArray identity = QByteArray::fromStdString(keys.current_username());
    identity.append('\0');
    const auto public_key = keys.ed25519_pk();
    identity.append(reinterpret_cast<const char*>(public_key.data()), public_key.size());
    return identity;
}

QString legacy_clock_key(const IdentityKeys& keys) {
    return "lamport/" + QString::fromLatin1(
        QCryptographicHash::hash(clock_identity(keys), QCryptographicHash::Sha256).toHex()
    );
}

QString clock_key(const IdentityKeys& keys, const QString& peer) {
    QByteArray identity = clock_identity(keys);
    identity.append('\0');
    identity.append(peer.toUtf8());
    return "lamport/conversation/" + QString::fromLatin1(
        QCryptographicHash::hash(identity, QCryptographicHash::Sha256).toHex()
    );
}
}

std::uint64_t MessengerController::clock(const QString& peer) const {
    QSettings settings{"TrueSight", "TrueSightClient"};
    bool valid = false;
    const auto key = clock_key(identity_keys_, peer);
    // Existing installations had one identity-wide clock. Seed each
    // conversation once so previously signed messages remain ordered.
    const auto stored = settings.contains(key)
        ? settings.value(key) : settings.value(legacy_clock_key(identity_keys_), "0");
    const auto value = stored.toString().toULongLong(&valid);
    if (!valid || value >= static_cast<std::uint64_t>(std::numeric_limits<qint64>::max())) {
        throw std::runtime_error("Invalid or exhausted Lamport clock");
    }
    return value;
}

std::uint64_t MessengerController::tick(const QString& peer, std::uint64_t observed) {
    const auto next = std::max(clock(peer), observed) + 1;
    if (next > static_cast<std::uint64_t>(std::numeric_limits<qint64>::max())) {
        throw std::runtime_error("Lamport clock exhausted");
    }

    QSettings settings{"TrueSight", "TrueSightClient"};
    settings.setValue(clock_key(identity_keys_, peer), QString::number(next));
    settings.sync();
    if (settings.status() != QSettings::NoError) {
        throw std::runtime_error("Could not persist Lamport clock");
    }
    return next;
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
    const auto session = session_generation_;
    try {
        const auto json = co_await http_client_.get(
            app_settings_.backendUrl() + "/conversations",
            app_settings_.accessToken()
        );
        if (session != session_generation_) {
            co_return;
        }

        const QJsonValue value = json.value("conversations");
        if (!value.isArray()) {
            throw std::runtime_error("conversations is not an array");
        }

        QVariantList parsed_conversations;
        for (const auto& conversation : value.toArray()) {
            if (!conversation.isObject())
                throw std::runtime_error("Conversation is not an object");

            const QJsonObject object = conversation.toObject();
            const QJsonValue peer_value = object.value("peer");
            const QJsonValue last_message_at_value =
                object.value("last_message_at");
            if (!peer_value.isString() ||
                !last_message_at_value.isString()) {
                throw std::runtime_error("Conversation fields are missing");
            }

            QVariantMap row;
            row["peer"] = peer_value.toString();
            row["last_message_at"] =
                utils::parseDate(last_message_at_value.toString());
            parsed_conversations.push_back(row);
        }

        for (const auto& imported_peer : imported_peers_) {
            const auto found = std::any_of(parsed_conversations.begin(), parsed_conversations.end(),
                [&imported_peer](const QVariant& conversation) {
                    return conversation.toMap().value("peer").toString() == imported_peer;
                });
            if (!found) {
                QVariantMap row;
                row["peer"] = imported_peer;
                parsed_conversations.push_back(row);
            }
        }
        conversations_ = std::move(parsed_conversations);
        emit conversationsChanged();
    } catch (const std::exception& error) {
        if (session == session_generation_) {
            emit conversationsLoadFailed(QString::fromUtf8(error.what()));
        }
    }

    co_return;
}

QCoro::Task<> MessengerController::loadMessagesAsync(QString peer)
{
    const auto generation = ++load_generation_;
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

        if (peer != peer_ || generation != load_generation_) {
            co_return;
        }

        const QString me = QString::fromStdString(identity_keys_.current_username());

        auto other_user_keys = identity_keys_.get_user_pk(peer.toStdString());
        if (!other_user_keys.has_value()) {
            throw std::runtime_error("peer not found");
        }

        auto other_user_pk = other_user_keys->x25519_pk;
        auto our_sk = identity_keys_.x25519_sk();

        auto messages = value.toArray();
        std::vector<Message> parsed_messages;

        std::string their_msg = "true-sight-v1/messages/from"
            + peer.toStdString() + "to" + identity_keys_.current_username();

        std::string my_msg = "true-sight-v1/messages/from"
            + identity_keys_.current_username() + "to" + peer.toStdString();

        const std::string salt = "true-sight-v1/x25519-hkdf";
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

        const auto my_encryption_key = crypto::hkdf_sha256(
            Z,
            std::span<const uint8_t>{
                reinterpret_cast<const uint8_t*>(salt.data()),
                salt.size()
            },
            std::span<const uint8_t>{
                reinterpret_cast<const uint8_t*>(my_msg.data()),
                my_msg.size()
            }
        );

        const auto their_encryption_key = crypto::hkdf_sha256(
            Z,
            std::span<const uint8_t>{
                reinterpret_cast<const uint8_t*>(salt.data()),
                salt.size()
            },
            std::span<const uint8_t>{
                reinterpret_cast<const uint8_t*>(their_msg.data()),
                their_msg.size()
            }
        );

        struct VerifiedRow {
            std::uint64_t counter;
            QString sender;
            QString signature;
            QVariantMap data;
        };
        std::vector<VerifiedRow> verified_messages;
        std::set<QString> seen_signatures;
        std::uint64_t maximum_received = 0;
        for (const auto& msg : messages) {
            if (!msg.isObject()) {
                throw std::runtime_error("Message is not an object");
            }
            QVariantMap row;

            const QJsonObject object = msg.toObject();
            const QJsonValue sender_value = object.value("sender_iid");
            const QJsonValue receiver_value = object.value("receiver_iid");
            if (!sender_value.isString() || !receiver_value.isString()) {
                throw std::runtime_error("Message participants are missing");
            }

            const QString sender = sender_value.toString();
            const QString receiver = receiver_value.toString();
            const bool sent_by_me = sender == me && receiver == peer;
            const bool sent_by_peer = sender == peer && receiver == me;
            if (!sent_by_me && !sent_by_peer) {
                throw std::runtime_error("Message participants do not match the conversation");
            }

            const auto version_value = object.value("protocol_version");
            const auto counter_value = object.value("message_counter");
            if (!version_value.isDouble() || version_value.toInteger() != 2 ||
                !counter_value.isDouble()) {
                throw std::runtime_error("Unsupported or missing signed message protocol");
            }
            const auto counter = counter_value.toInteger();
            if (counter <= 0) {
                throw std::runtime_error("Invalid Lamport clock");
            }

            auto nonce = crypto::decodeBase64Url(object.value("nonce").toString());
            auto auth_tag = crypto::decodeBase64Url(object.value("auth_tag").toString());
            auto ciphertext = crypto::decodeBase64Url(object.value("ciphertext").toString());
            const auto signature_text = object.value("signature");
            if (!signature_text.isString()) {
                throw std::runtime_error("Message signature is missing");
            }
            auto signature = crypto::decodeBase64Url(signature_text.toString());

            if (nonce.size() != 12 || auth_tag.size() != 16)
                throw std::runtime_error("Invalid nonce or auth tag size");

            const auto envelope = crypto::messages::signed_envelope(
                sender.toStdString(), receiver.toStdString(),
                static_cast<std::uint64_t>(counter), nonce, ciphertext, auth_tag
            );
            const auto signer_key = sent_by_me
                ? identity_keys_.ed25519_pk() : other_user_keys->ed25519_pk;
            if (!crypto::verify_ed25519(signer_key, envelope, signature)) {
                throw std::runtime_error("Message signature verification failed");
            }
            const auto canonical_signature = QString::fromStdString(
                crypto::base64url_encode(signature)
            );
            if (!seen_signatures.insert(canonical_signature).second) {
                continue;
            }
            maximum_received = std::max(maximum_received, static_cast<std::uint64_t>(counter));

            auto decrypted_msg = crypto::aes_256_gcm_siv_decrypt(
                sent_by_me ? my_encryption_key : their_encryption_key,
                std::span<const crypto::u8, 12>{nonce.data(), nonce.size()},
                std::span<const crypto::u8>{ciphertext},
                std::span<const crypto::u8, 16>{auth_tag.data(), auth_tag.size()},
                {}
            );

            if (!decrypted_msg) {
                row["message_text"] = QStringLiteral("[Unable to decrypt]");
            } else {
                row["message_text"] = QString::fromUtf8(
                    reinterpret_cast<const char*>(decrypted_msg->data()),
                    static_cast<qsizetype>(decrypted_msg->size())
                );
            }

            row["created_at"] =
                utils::parseDate(object.value("created_at").toString());
            row["receiver_iid"] = receiver;

            verified_messages.push_back({static_cast<std::uint64_t>(counter),
                                         sender, canonical_signature, row});
        }

        if (peer != peer_ || generation != load_generation_) {
            co_return;
        }
        // A refresh of already observed history is not a new receive event.
        if (maximum_received > 0 && maximum_received >= clock(peer)) {
            tick(peer, maximum_received);
        }

        std::sort(verified_messages.begin(), verified_messages.end(),
            [](const VerifiedRow& a, const VerifiedRow& b) {
                return std::tie(a.counter, a.sender, a.signature) <
                       std::tie(b.counter, b.sender, b.signature);
            });
        QVariantList decrypted_messages;
        for (const auto& verified : verified_messages) {
            decrypted_messages.push_back(verified.data);
        }
        messages_ = std::move(decrypted_messages);
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

void MessengerController::onMessageReceived(const QString& peer) {
    if (peer_ != peer) {
        return;
    }

    QCoro::connect(
        loadMessagesAsync(std::move(peer)),
        this,
        [] {}
    );
}

QCoro::Task<> MessengerController::sendMessageAsync(QString peer, QString message) {
    const auto session = session_generation_;
    try {
        if (peer.isEmpty() || message.isEmpty()) {
            throw std::runtime_error("No recipient or message");
        }
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

        const auto counter = tick(peer);
        const auto envelope = crypto::messages::signed_envelope(
            identity_keys_.current_username(), peer.toStdString(), counter,
            encrypted_message.nonce, encrypted_message.ciphertext, encrypted_message.tag
        );
        const auto signature = crypto::sign_ed25519(identity_keys_.ed25519_sk(), envelope);

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
            {"signature", QString::fromStdString(crypto::base64url_encode(signature))},
            {"protocol_version", 2},
            {"message_counter", static_cast<qint64>(counter)}
        };

        QUrl url{app_settings_.backendUrl() + "/messages"};
        auto json = co_await http_client_.post(
            url,
            message_data,
            app_settings_.accessToken()
        );
        if (session != session_generation_) {
            co_return;
        }

        const QJsonValue status = json.value("status");
        if (!status.isString() || status.toString() != "ok") {
            throw std::runtime_error("sending message failed");
        }

        if (peer != peer_) {
            co_return;
        }

        co_await loadMessagesAsync(peer);

        if (session == session_generation_) {
            emit messageSent();
        }
    } catch (const std::exception& error) {
        if (session == session_generation_ && peer == peer_) {
            emit messageSentFailed(QString::fromUtf8(error.what()));
        }
    }

    co_return;
}
