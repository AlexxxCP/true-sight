# Signed message ordering (version 2)

Each client keeps a separate Lamport counter for each peer under its Ed25519
identity. The counter is stored locally before a send. After verifying a
received message with clock `r`, the client ensures its next send to that peer
will have a value greater than `r`. Messages in other conversations do not
advance this conversation's clock. Existing identity-wide counters seed each
conversation once when it first writes its own counter.
Counters are positive signed 64-bit integers on the wire.

The sender signs this byte sequence with its Ed25519 private key:

1. Each byte string is encoded as its length (unsigned 64-bit, big endian),
   followed by its bytes. Integers are unsigned 64-bit, big endian.
2. Encode the byte string `true-sight/message-envelope/v2`.
3. Encode integer `2` (protocol version).
4. Encode the sender and receiver IDs as UTF-8 byte strings, in that order.
5. Encode the Lamport counter as an integer.
6. Encode the raw nonce, ciphertext, and authentication tag as byte strings,
   in that order.

The signature is sent as unpadded base64url. A client verifies it against the
sender's trusted Ed25519 public key before using the clock or displaying the
plaintext. It sorts verified messages by `(clock, sender ID, signature)`. The
signature provides a stable tie breaker for concurrent messages. The server
stores and relays the signature, but its ordering and timestamps are not
trusted for display order.

Version 1 messages have no signature and cannot be authenticated retroactively.
The version 2 client rejects a conversation page containing one; migration
000009 preserves those rows but does not convert them. Apply the migration
before deploying the version 2 server and client together.

This scheme does not prove that the server delivered every message. It also
assumes one active writer for a given identity; installations sharing the same
private key do not coordinate their local counters.
