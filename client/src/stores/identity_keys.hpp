#pragma once

#include <optional>
#include <stdint.h>
#include <unordered_map>
#include <vector>
#include <QUrl>
#include <openssl/crypto.h>

class IdentityKeys {

public:
    struct Keys {
        std::vector<uint8_t> ed25519_pk;
        std::vector<uint8_t> x25519_pk;
    };

    void load_from_bundle(const QUrl& path);
    std::string import_share(const QUrl& path);
    void load_saved_peers();
    void forget_saved_peers();
    std::vector<std::string> peer_usernames() const;

    void set_current_username(std::string username) {
        current_username_ = username;
    }

    std::string current_username() const {
        return current_username_;
    }

    std::vector<uint8_t> ed25519_pk() const {
        return ed25519_pk_;
    }

    std::vector<uint8_t> ed25519_sk() const {
        return ed25519_sk_;
    }

    std::vector<uint8_t> x25519_pk() const {
        return x25519_pk_;
    }

    std::vector<uint8_t> x25519_sk() const {
        return x25519_sk_;
    }

    std::optional<Keys> get_user_pk(const std::string& user_iid) const {
        if (!other_users_.contains(user_iid)) {
            return std::nullopt;
        }
        return other_users_.at(user_iid);
    }

    void clear() {
        if (!ed25519_sk_.empty()) {
            OPENSSL_cleanse(ed25519_sk_.data(), ed25519_sk_.size());
        }
        if (!x25519_sk_.empty()) {
            OPENSSL_cleanse(x25519_sk_.data(), x25519_sk_.size());
        }
        ed25519_sk_.clear();
        ed25519_pk_.clear();
        x25519_pk_.clear();
        x25519_sk_.clear();
        other_users_.clear();
        current_username_.clear();
    }

private:
    std::vector<uint8_t> ed25519_pk_;
    std::vector<uint8_t> ed25519_sk_;

    std::vector<uint8_t> x25519_pk_;
    std::vector<uint8_t> x25519_sk_;

    std::string current_username_;

    std::unordered_map<std::string, Keys> other_users_;
    QString peers_settings_key() const;
    void save_peers() const;
 };
