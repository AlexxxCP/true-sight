#pragma once

#include <QByteArray>
#include <QString>

#include <cstdint>
#include <span>

namespace crypto::share {

inline QByteArray signed_payload(const QString& iid, const char* key_type,
                                 std::span<const std::uint8_t> public_key) {
    QByteArray payload("true-sight-share-v1/");
    payload.append(key_type);
    payload.append('\0');
    payload.append(iid.toUtf8());
    payload.append('\0');
    payload.append(reinterpret_cast<const char*>(public_key.data()), public_key.size());
    return payload;
}

} // namespace crypto::share
