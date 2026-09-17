import { onMounted, onUnmounted, reactive, shallowRef } from 'vue'
import { request } from '../lib/api'
import { readJsonFile, saveJsonFile } from '../lib/files'
import { deleteSetting, getSetting, setSetting } from '../lib/storage'
import {
  base64, base64url, createShare, decodeBase64, decryptMessage, encryptMessage,
  envelope, generateBundle, identityHash, messageKey, parseBundle, parseShare,
  sameBytes, utf8,
} from '../lib/crypto'

export function useClient() {
  const view = reactive({
    screen: 'choice', busy: false, error: '', messageError: '', username: '',
    registeredUsername: '', shareSaved: false, keySaved: false,
    conversations: [], peer: '', messages: [],
  })
  const savedHandle = shallowRef(null)
  let registration = null
  let session = null
  let generation = 0
  let loadGeneration = 0
  let pollTimer

  async function startRegistration() {
    registration = { bundle: await generateBundle() }
    registration.keys = await parseBundle(registration.bundle)
    view.shareSaved = false
    view.keySaved = false
    view.error = ''
    view.screen = 'register'
  }

  async function register(username) {
    const iid = username.trim()
    if (!registration || !iid || iid.length > 128) {
      throw new Error('Enter a username (up to 128 characters)')
    }
    view.busy = true
    try {
      const result = await request('/register', {
        user_iid: iid, ed25519_pk: registration.bundle.keys.ed25519_pk,
      })
      if (result.status !== 'ok') throw new Error('Registration failed')
      registration.share = await createShare(iid, registration.keys)
      view.registeredUsername = iid
      view.screen = 'downloads'
    } finally { view.busy = false }
  }

  async function saveRegistration(kind) {
    if (!registration?.share || !['share', 'tskey'].includes(kind)) {
      throw new Error('No registration file available')
    }
    const safeName = view.registeredUsername.replace(/[^A-Za-z0-9._-]/g, '_') || 'user'
    await saveJsonFile(kind === 'share' ? registration.share : registration.bundle,
      `${safeName}.${kind}`)
    if (kind === 'share') view.shareSaved = true
    else view.keySaved = true
  }

  function finishRegistration() {
    if (!view.shareSaved || !view.keySaved) return
    registration = null
    view.screen = 'login'
  }

  async function authenticate(username, file, handle = null) {
    const iid = username.trim()
    if (!iid) throw new Error('Enter your username')
    const current = ++generation
    session = null
    view.busy = true
    try {
      const keys = await parseBundle(await readJsonFile(file, '.tskey'))
      const challenge = await request('/get-challenge', { user_iid: iid })
      if (current !== generation) return
      if (typeof challenge.challenge !== 'string' || typeof challenge.challenge_id !== 'string') {
        throw new Error('Invalid authentication challenge')
      }
      const signature = new Uint8Array(await crypto.subtle.sign('Ed25519', keys.edPrivate,
        utf8(challenge.challenge)))
      const result = await request('/validate-challenge', {
        challenge_id: challenge.challenge_id, signed_challenge: base64url(signature),
      })
      if (current !== generation) return
      if (!result.access_token) throw new Error('Server returned no access token')
      const identity = await identityHash(iid, keys.edPublic)
      const savedPeers = await getSetting(`peers/${identity}`) || []
      const peers = new Map()
      for (const row of savedPeers) {
        if (!row?.iid || peers.has(row.iid)) throw new Error('Saved peer keys are invalid')
        peers.set(row.iid, {
          ed: decodeBase64(row.ed, 32), x: decodeBase64(row.x, 32),
        })
      }
      session = { iid, identity, keys, peers, token: result.access_token }
      if (handle) {
        await setSetting('login', { iid, handle })
        savedHandle.value = handle
      } else {
        await deleteSetting('login')
        savedHandle.value = null
      }
      view.username = iid
      view.screen = 'messenger'
      view.error = ''
      view.peer = ''
      view.messages = []
      view.conversations = [...peers.keys()].map((peer) => ({ peer }))
      try { await loadConversations() }
      catch (error) { view.messageError = error.message }
    } catch (error) {
      if (current === generation) session = null
      throw error
    } finally {
      if (current === generation) view.busy = false
    }
  }

  async function resumeSavedLogin() {
    if (!savedHandle.value) return
    let permission = await savedHandle.value.queryPermission({ mode: 'read' })
    if (permission !== 'granted') {
      permission = await savedHandle.value.requestPermission({ mode: 'read' })
    }
    if (permission !== 'granted') throw new Error('File access was not granted')
    await authenticate(view.username, await savedHandle.value.getFile(), savedHandle.value)
  }

  async function autoLogin() {
    const saved = await getSetting('login')
    if (!saved?.iid || !saved.handle) return
    view.username = saved.iid
    savedHandle.value = saved.handle
    view.screen = 'login'
    try {
      if (await saved.handle.queryPermission({ mode: 'read' }) === 'granted') {
        await resumeSavedLogin()
      }
    } catch (error) { view.error = error.message }
  }

  async function logout() {
    ++generation
    ++loadGeneration
    let failure
    try {
      if (session) await deleteSetting(`peers/${session.identity}`)
    } catch (error) { failure = error }
    try { await deleteSetting('login') }
    catch (error) { failure ||= error }
    session = null
    savedHandle.value = null
    view.screen = 'choice'
    view.username = ''
    view.peer = ''
    view.messages = []
    view.conversations = []
    view.error = ''
    view.messageError = ''
    view.busy = false
    if (failure) throw new Error(`Could not clear saved login: ${failure.message}`)
  }

  async function importConversation(file) {
    if (!session) throw new Error('Log in first')
    const imported = await parseShare(await readJsonFile(file, '.share'))
    if (imported.iid === session.iid) throw new Error('Cannot add your own key')
    const existing = session.peers.get(imported.iid)
    if (existing && (!sameBytes(existing.ed, imported.ed) ||
        !sameBytes(existing.x, imported.x))) {
      throw new Error('Public keys for this user differ from the imported keys')
    }
    session.peers.set(imported.iid, { ed: imported.ed, x: imported.x })
    await setSetting(`peers/${session.identity}`, [...session.peers].map(([iid, keys]) => ({
      iid, ed: base64(keys.ed), x: base64(keys.x),
    })))
    if (!view.conversations.some((row) => row.peer === imported.iid)) {
      view.conversations.unshift({ peer: imported.iid })
    }
    await openConversation(imported.iid)
  }

  async function loadConversations() {
    if (!session) return
    const current = generation
    const result = await request('/conversations', undefined, session.token)
    if (current !== generation) return
    if (!Array.isArray(result.conversations)) throw new Error('Invalid conversation list')
    const rows = result.conversations.map((row) => {
      if (typeof row.peer !== 'string' || typeof row.last_message_at !== 'string') {
        throw new Error('Invalid conversation')
      }
      return { peer: row.peer, last_message_at: row.last_message_at }
    })
    for (const iid of session.peers.keys()) {
      if (!rows.some((row) => row.peer === iid)) rows.push({ peer: iid })
    }
    view.conversations = rows
  }

  async function clock(other) {
    const value = Number(await getSetting(`clock/${session.identity}/${other}`) || 0)
    if (!Number.isSafeInteger(value) || value < 0 || value >= Number.MAX_SAFE_INTEGER) {
      throw new Error('Invalid Lamport clock')
    }
    return value
  }

  async function tick(other, observed = 0) {
    const next = Math.max(await clock(other), observed) + 1
    if (!Number.isSafeInteger(next)) throw new Error('Lamport clock exhausted')
    await setSetting(`clock/${session.identity}/${other}`, next)
    return next
  }

  async function openConversation(other) {
    if (!session || !other) return
    const current = generation
    const load = ++loadGeneration
    if (view.peer !== other) {
      view.peer = other
      view.messages = []
    }
    view.messageError = ''
    try {
      const result = await request('/messages?' + new URLSearchParams({
        from: other, limit: '100',
      }), undefined, session.token)
      if (current !== generation || load !== loadGeneration) return
      if (!Array.isArray(result.messages)) throw new Error('Invalid message list')
      const trusted = session.peers.get(other)
      if (!trusted) throw new Error('Peer public key not found')
      const sentKey = await messageKey(session.keys.xPrivate, trusted.x, session.iid, other)
      const receivedKey = await messageKey(session.keys.xPrivate, trusted.x, other, session.iid)
      const seen = new Set()
      const rows = []
      let maximum = 0
      for (const message of result.messages) {
        const mine = message.sender_iid === session.iid && message.receiver_iid === other
        const theirs = message.sender_iid === other && message.receiver_iid === session.iid
        if (!mine && !theirs) throw new Error('Message participants do not match conversation')
        const counter = message.message_counter
        if (message.protocol_version !== 2 || !Number.isSafeInteger(counter) || counter <= 0) {
          throw new Error('Unsupported or invalid signed message')
        }
        const nonce = decodeBase64(message.nonce, 12, true)
        const tag = decodeBase64(message.auth_tag, 16, true)
        const ciphertext = decodeBase64(message.ciphertext, null, true)
        const signature = decodeBase64(message.signature, 64, true)
        const signerRaw = mine ? session.keys.edPublic : trusted.ed
        const signer = await crypto.subtle.importKey('raw', signerRaw,
          { name: 'Ed25519' }, false, ['verify'])
        const signed = envelope(message.sender_iid, message.receiver_iid,
          counter, nonce, ciphertext, tag)
        if (!await crypto.subtle.verify('Ed25519', signer, signature, signed)) {
          throw new Error('Message signature verification failed')
        }
        const signatureText = base64url(signature)
        if (seen.has(signatureText)) continue
        seen.add(signatureText)
        maximum = Math.max(maximum, counter)
        rows.push({
          message_text: decryptMessage(mine ? sentKey : receivedKey, nonce, ciphertext, tag),
          receiver_iid: message.receiver_iid, sender_iid: message.sender_iid,
          created_at: message.created_at, counter, signature: signatureText,
        })
      }
      if (current !== generation || load !== loadGeneration) return
      if (maximum && maximum >= await clock(other)) await tick(other, maximum)
      const compare = (a, b) => a < b ? -1 : a > b ? 1 : 0
      rows.sort((a, b) => a.counter - b.counter ||
        compare(a.sender_iid, b.sender_iid) || compare(a.signature, b.signature))
      view.messages = rows
    } catch (error) {
      if (current === generation && load === loadGeneration) {
        view.messageError = error.message
      }
    }
  }

  async function sendMessage(text) {
    if (!session || !view.peer || !text.trim()) return
    const current = generation
    const other = view.peer
    const trusted = session.peers.get(other)
    if (!trusted) throw new Error('Peer public key not found')
    const key = await messageKey(session.keys.xPrivate, trusted.x, session.iid, other)
    const { nonce, ciphertext, tag } = encryptMessage(key, text)
    const counter = await tick(other)
    const signed = envelope(session.iid, other, counter, nonce, ciphertext, tag)
    const signature = new Uint8Array(await crypto.subtle.sign('Ed25519',
      session.keys.edPrivate, signed))
    const result = await request('/messages', {
      to: other, nonce: base64url(nonce), ciphertext: base64url(ciphertext),
      auth_tag: base64url(tag), signature: base64url(signature),
      protocol_version: 2, message_counter: counter,
    }, session.token)
    if (current !== generation) return
    if (result.status !== 'ok') throw new Error('Sending message failed')
    await openConversation(other)
  }

  onMounted(() => {
    autoLogin().catch((error) => { view.error = error.message })
    pollTimer = setInterval(() => {
      if (session && view.screen === 'messenger' && view.peer) openConversation(view.peer)
    }, 5000)
  })
  onUnmounted(() => clearInterval(pollTimer))

  return {
    view, savedHandle, startRegistration, register,
    saveRegistration, finishRegistration, authenticate, resumeSavedLogin,
    logout, importConversation, openConversation, sendMessage,
  }
}
