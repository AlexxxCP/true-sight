BEGIN;

CREATE TABLE user_to_pk(
    id UUID PRIMARY KEY,
    user_iid TEXT NOT NULL UNIQUE, -- Because it is UNIQUE a B-tree is created automatically
    user_pk TEXT NOT NULL,

    created_at TIMESTAMPTZ NOT NULL default NOW()
);

COMMIT;
