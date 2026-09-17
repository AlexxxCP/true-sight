<script setup>
import { nextTick, ref, watch } from 'vue'

const props = defineProps({
  peer: { type: String, default: '' },
  messages: { type: Array, required: true },
  error: { type: String, default: '' },
})
const emit = defineEmits(['send'])
const draft = ref('')
const messageList = ref(null)

watch(() => props.messages, async () => {
  await nextTick()
  if (messageList.value) messageList.value.scrollTop = messageList.value.scrollHeight
})

function send() {
  if (!draft.value.trim()) return
  emit('send', draft.value)
  draft.value = ''
}

function time(value) {
  if (!value) return ''
  const date = new Date(value)
  return Number.isNaN(date.getTime()) ? '' : date.toLocaleTimeString([], {
    hour: '2-digit', minute: '2-digit',
  })
}
</script>

<template>
  <section class="chat">
    <div class="chat-header">
      <strong>{{ peer || 'Select a conversation' }}</strong>
      <small v-if="peer">Available</small>
    </div>
    <p v-if="error" class="error">{{ error }}</p>
    <div ref="messageList" class="messages">
      <p v-if="!peer" class="empty center-empty">Select a conversation</p>
      <div v-for="(message, index) in messages" :key="message.signature || index"
        class="bubble" :class="{ mine: message.receiver_iid === peer }">
        <span>{{ message.message_text }}</span>
        <small>{{ time(message.created_at) }}</small>
      </div>
    </div>
    <form class="composer" @submit.prevent="send">
      <input v-model="draft" class="input" :disabled="!peer" placeholder="Type a message…">
      <button class="button primary" :disabled="!peer || !draft.trim()">Send</button>
    </form>
  </section>
</template>
