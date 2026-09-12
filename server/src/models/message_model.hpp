#pragma once

#include "db/database.hpp"

class MessageModel {

struct Message {
    std::string message_id;
    std::string sender_iid;
    std::string receiver_iid;

    std::vector<std::byte> nonce;
    std::vector<std::byte> ciphertext;
    std::vector<std::byte> encrypted_data_key;
    std::vector<std::byte> signature;

    u32 crypto_version;
};

public:
    explicit MessageModel(Database& db)
    : db_{db} {}

    std::vector<Message> get_conversation(std::string sender_iid, std::string receiver_iid, u32 limit, u32 offset);

private:
    Database& db_;

};
