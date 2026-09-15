#pragma once

#include <optional>
#include <stdint.h>
#include <unordered_map>
#include <vector>
#include <QUrl>

class IdentityKeys {

public:
    struct Keys {
        std::vector<uint8_t> ed25519_pk;
        std::vector<uint8_t> x25519_pk;
    };

    void load_from_bundle(const QUrl& path);

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
        ed25519_sk_.clear();
        ed25519_pk_.clear();
        x25519_pk_.clear();
        x25519_sk_.clear();
    }

private:
    std::vector<uint8_t> ed25519_pk_;
    std::vector<uint8_t> ed25519_sk_;

    std::vector<uint8_t> x25519_pk_;
    std::vector<uint8_t> x25519_sk_;

    std::string current_username_;

    std::unordered_map<std::string, Keys> other_users_ = {
        {
            "test-user",
            IdentityKeys::Keys{
                .ed25519_pk = {
                    0x7f,             0x45,             0x76,             0xe0,             0xeb,             0x65,             0x96,             0xe4,
                    0x72,             0x16,             0xd1,             0xa5,             0xcf,             0x44,             0xc2,             0xb5,
                    0xa2,             0x42,             0x12,             0xba,             0xa7,             0x74,             0x48,             0xaf,
                    0x5e,             0x16,             0x14,             0x60,             0xa5,             0xf8,             0x80,             0xdc
                },
                .x25519_pk = {
                    0x5e,             0xb2,             0x18,             0xb2,             0x5d,             0xa0,             0x45,             0x82,
                    0x23,             0x1d,             0x81,             0x00,             0xed,             0xe7,             0xa1,             0xe5,
                    0x2d,             0x79,             0x91,             0xf2,             0xd2,             0x62,             0x0c,             0xb8,
                    0xb2,             0x61,             0x03,             0xa4,             0x80,             0xf1,             0xf6,             0x6b
                }
            }
        },
        {
            "bob",
            IdentityKeys::Keys{
                .ed25519_pk = {
                    0x7a,             0x97,             0x44,             0x05,             0x29,             0x48,             0x15,             0x03,
                    0x6c,             0xd1,             0xaa,             0x89,             0x4c,             0xc9,             0xaf,             0x51,
                    0x54,             0x9a,             0xc9,             0x4f,             0xf5,             0x2a,             0x28,             0xac,
                    0xb1,             0x06,             0xb0,             0x9c,             0x08,             0x8a,             0xa6,             0x15
                },
                .x25519_pk = {
                    0x44,             0x39,             0x7e,             0xf5,             0x7e,             0x75,             0x45,             0x18,
                    0x24,             0x35,             0x0e,             0x87,             0xfa,             0x32,             0x1b,             0xbc,
                    0xd1,             0xe1,             0xc0,             0xc7,             0x20,             0x54,             0xb5,             0xb4,
                    0xaa,             0x65,             0x1d,             0x8b,             0x3e,             0x55,             0x33,             0x00
                }
            }
        },
    };
 };
