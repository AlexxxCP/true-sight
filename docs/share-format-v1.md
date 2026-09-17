# Share file format (v1)

A `.share` file is UTF-8 JSON:

```json
{
  "v": 1,
  "iid": "bob",
  "x25519_pk": "<base64>",
  "x25519_sig": "<base64>",
  "ed25519_pk": "<base64>",
  "ed25519_sig": "<base64>"
}
```

Public keys are their 32 raw bytes, encoded with standard padded Base64. Each
signature is 64 raw Ed25519 signature bytes, encoded the same way. Both signatures
are made with the private key corresponding to `ed25519_pk`.

The signed byte string for each key is:

```text
UTF8("true-sight-share-v1/" + key_type) || 0x00 || UTF8(iid) || 0x00 || raw_public_key
```

`key_type` is exactly `x25519` or `ed25519`. Import verifies both signatures
before storing the peer keys. The signatures bind the two keys to the claimed
IID within this file; the recipient must still obtain the file from a trusted
source to know it belongs to that person.
