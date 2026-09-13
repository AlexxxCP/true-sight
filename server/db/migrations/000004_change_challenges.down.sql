BEGIN;

ALTER TABLE auth_challenge
DROP COLUMN is_used;

COMMIT;
