#pragma once

#include "app_settings.hpp"
#include "http/http_client.hpp"
#include "qcorotask.h"
#include "stores/identity_keys.hpp"

#include <QObject>
#include <QString>
#include <QVariantList>
#include <cstdint>
#include <set>

class MessengerController final : public QObject {
    Q_OBJECT

    Q_PROPERTY(
        QVariantList conversations
        READ conversations
        NOTIFY conversationsChanged
    )

    Q_PROPERTY(
        QVariantList messages
        READ messages
        NOTIFY messagesChanged
    )

    Q_PROPERTY(
        QString peer
        READ peer
        NOTIFY peerChanged
    )

public:
    explicit MessengerController(
        AppSettings& app_settings,
        IdentityKeys& identity_keys,
        QObject* parent = nullptr
    );

    QVariantList conversations() const;
    QVariantList messages() const;
    QString peer() const;
    void onMessageReceived(const QString& peer);

    void loadConversations();
    void onAuthenticated();
    void resetSession();

    Q_INVOKABLE void openConversation(QString peer);
    Q_INVOKABLE void sendMessage(QString message);
    Q_INVOKABLE QString addConversationFromShare(const QUrl& file);

signals:
    void conversationsChanged();
    void messagesChanged();
    void peerChanged();
    void messageSent();
    void conversationsLoadFailed(const QString& error);
    void messagesLoadFailed(const QString& error);
    void messageSentFailed(const QString& error);

private:
    QCoro::Task<> loadConversationsAsync();
    QCoro::Task<> loadMessagesAsync(QString peer);
    QCoro::Task<> sendMessageAsync(QString peer, QString message);
    std::uint64_t clock(const QString& peer) const;
    std::uint64_t tick(const QString& peer, std::uint64_t observed = 0);

    struct Message {
        QString senderIid;
        QString receiverIid;
        QString text;
        QString createdAt;
    };

    IdentityKeys& identity_keys_;
    AppSettings& app_settings_;
    HttpClient http_client_;
    QVariantList conversations_;
    QVariantList messages_;
    QString peer_;
    std::uint64_t load_generation_ = 0;
    std::uint64_t session_generation_ = 0;
    std::set<QString> imported_peers_;

};
