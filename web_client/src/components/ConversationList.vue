<script setup>
import { computed, ref, shallowRef } from 'vue'
import FileDrop from './FileDrop.vue'

const props = defineProps({
  conversations: { type: Array, required: true },
  peer: { type: String, default: '' },
  error: { type: String, default: '' },
})
const emit = defineEmits(['open', 'import', 'error', 'clear-error'])
const filter = ref('')
const modalOpen = ref(false)
const shareFile = shallowRef(null)
const visible = computed(() => props.conversations.filter((row) =>
  row.peer.toLowerCase().includes(filter.value.toLowerCase())))

function openModal() {
  shareFile.value = null
  emit('clear-error')
  modalOpen.value = true
}

function importFile() {
  if (shareFile.value) emit('import', shareFile.value, () => { modalOpen.value = false })
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
  <aside class="sidebar">
    <h2>Conversations</h2>
    <input v-model="filter" class="input search" placeholder="Filter conversations">
    <button class="button outline wide add-button" @click="openModal">Add conversation +</button>
    <div class="conversation-list">
      <button v-for="row in visible" :key="row.peer" class="conversation"
        :class="{ active: peer === row.peer }" @click="emit('open', row.peer)">
        <span class="avatar">{{ row.peer.slice(0, 1).toUpperCase() }}</span>
        <span class="conversation-info">
          <strong>{{ row.peer }}</strong><small>{{ time(row.last_message_at) }}</small>
        </span>
      </button>
      <p v-if="!visible.length" class="empty">No conversations yet</p>
    </div>
  </aside>

  <div v-if="modalOpen" class="modal-backdrop" @click.self="modalOpen = false">
    <section class="modal" role="dialog" aria-modal="true" aria-label="Add conversation">
      <div class="modal-header">
        <h2>Add conversation</h2>
        <button class="icon-button" @click="modalOpen = false">×</button>
      </div>
      <p>Drop the other person's public key share file.</p>
      <FileDrop :file="shareFile" suffix=".share" title="Public key"
        @selected="shareFile = $event.file" @error="emit('error', $event)" />
      <p v-if="error" class="error">{{ error }}</p>
      <div class="modal-actions">
        <button class="button outline" @click="modalOpen = false">Cancel</button>
        <button class="button primary" :disabled="!shareFile" @click="importFile">Add</button>
      </div>
    </section>
  </div>
</template>
