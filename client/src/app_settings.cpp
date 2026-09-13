#include "app_settings.hpp"

#include "app_settings.hpp"

QString AppSettings::accessToken() const {
    return settings_.value("access_token").toString();
}

void AppSettings::setAccessToken(const QString& token) {
    settings_.setValue("access_token", token);
}

QString AppSettings::username() const {
    return settings_.value("username").toString();
}

QString AppSettings::backendUrl() const {
    return settings_.value("backend_url").toString();
}

void AppSettings::setUsername(const QString& username) {
    settings_.setValue("username", username);
}
