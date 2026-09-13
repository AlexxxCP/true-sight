#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <private_key.pem> <challenge_file>" >&2
    exit 1
fi

PRIVATE_KEY="$1"
CHALLENGE_FILE="$2"

if command -v brew >/dev/null 2>&1 && [ -x "$(brew --prefix openssl@3 2>/dev/null)/bin/openssl" ]; then
    OPENSSL="$(brew --prefix openssl@3)/bin/openssl"
else
    OPENSSL="$(command -v openssl)"
fi

SIGNATURE_FILE="$(mktemp)"
trap 'rm -f "$SIGNATURE_FILE"' EXIT

"$OPENSSL" pkeyutl \
    -sign \
    -rawin \
    -inkey "$PRIVATE_KEY" \
    -in "$CHALLENGE_FILE" \
    -out "$SIGNATURE_FILE"

"$OPENSSL" base64 -A -in "$SIGNATURE_FILE" \
    | tr '+/' '-_' \
    | tr -d '='

printf '\n'
