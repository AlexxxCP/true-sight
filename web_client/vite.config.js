import vue from '@vitejs/plugin-vue'
import { defineConfig } from 'vite'

// https://vite.dev/config/
export default defineConfig({
  plugins: [vue()],
  server: {
    proxy: {
      '/register': 'http://localhost:8888',
      '/get-challenge': 'http://localhost:8888',
      '/validate-challenge': 'http://localhost:8888',
      '/conversations': 'http://localhost:8888',
      '/messages': 'http://localhost:8888',
    },
  },
})
