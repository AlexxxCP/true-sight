import { gcmsiv } from '@noble/ciphers/aes.js'

const encoder = new TextEncoder()
const decoder = new TextDecoder('utf-8', { fatal: true })
const prefixes = {
  Ed25519: '302a300506032b6570032100',
  X25519: '302a300506032b656e032100',
}

export const utf8 = (text) => encoder.encode(text)

export function concat(...parts) {
  const result = new Uint8Array(parts.reduce((length, part) => length + part.length, 0))
  let offset = 0
  for (const part of parts) {
    result.set(part, offset)
    offset += part.length
  }
  return result
}

export function sameBytes(left, right) {
  return left.length === right.length && left.every((byte, index) => byte === right[index])
}

export function base64(data) {
  let binary = ''
  for (const byte of data) binary += String.fromCharCode(byte)
  return btoa(binary)
}

export function base64url(data) {
  return base64(data).replace(/\+/g, '-').replace(/\//g, '_').replace(/=+$/, '')
}

export function decodeBase64(text, length, url = false) {
  if (typeof text !== 'string' || !text) throw new Error('Missing encoded bytes')
  let raw
  try {
    const normalized = url ? text.replace(/-/g, '+').replace(/_/g, '/') : text
    raw = Uint8Array.from(atob(normalized), (character) => character.charCodeAt(0))
  } catch {
    throw new Error('Invalid base64')
  }
  if ((length !== null && raw.length !== length) ||
      (url ? base64url(raw) : base64(raw)) !== text) {
    throw new Error('Invalid encoded bytes')
  }
  return raw
}

function toPem(label, data) {
  const lines = base64(data).match(/.{1,64}/g).join('\n')
  return `-----BEGIN ${label}-----\n${lines}\n-----END ${label}-----\n`
}

function fromPem(text, label) {
  if (typeof text !== 'string' || !text.includes(`BEGIN ${label}`)) {
    throw new Error('Invalid PEM key')
  }
  return decodeBase64(text.replace(/-----BEGIN [^-]+-----|-----END [^-]+-----|\s/g, ''), null)
}

function rawPublic(pem, type) {
  const der = fromPem(pem, 'PUBLIC KEY')
  const prefix = Uint8Array.from(prefixes[type].match(/../g), (hex) => parseInt(hex, 16))
  if (der.length !== prefix.length + 32 || !sameBytes(der.slice(0, prefix.length), prefix)) {
    throw new Error(`Invalid ${type} public key`)
  }
  return der.slice(-32)
}

function importPublic(raw, type) {
  return crypto.subtle.importKey('raw', raw, { name: type }, false,
    type === 'Ed25519' ? ['verify'] : [])
}

export async function generateBundle() {
  const ed = await crypto.subtle.generateKey('Ed25519', true, ['sign', 'verify'])
  const x = await crypto.subtle.generateKey('X25519', true, ['deriveBits'])
  const exported = async (format, key, label) =>
    toPem(label, new Uint8Array(await crypto.subtle.exportKey(format, key)))
  return {
    format: 'truesight-key-bundle', version: 1,
    keys: {
      ed25519_pk: await exported('spki', ed.publicKey, 'PUBLIC KEY'),
      ed25519_sk: await exported('pkcs8', ed.privateKey, 'PRIVATE KEY'),
      x25519_pk: await exported('spki', x.publicKey, 'PUBLIC KEY'),
      x25519_sk: await exported('pkcs8', x.privateKey, 'PRIVATE KEY'),
    },
  }
}

export async function parseBundle(bundle) {
  if (bundle?.format !== 'truesight-key-bundle' || bundle.version !== 1 || !bundle.keys) {
    throw new Error('Unsupported .tskey file')
  }
  const keys = bundle.keys
  const edPrivate = await crypto.subtle.importKey('pkcs8', fromPem(keys.ed25519_sk, 'PRIVATE KEY'),
    { name: 'Ed25519' }, false, ['sign'])
  const xPrivate = await crypto.subtle.importKey('pkcs8', fromPem(keys.x25519_sk, 'PRIVATE KEY'),
    { name: 'X25519' }, false, ['deriveBits'])
  const edPublic = rawPublic(keys.ed25519_pk, 'Ed25519')
  const xPublic = rawPublic(keys.x25519_pk, 'X25519')
  const probe = utf8('true-sight-key-check')
  const signature = await crypto.subtle.sign('Ed25519', edPrivate, probe)
  if (!await crypto.subtle.verify('Ed25519', await importPublic(edPublic, 'Ed25519'),
    signature, probe)) {
    throw new Error('Ed25519 keypair does not match')
  }
  const basePoint = new Uint8Array(32)
  basePoint[0] = 9
  const derived = new Uint8Array(await crypto.subtle.deriveBits({
    name: 'X25519', public: await importPublic(basePoint, 'X25519'),
  }, xPrivate, 256))
  if (!sameBytes(derived, xPublic)) throw new Error('X25519 keypair does not match')
  return { edPrivate, xPrivate, edPublic, xPublic }
}

function sharePayload(iid, type, key) {
  return concat(utf8(`true-sight-share-v1/${type}`), new Uint8Array([0]),
    utf8(iid), new Uint8Array([0]), key)
}

export async function createShare(iid, keys) {
  const signature = async (type, key) => base64(new Uint8Array(
    await crypto.subtle.sign('Ed25519', keys.edPrivate, sharePayload(iid, type, key))))
  return {
    v: 1, iid,
    x25519_pk: base64(keys.xPublic),
    x25519_sig: await signature('x25519', keys.xPublic),
    ed25519_pk: base64(keys.edPublic),
    ed25519_sig: await signature('ed25519', keys.edPublic),
  }
}

export async function parseShare(value) {
  if (value?.v !== 1 || typeof value.iid !== 'string' || !value.iid.trim()) {
    throw new Error('Unsupported .share file')
  }
  const ed = decodeBase64(value.ed25519_pk, 32)
  const x = decodeBase64(value.x25519_pk, 32)
  const edSignature = decodeBase64(value.ed25519_sig, 64)
  const xSignature = decodeBase64(value.x25519_sig, 64)
  const signer = await importPublic(ed, 'Ed25519')
  if (!await crypto.subtle.verify('Ed25519', signer, edSignature,
    sharePayload(value.iid, 'ed25519', ed)) ||
      !await crypto.subtle.verify('Ed25519', signer, xSignature,
        sharePayload(value.iid, 'x25519', x))) {
    throw new Error('Invalid .share signatures')
  }
  return { iid: value.iid, ed, x }
}

function u64(value) {
  const result = new Uint8Array(8)
  new DataView(result.buffer).setBigUint64(0, BigInt(value))
  return result
}

function field(value) { return concat(u64(value.length), value) }

export function envelope(sender, receiver, counter, nonce, ciphertext, tag) {
  return concat(field(utf8('true-sight/message-envelope/v2')), u64(2),
    field(utf8(sender)), field(utf8(receiver)), u64(counter),
    field(nonce), field(ciphertext), field(tag))
}

export async function messageKey(privateKey, peerRaw, sender, receiver) {
  const shared = await crypto.subtle.deriveBits({
    name: 'X25519', public: await importPublic(peerRaw, 'X25519'),
  }, privateKey, 256)
  const material = await crypto.subtle.importKey('raw', shared, 'HKDF', false, ['deriveBits'])
  return new Uint8Array(await crypto.subtle.deriveBits({
    name: 'HKDF', hash: 'SHA-256', salt: utf8('true-sight-v1/x25519-hkdf'),
    info: utf8(`true-sight-v1/messages/from${sender}to${receiver}`),
  }, material, 256))
}

export function encryptMessage(key, text) {
  const nonce = crypto.getRandomValues(new Uint8Array(12))
  const encrypted = gcmsiv(key, nonce).encrypt(utf8(text))
  return { nonce, ciphertext: encrypted.slice(0, -16), tag: encrypted.slice(-16) }
}

export function decryptMessage(key, nonce, ciphertext, tag) {
  try { return decoder.decode(gcmsiv(key, nonce).decrypt(concat(ciphertext, tag))) }
  catch { return '[Unable to decrypt]' }
}

export async function identityHash(iid, edPublic) {
  const digest = new Uint8Array(await crypto.subtle.digest('SHA-256',
    concat(utf8(iid), new Uint8Array([0]), edPublic)))
  return [...digest].map((byte) => byte.toString(16).padStart(2, '0')).join('')
}
