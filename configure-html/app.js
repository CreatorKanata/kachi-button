/* Standalone settings UI: user-selected WebUSB device, serialized edits, explicit save. */
'use strict';
const byId = id => document.getElementById(id);
const form = byId('settings');
let usb;
let connected = false;
let busy = false;
let edited = false;
const supported = Boolean(navigator.usb && window.isSecureContext);
function message(text, error = false) {
  byId('message').textContent = text;
  byId('message').classList.toggle('error', error);
}
function controls() {
  byId('keys').disabled = busy || !connected;
  byId('read').disabled = busy || !connected;
  byId('connect').disabled = busy || !supported;
}
function render(data) {
  for (const key of data.keys) {
    byId(`text${key.id}`).value = key.text;
    byId(`repeat${key.id}`).value = key.repeat;
    byId(`interval${key.id}`).value = key.interval_ms;
  }
  byId('chip-id').textContent = data.chipId ?? 'Unavailable · update firmware';
  connected = true;
  edited = false;
  byId('connection').textContent = data.waiting ? 'Connected · write-wait mode' : 'Connected · keyboard mode';
  byId('saved').textContent = data.saving ? 'Device is saving…' : data.unsaved ? 'Device has temporary settings' : 'Device settings are saved';
}
async function closeUSB() {
  const previous = usb;
  usb = undefined;
  if (previous?.device.opened) {
    try { await previous.device.close(); } catch { /* Unplugged devices are already closed. */ }
  }
}
async function run(action, success, mutation = false) {
  if (busy) return;
  busy = true;
  controls();
  message('Working…');
  try {
    const data = await action();
    render(data);
    message(data.waiting ? 'Reconnect without holding keys to enable typing. Settings remain available.' : success);
  } catch (error) {
    connected = false;
    byId('chip-id').textContent = '—';
    await closeUSB();
    byId('connection').textContent = 'Not connected';
    byId('saved').textContent = 'Device state is not confirmed';
    const detail = error.name === 'NotFoundError' ? 'No device selected or the device was unplugged.' : error.message;
    message(detail + (mutation ? ' Some keys may already be applied. Reconnect and read before retrying.' : ''), true);
  } finally {
    busy = false;
    controls();
  }
}
function collect() {
  for (let id = 0; id < C.KEY_COUNT; id++) {
    const input = byId(`text${id}`);
    input.setCustomValidity(/^[\x20-\x7e]*$/.test(input.value) ? '' : 'Use printable ASCII letters, numbers, spaces, and symbols.');
  }
  if (!form.reportValidity()) return null;
  return [0, 1, 2].map(id => ({id, text: byId(`text${id}`).value,
    repeat: Number(byId(`repeat${id}`).value), interval_ms: Number(byId(`interval${id}`).value)}));
}
function apply(persist) {
  const keys = collect();
  if (keys) run(() => usb.apply(keys, persist),
    persist ? 'Saved to device and verified.' : 'Applied and verified. These settings are temporary.', true);
}
function canDiscard() {
  return !edited || confirm('Discard unsubmitted form changes and read the device?');
}
byId('connect').addEventListener('click', () => {
  if (!canDiscard()) return;
  // Request synchronously within the click, before any await, to retain user activation.
  const selection = navigator.usb.requestDevice({filters: [{vendorId: C.USB_VID, productId: C.USB_PID}]});
  run(async () => {
    const device = await selection;
    await closeUSB();
    usb = new KachiUSB(device);
    await usb.open();
    return usb.state();
  }, 'Device settings loaded.');
});
byId('read').addEventListener('click', () => {
  if (canDiscard()) run(() => usb.state(), 'Device settings loaded.');
});
form.addEventListener('submit', event => {event.preventDefault(); apply(true);});
byId('temporary').addEventListener('click', () => apply(false));
form.addEventListener('input', event => {
  event.target.setCustomValidity('');
  edited = true;
  message('Form changes have not been applied.');
});
navigator.usb?.addEventListener('disconnect', event => {
  if (event.device === usb?.device) {
    connected = false;
    byId('chip-id').textContent = '—';
    byId('connection').textContent = 'Disconnected';
    byId('saved').textContent = 'Reconnect USB, then choose Connect USB.';
    message('USB disconnected. Form values have been kept.', true);
    controls();
  }
});
for (let id = 0; id < C.KEY_COUNT; id++) {
  byId(`text${id}`).maxLength = C.MAX_TEXT_LENGTH;
  Object.assign(byId(`repeat${id}`), {min: 1, max: C.MAX_REPEAT});
  Object.assign(byId(`interval${id}`), {min: 0, max: C.MAX_INTERVAL_MS});
}
controls();
message(supported ? 'Connect Kachi Button by USB, then choose Connect USB.' :
  'WebUSB is unavailable. Open this file in desktop Chrome with USB access allowed.', !supported);
