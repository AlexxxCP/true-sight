BEGIN;

ALTER TABLE auth_challenge
ALTER COLUMN id SET DEFAULT gen_random_uuid();

ALTER TABLE messages
ALTER COLUMN message_id SET DEFAULT gen_random_uuid();

ALTER TABLE user_to_pk
ALTER COLUMN id SET DEFAULT gen_random_uuid();

COMMIT;
