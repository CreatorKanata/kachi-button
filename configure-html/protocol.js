/* WebUSB protocol v2: device-recipient EP0 commands leave the HID interface attached. */
'use strict';
const C = KACHI_CONFIG;
function packMacro(key) {
  if (!Number.isInteger(key.id) || key.id < 0 || key.id >= C.KEY_COUNT ||
      typeof key.text !== 'string' || key.text.length > C.MAX_TEXT_LENGTH ||
      !/^[\x20-\x7e]*$/.test(key.text)) throw new Error('Use up to 32 printable ASCII characters.');
  if (!Number.isInteger(key.repeat) || key.repeat < 1 || key.repeat > C.MAX_REPEAT)
    throw new Error('Repetitions must be a whole number from 1 to 99.');
  if (!Number.isInteger(key.interval_ms) || key.interval_ms < 0 || key.interval_ms > C.MAX_INTERVAL_MS)
    throw new Error('Interval must be a whole number from 0 to 60000 ms.');
  const bytes = new Uint8Array(C.MAX_TEXT_LENGTH + 4);
  bytes.set([key.repeat, key.text.length, key.interval_ms & 255, key.interval_ms >> 8]);
  bytes.set([...key.text].map(char => char.charCodeAt(0)), 4);
  return bytes;
}
function sameMacro(left, right) {
  return ['id', 'text', 'repeat', 'interval_ms'].every(field => left[field] === right[field]);
}
class KachiUSB {
  constructor(device) { this.device = device; }
  setup(request, index = 0) {
    return {requestType: 'vendor', recipient: 'device', request, value: C.BOOT_COMMAND_MAGIC, index};
  }
  async input(request, length, index = 0) {
    const result = await this.device.controlTransferIn(this.setup(request, index), length);
    if (result.status !== 'ok' || result.data?.byteLength !== length)
      throw new Error('Incomplete USB response. Reconnect and read the device.');
    return new Uint8Array(result.data.buffer, result.data.byteOffset, result.data.byteLength);
  }
  async output(request, index = 0) {
    const result = await this.device.controlTransferOut(this.setup(request, index), new Uint8Array(0));
    if (result.status !== 'ok' || result.bytesWritten !== 0)
      throw new Error('USB command rejected. Read device before retrying.');
  }
  async open() {
    const device = this.device;
    if (device.vendorId !== C.USB_VID || device.productId !== C.USB_PID ||
        device.productName !== C.USB_PRODUCT || !C.USB_MANUFACTURERS.includes(device.manufacturerName))
      throw new Error('This is not a matching Kachi Button.');
    if (!device.opened) await device.open();
    if (!device.configuration) await device.selectConfiguration(1);
    // No claimInterface, reset, or keyboard report access is needed for vendor EP0.
    await this.info();
  }
  async info() {
    const bytes = await this.input(C.CONFIG_INFO_REQUEST, 8);
    if ([75, 67, 2, C.MAX_TEXT_LENGTH, C.MAX_REPEAT].some((value, i) => bytes[i] !== value))
      throw new Error('Unsupported firmware. Configuration protocol v2 is required.');
    return {result: bytes[5], saving: Boolean(bytes[6]), unsaved: Boolean(bytes[7])};
  }
  async key(id) {
    const bytes = new Uint8Array(C.MAX_TEXT_LENGTH + 4);
    for (let offset = 0; offset < bytes.length; offset += 8)
      bytes.set(await this.input(C.CONFIG_GET_REQUEST, Math.min(8, bytes.length - offset), (offset << 8) | id), offset);
    const key = {id, text: String.fromCharCode(...bytes.slice(4, 4 + bytes[1])),
      repeat: bytes[0], interval_ms: bytes[2] | (bytes[3] << 8)};
    if (packMacro(key).some((value, i) => value !== bytes[i])) throw new Error('Invalid device settings.');
    return key;
  }
  async chipId() {
    const result = await this.device.controlTransferIn(this.setup(C.CHIP_ID_REQUEST), 8);
    // Legacy v2 firmware stalls this optional request; keep its settings usable.
    if (result.status === 'stall') return null;
    if (result.status !== 'ok' || result.data?.byteLength !== 8)
      throw new Error('Incomplete chip ID response.');
    const bytes = new Uint8Array(result.data.buffer, result.data.byteOffset, 8);
    if ([75, 73, 1].some((value, i) => bytes[i] !== value))
      throw new Error('Unsupported chip ID response.');
    return [...bytes.slice(3)].map(value => value.toString(16).padStart(2, '0')).join('-').toUpperCase();
  }
  async state() {
    const status = await this.input(C.BOOT_STATUS_REQUEST, 4);
    if ([75, 66, 2].some((value, i) => status[i] !== value)) throw new Error('Unsupported device status.');
    const keys = [];
    for (let id = 0; id < C.KEY_COUNT; id++) keys.push(await this.key(id));
    return {keys, chipId: await this.chipId(), ...await this.info(), waiting: Boolean(status[3])};
  }
  async set(key) {
    const bytes = packMacro(key);
    await this.output(C.CONFIG_BEGIN_REQUEST, key.id);
    for (let offset = 0; offset < bytes.length; offset++)
      await this.output(C.CONFIG_BYTE_REQUEST, (bytes[offset] << 8) | offset);
    await this.output(C.CONFIG_APPLY_REQUEST);
    if (!sameMacro(await this.key(key.id), key)) throw new Error('Settings read-back mismatch.');
  }
  async save() {
    await this.output(C.CONFIG_SAVE_REQUEST);
    const deadline = Date.now() + C.SAVE_TIMEOUT_MS;
    while (Date.now() < deadline) {
      const info = await this.info();
      if (!info.saving) {
        if (info.result || info.unsaved) throw new Error('Device save verification failed.');
        return;
      }
      await new Promise(resolve => setTimeout(resolve, C.SAVE_POLL_MS));
    }
    throw new Error('Save timed out. Read device before retrying.');
  }
  async apply(keys, persist) {
    if (!Array.isArray(keys) || keys.length !== C.KEY_COUNT || typeof persist !== 'boolean')
      throw new Error('Expected three key settings.');
    // Validate all rows before the first mutation; each device update is atomic per key.
    keys.forEach((key, id) => {
      if (key.id !== id) throw new Error('Invalid key order.');
      packMacro(key);
    });
    if ((await this.info()).saving) throw new Error('Device is saving. Try reading again shortly.');
    for (const key of keys) if (!sameMacro(await this.key(key.id), key)) await this.set(key);
    if (persist) await this.save();
    const result = await this.state();
    if (!result.keys.every((key, id) => sameMacro(key, keys[id]))) throw new Error('Settings read-back mismatch.');
    return result;
  }
}
