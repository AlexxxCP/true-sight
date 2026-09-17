BEGIN;

ALTER TABLE messages DROP CONSTRAINT messages_v2_signature_check;
ALTER TABLE messages DROP COLUMN signature;

COMMIT;
