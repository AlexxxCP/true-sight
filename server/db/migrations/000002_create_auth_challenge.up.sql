BEGIN;

CREATE TABLE auth_challenge (
    id UUID PRIMARY KEY,
    user_iid TEXT NOT NULL,
    challenge BYTEA NOT NULL,
    expires_at TIMESTAMPTZ NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX auth_challenges_expires_at_index
    on auth_challenge(expires_at);

CREATE INDEX auth_challenges_user_iid_expires_at_index
    ON auth_challenge(user_iid, expires_at);

COMMIT;
