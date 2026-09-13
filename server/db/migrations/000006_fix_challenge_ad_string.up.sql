BEGIN;

ALTER TABLE auth_challenge ALTER COLUMN challenge TYPE TEXT USING convert_from(challenge, 'UTF8');

COMMIT;
