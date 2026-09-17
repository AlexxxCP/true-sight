export async function readJsonFile(file, suffix) {
  if (!file?.name.toLowerCase().endsWith(suffix) || file.size > 64 * 1024) {
    throw new Error(`Select a local ${suffix} file under 64 KiB`)
  }
  try { return JSON.parse(await file.text()) }
  catch { throw new Error(`Invalid ${suffix} JSON`) }
}

export async function saveJsonFile(value, filename) {
  const content = JSON.stringify(value, null, 2) + '\n'
  if (window.showSaveFilePicker) {
    const suffix = filename.endsWith('.share') ? '.share' : '.tskey'
    const handle = await window.showSaveFilePicker({
      suggestedName: filename,
      types: [{ description: `TrueSight ${suffix}`, accept: { 'application/json': [suffix] } }],
    })
    const writable = await handle.createWritable()
    await writable.write(content)
    await writable.close()
    return
  }
  const link = document.createElement('a')
  const url = URL.createObjectURL(new Blob([content], { type: 'application/json' }))
  link.href = url
  link.download = filename
  link.click()
  setTimeout(() => URL.revokeObjectURL(url), 1000)
}
