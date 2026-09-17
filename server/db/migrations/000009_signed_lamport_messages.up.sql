BEGIN;

-- Legacy version 1 rows cannot acquire a sender signature retroactively.
ALTER TABLE messages ADD COLUMN signature BYTEA;

ALTER TABLE messages ADD CONSTRAINT messages_v2_signature_check
CHECK (protocol_version <> 2 OR
       (signature IS NOT NULL AND octet_length(signature) = 64 AND message_counter > 0));

COMMIT;
