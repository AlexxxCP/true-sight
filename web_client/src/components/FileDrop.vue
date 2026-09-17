<script setup>
import { ref } from 'vue'

const props = defineProps({
  file: { type: File, default: null },
  suffix: { type: String, required: true },
  title: { type: String, required: true },
})
const emit = defineEmits(['selected', 'error'])
const input = ref(null)
const dragging = ref(false)

function select(file, handle = null) {
  if (!file?.name.toLowerCase().endsWith(props.suffix)) {
    emit('error', `Select one ${props.suffix} file`)
    return
  }
  emit('selected', { file, handle })
}

async function browse() {
  try {
    if (window.showOpenFilePicker) {
      const [handle] = await window.showOpenFilePicker({
        multiple: false,
        types: [{
          description: `TrueSight ${props.suffix}`,
          accept: { 'application/json': [props.suffix] },
        }],
      })
      select(await handle.getFile(), handle)
    } else {
      input.value.click()
    }
  } catch (error) {
    if (error.name !== 'AbortError') emit('error', error.message)
  }
}

function drop(event) {
  dragging.value = false
  if (event.dataTransfer.files.length !== 1) {
    emit('error', 'Drop one file')
    return
  }
  select(event.dataTransfer.files[0])
}
</script>

<template>
  <button type="button" class="file-drop" :class="{ dragging, selected: file }"
    @click="browse" @dragenter.prevent="dragging = true"
    @dragover.prevent="dragging = true" @dragleave.prevent="dragging = false"
    @drop.prevent="drop">
    <strong>{{ title }}</strong>
    <span>{{ file?.name || `Drop one local ${suffix} file or click to browse` }}</span>
    <small v-if="file">Click to choose another file</small>
  </button>
  <input ref="input" class="visually-hidden" type="file" :accept="suffix"
    @change="select($event.target.files[0])">
</template>
