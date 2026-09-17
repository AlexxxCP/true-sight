let databasePromise

function database() {
  databasePromise ||= new Promise((resolve, reject) => {
    const request = indexedDB.open('true-sight-vue', 1)
    request.onupgradeneeded = () => request.result.createObjectStore('settings')
    request.onsuccess = () => resolve(request.result)
    request.onerror = () => reject(request.error)
  })
  return databasePromise
}

export async function getSetting(key) {
  const db = await database()
  return new Promise((resolve, reject) => {
    const request = db.transaction('settings').objectStore('settings').get(key)
    request.onsuccess = () => resolve(request.result)
    request.onerror = () => reject(request.error)
  })
}

export async function setSetting(key, value) {
  const db = await database()
  return new Promise((resolve, reject) => {
    const transaction = db.transaction('settings', 'readwrite')
    transaction.objectStore('settings').put(value, key)
    transaction.oncomplete = resolve
    transaction.onerror = () => reject(transaction.error)
  })
}

export async function deleteSetting(key) {
  const db = await database()
  return new Promise((resolve, reject) => {
    const transaction = db.transaction('settings', 'readwrite')
    transaction.objectStore('settings').delete(key)
    transaction.oncomplete = resolve
    transaction.onerror = () => reject(transaction.error)
  })
}
