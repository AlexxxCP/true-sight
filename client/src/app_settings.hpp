#pragma once

#include <QSettings>
#include <cstdlib>
#include <stdexcept>

class AppSettings {

public:
    AppSettings() {
        QByteArray backend_url = qgetenv("BACKEND_URL");
        QByteArray ws_url = qgetenv("WS_URL");

        if (backend_url.isEmpty()) {
            backend_url = QByteArrayLiteral("http://localhost:8888");
        }

        if (ws_url.isEmpty()) {
            ws_url = QByteArrayLiteral("ws://localhost:8888/ws");
        }

        settings_.setValue(
            "backend_url",
            QString::fromUtf8(backend_url)
        );

        settings_.setValue(
            "ws_url",
            QString::fromUtf8(ws_url)
        );
    }

    QString accessToken() const;
    void setAccessToken(const QString& token);

    QString username() const;
    void setUsername(const QString& username);
    QString backendUrl() const;
    QString wsUrl() const;

    bool loggedIn() const;
    void setLoggedIn(bool);

private:
    QSettings settings_{"TrueSight", "TrueSightClient"};
    QString access_token_;
    QString username_;

};
