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

QString AppSettings::savedUsername() const {
    return settings_.value("saved_login/username").toString();
}

QUrl AppSettings::savedPrivateKeyPath() const {
    return QUrl(settings_.value("saved_login/private_key_path").toString());
}

void AppSettings::saveLogin(const QString& username, const QUrl& private_key_path) {
    settings_.setValue("saved_login/username", username);
    settings_.setValue("saved_login/private_key_path", private_key_path.toString());
    settings_.sync();
    if (settings_.status() != QSettings::NoError) {
        throw std::runtime_error("Could not save login settings");
    }
}

void AppSettings::clearSavedLogin() {
    settings_.remove("saved_login");
    settings_.remove("logged_in");
    settings_.sync();
    if (settings_.status() != QSettings::NoError) {
        throw std::runtime_error("Could not clear login settings");
    }
}

void AppSettings::setUsername(const QString& username) {
    username_ = username;
}
