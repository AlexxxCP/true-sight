#pragma once

#include "app_settings.hpp"
#include "http/http_client.hpp"
#include "qcorotask.h"

#include <QObject>
#include <QUrl>
#include <vector>

class RegistrationController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool shareSaved READ shareSaved NOTIFY downloadsChanged)
    Q_PROPERTY(bool privateKeySaved READ privateKeySaved NOTIFY downloadsChanged)

public:
    explicit RegistrationController(AppSettings& settings, QObject* parent = nullptr);

    bool shareSaved() const { return share_saved_; }
    bool privateKeySaved() const { return private_key_saved_; }

    Q_INVOKABLE QString beginRegistration();
    Q_INVOKABLE void registerUsername(QString username);
    Q_INVOKABLE QString saveShare(const QUrl& destination);
    Q_INVOKABLE QString savePrivateKey(const QUrl& destination);
    Q_INVOKABLE void finishRegistration();

signals:
    void registrationSucceeded(const QString& username);
    void registrationFailed(const QString& error);
    void downloadsChanged();

private:
    QCoro::Task<> registerUsernameAsync(QString username);
    QString saveFile(const QUrl& destination, const QByteArray& content,
                     const QString& suffix);

    AppSettings& settings_;
    HttpClient http_;
    QByteArray ed_public_pem_;
    QByteArray x_public_pem_;
    std::vector<uint8_t> ed_private_raw_;
    QByteArray private_bundle_;
    QByteArray share_file_;
    bool registering_ = false;
    bool registered_ = false;
    bool share_saved_ = false;
    bool private_key_saved_ = false;
};
