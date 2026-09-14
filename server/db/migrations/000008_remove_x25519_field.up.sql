BEGIN;

ALTER TABLE user_to_pk
DROP COLUMN x25519_pk;

ALTER TABLE messages
DROP COLUMN encrypted_data_key,
DROP COLUMN signature,
DROP COLUMN crypto_version;

ALTER TABLE messages
ADD COLUMN auth_tag BYTEA NOT NULL,
ADD COLUMN protocol_version SMALLINT NOT NULL DEFAULT 1,
ADD COLUMN message_counter BIGINT NOT NULL;

COMMIT;
