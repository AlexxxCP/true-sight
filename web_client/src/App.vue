<script setup>
import AuthFlow from './components/AuthFlow.vue'
import ChatView from './components/ChatView.vue'
import ConversationList from './components/ConversationList.vue'
import { useClient } from './composables/useClient'

const client = useClient()
const { view, savedHandle } = client

async function run(action) {
  view.error = ''
  try { await action() }
  catch (error) {
    if (error.name !== 'AbortError') view.error = error.message
  }
}

async function importConversation(file, close) {
  await run(async () => {
    await client.importConversation(file)
    close()
  })
}
</script>

<template>
  <header class="app-header">
    <strong>TrueSight</strong>
    <button v-if="view.screen === 'messenger'" class="button outline small"
      @click="run(client.logout)">Logout</button>
  </header>

  <AuthFlow v-if="view.screen !== 'messenger'" :view="view" :saved-handle="savedHandle"
    @begin-registration="run(client.startRegistration)"
    @show-login="view.screen = 'login'; view.error = ''"
    @register="run(() => client.register($event))"
    @save-registration="run(() => client.saveRegistration($event))"
    @finish-registration="client.finishRegistration"
    @authenticate="run(() => client.authenticate($event.username, $event.file, $event.handle))"
    @resume-login="run(client.resumeSavedLogin)"
    @error="view.error = $event" />

  <main v-else class="messenger">
    <ConversationList :conversations="view.conversations" :peer="view.peer"
      :error="view.error" @open="client.openConversation"
      @import="importConversation" @error="view.error = $event"
      @clear-error="view.error = ''" />
    <ChatView :peer="view.peer" :messages="view.messages"
      :error="view.messageError || view.error"
      @send="run(() => client.sendMessage($event))" />
  </main>
</template>
