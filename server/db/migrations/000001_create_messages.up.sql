BEGIN;

CREATE TABLE messages (
    message_id UUID PRIMARY KEY,

    sender_iid TEXT NOT NULL,
    receiver_iid TEXT NOT NULL,

    nonce BYTEA NOT NULL,
    ciphertext BYTEA NOT NULL,
    encrypted_data_key BYTEA NOT NULL,

    signature BYTEA NOT NULL,
    crypto_version INTEGER NOT NULL,

    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX message_receiver_idx
    ON messages(receiver_iid, created_at);

CREATE INDEX message_sender_idx
    ON messages(sender_iid, created_at);

CREATE INDEX message_sender_receiver_idx
    ON messages(sender_iid, receiver_iid);

CREATE INDEX message_receiver_sender_idx
    ON messages(receiver_iid, sender_iid);

COMMIT;
