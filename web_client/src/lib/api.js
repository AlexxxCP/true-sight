const base = import.meta.env.VITE_API_URL || ''

export async function request(path, body, token) {
  const response = await fetch(base + path, {
    method: body === undefined ? 'GET' : 'POST',
    headers: {
      ...(body === undefined ? {} : { 'Content-Type': 'application/json' }),
      ...(token ? { Authorization: `Bearer ${token}` } : {}),
    },
    ...(body === undefined ? {} : { body: JSON.stringify(body) }),
  })
  let result
  try { result = await response.json() }
  catch { throw new Error(`Invalid server response (${response.status})`) }
  if (!response.ok) {
    throw new Error(result.error || result.message || `Request failed (${response.status})`)
  }
  return result
}
