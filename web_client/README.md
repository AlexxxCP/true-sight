# TrueSight web client

Vue 3 SPA built with Vite. It uses the existing backend endpoints and the same
`.tskey`, `.share`, signed-message, encryption, and Lamport-counter formats as
the Qt client.

```sh
cd web_client
npm install
npm run dev
```

Vite proxies the backend API to `http://localhost:8888` in development. For a
production build, serve `dist/` and proxy `/register`, `/get-challenge`,
`/validate-challenge`, `/conversations`, and `/messages` to the backend on the
same origin. `VITE_API_URL` can override the API base when the backend permits
cross-origin requests.

Browser WebSockets cannot attach the backend's Bearer authorization header, so
the SPA refreshes the active conversation every five seconds. Browsers with the
File System Access API can keep a `.tskey` file handle in IndexedDB for later
login. The browser may require a click to renew file permission; the private
key file contents are never saved to IndexedDB. Dragging a `.tskey` file into
the page signs in for that session, but does not grant a persistent file handle.

The SPA depends on browser support for WebCrypto Ed25519 and X25519.
