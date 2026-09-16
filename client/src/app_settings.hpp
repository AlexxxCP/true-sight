#pragma once

#include <QSettings>
#include <cstdlib>
#include <stdexcept>

class AppSettings {

public:
    AppSettings() {
        QByteArray backend_url = qgetenv("BACKEND_URL");

        if (backend_url.isEmpty()) {
            backend_url = QByteArrayLiteral("http://localhost:8888");
        }

        settings_.setValue(
            "backend_url",
            QString::fromUtf8(backend_url)
        );
    }

    QString accessToken() const;
    void setAccessToken(const QString& token);

    QString username() const;
    void setUsername(const QString& username);
    QString backendUrl() const;

    bool loggedIn() const;
    void setLoggedIn(bool);

private:
    QSettings settings_{"TrueSight", "TrueSightClient"};

};
