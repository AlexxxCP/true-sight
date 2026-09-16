#include "app_settings.hpp"

#include "app_settings.hpp"



QString AppSettings::accessToken() const {
    return access_token_;
}

void AppSettings::setAccessToken(const QString& token) {
    access_token_ = token;
}

void AppSettings::setLoggedIn(bool status) {
    settings_.setValue("logged_in", status);
}

bool AppSettings::loggedIn() const {
    return settings_.value("logged_in", false).toBool();
}

QString AppSettings::username() const {
    return username_;
}

QString AppSettings::backendUrl() const {
    return settings_.value("backend_url").toString();
}

QString AppSettings::wsUrl() const {
    return settings_.value("ws_url").toString();
}

void AppSettings::setUsername(const QString& username) {
    username_ = username;
}
