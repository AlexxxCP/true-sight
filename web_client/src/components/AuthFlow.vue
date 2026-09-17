<script setup>
import { ref, shallowRef, watch } from 'vue'
import FileDrop from './FileDrop.vue'

const props = defineProps({
  view: { type: Object, required: true },
  savedHandle: { type: Object, default: null },
})
const emit = defineEmits([
  'begin-registration', 'show-login', 'register', 'save-registration',
  'finish-registration', 'authenticate', 'resume-login', 'error',
])
const username = ref('')
const keyFile = shallowRef(null)
const keyHandle = shallowRef(null)

watch(() => props.view.username, (value) => { username.value = value })
watch(() => props.view.screen, (value) => {
  if (value === 'choice') { username.value = ''; keyFile.value = null; keyHandle.value = null }
})

function selected({ file, handle }) {
  keyFile.value = file
  keyHandle.value = handle
}
</script>

<template>
  <main class="center-screen">
    <div v-if="view.screen === 'choice'" class="choice">
      <div class="choice-buttons">
        <button class="button primary" @click="emit('begin-registration')">Register</button>
        <button class="button outline" @click="emit('show-login')">Login</button>
      </div>
      <p v-if="view.error" class="error">{{ view.error }}</p>
    </div>

    <form v-else-if="view.screen === 'register'" class="auth-card"
      @submit.prevent="emit('register', username.trim())">
      <h1>Register</h1>
      <input v-model="username" class="input" placeholder="Username" maxlength="128">
      <button class="button primary wide" :disabled="view.busy || !username.trim()">
        {{ view.busy ? 'Registering…' : 'Continue' }}
      </button>
      <p v-if="view.error" class="error">{{ view.error }}</p>
    </form>

    <div v-else-if="view.screen === 'downloads'" class="auth-card">
      <h1>Save your keys</h1>
      <p>Save both files before continuing. Keep the .tskey file private; share the .share file with people you want to message.</p>
      <button class="button outline wide" @click="emit('save-registration', 'share')">
        Download .share file {{ view.shareSaved ? '✓' : '' }}
      </button>
      <button class="button outline wide" @click="emit('save-registration', 'tskey')">
        Download .tskey file {{ view.keySaved ? '✓' : '' }}
      </button>
      <button class="button primary wide" :disabled="!view.shareSaved || !view.keySaved"
        @click="emit('finish-registration')">Continue</button>
      <p v-if="view.error" class="error">{{ view.error }}</p>
    </div>

    <form v-else class="auth-card" @submit.prevent="emit('authenticate', {
      username: username.trim(), file: keyFile, handle: keyHandle,
    })">
      <h1>Sign in</h1>
      <p>Enter your username and select your private key file.</p>
      <input v-model="username" class="input" placeholder="Username" autocomplete="username">
      <FileDrop :file="keyFile" suffix=".tskey" title="Private key"
        @selected="selected" @error="emit('error', $event)" />
      <button v-if="savedHandle && !keyFile" type="button" class="button outline wide"
        @click="emit('resume-login')">Resume saved login</button>
      <button v-if="username.trim() && keyFile" class="button primary wide" :disabled="view.busy">
        {{ view.busy ? 'Signing in…' : 'Continue' }}
      </button>
      <p v-if="view.error" class="error">{{ view.error }}</p>
    </form>
  </main>
</template>
