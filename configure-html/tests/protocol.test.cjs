/* Test browser protocol against a USB simulator, including stalls and failed persistence. */
const {test} = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const path = require('node:path');
const scope = vm.createContext({Uint8Array, Date, setTimeout});
for (const file of ['config.js', 'protocol.js'])
  vm.runInContext(fs.readFileSync(path.join(__dirname, '..', file), 'utf8'), scope);
const {KachiUSB, packMacro, C} = vm.runInContext('({KachiUSB, packMacro, C})', scope);
const defaults = () => ['Go Go!', 'Hi!', 'Thx'].map((text, id) => ({id, text, repeat: 1, interval_ms: 0}));
class FakeDevice {
  constructor() {
    Object.assign(this, {vendorId: C.USB_VID, productId: C.USB_PID,
      productName: C.USB_PRODUCT, manufacturerName: C.USB_MANUFACTURER,
      opened: false, configuration: {configurationValue: 1}, dirty: false, saveError: false});
    this.keys = defaults().map(packMacro);
    this.commands = [];
  }
  async open() { this.opened = true; }
  async controlTransferIn(setup, length) {
    assert.equal(setup.recipient, 'device');
    assert.equal(setup.requestType, 'vendor');
    assert.equal(setup.value, 0x4b42);
    let bytes;
    if (setup.request === 0x60) bytes = [75, 67, 2, 32, 99, this.saveError ? 2 : 0, 0, Number(this.dirty)];
    else if (setup.request === 0x66) bytes = [75, 73, 1, 0x12, 0x34, 0x56, 0x78, 0x9a];
    else if (setup.request === 0x5a) bytes = [75, 66, 2, 0];
    else if (setup.request === 0x61) {
      const offset = setup.index >> 8;
      bytes = this.keys[setup.index & 255].slice(offset, offset + length);
    } else throw new Error('Unexpected read');
    return {status: 'ok', data: new DataView(Uint8Array.from(bytes).buffer)};
  }
  async controlTransferOut(setup, data) {
    assert.equal(data.length, 0);
    assert.equal(setup.recipient, 'device');
    this.commands.push([setup.request, setup.index]);
    if (this.stall) return {status: 'stall'};
    if (setup.request === 0x62) { this.editId = setup.index; this.staged = []; }
    else if (setup.request === 0x63) {
      assert.equal(setup.index & 255, this.staged.length);
      this.staged.push(setup.index >> 8);
    } else if (setup.request === 0x64) {
      assert.equal(this.staged.length, 36);
      this.keys[this.editId] = Uint8Array.from(this.staged);
      this.dirty = true;
    } else if (setup.request === 0x65) { if (!this.saveError) this.dirty = false; }
    else throw new Error('Unexpected command');
    return {status: 'ok', bytesWritten: 0};
  }
}
test('validates all rows before USB writes, including fractional and non-ASCII input', async () => {
  for (const bad of [{text: 'あ'}, {text: 'x'.repeat(33)}, {text: '\n'}, {repeat: 0},
    {repeat: 100}, {repeat: 2.5}, {repeat: true}, {interval_ms: -1}, {interval_ms: 60001}]) {
    const device = new FakeDevice();
    const keys = defaults();
    Object.assign(keys[2], bad);
    await assert.rejects(new KachiUSB(device).apply(keys, true));
    assert.equal(device.commands.length, 0);
  }
  assert.equal(packMacro({id: 0, text: '', repeat: 99, interval_ms: 60000}).length, 36);
});
test('opens without claiming HID and reads all physical IDs', async () => {
  const device = new FakeDevice();
  const usb = new KachiUSB(device);
  await usb.open();
  assert.equal(device.opened, true);
  assert.equal(JSON.stringify((await usb.state()).keys), JSON.stringify(defaults()));
});
test('edit and save use sequential zero-data commands and verify settings', async () => {
  const device = new FakeDevice();
  const keys = defaults();
  keys[1] = {id: 1, text: 'Probe!', repeat: 3, interval_ms: 250};
  const result = await new KachiUSB(device).apply(keys, true);
  assert.equal(JSON.stringify(result.keys), JSON.stringify(keys));
  assert.equal(result.unsaved, false);
  assert.equal(device.commands.length, 39);
  assert.deepEqual(device.commands[0], [0x62, 1]);
  assert.deepEqual(device.commands[3], [0x63, (250 << 8) | 2]);
  assert.deepEqual(device.commands.at(-1), [0x65, 0]);
});
test('temporary update never writes EEPROM; unchanged keys are skipped', async () => {
  const device = new FakeDevice();
  const keys = defaults();
  keys[0].repeat = 2;
  const result = await new KachiUSB(device).apply(keys, false);
  assert.equal(result.unsaved, true);
  assert.equal(device.commands.length, 38);
  assert.equal(device.commands.some(([request]) => request === 0x65), false);
});
test('stall stops mutation and does not attempt save', async () => {
  const device = new FakeDevice();
  device.stall = true;
  const keys = defaults();
  keys[0].text = 'Probe';
  await assert.rejects(new KachiUSB(device).apply(keys, true), /rejected/);
  assert.equal(device.commands.length, 1);
});
test('save failure and short read never report success', async () => {
  const device = new FakeDevice();
  device.saveError = true;
  await assert.rejects(new KachiUSB(device).apply(defaults(), true), /save verification failed/);
  device.controlTransferIn = async () => ({status: 'ok', data: new DataView(new ArrayBuffer(1))});
  await assert.rejects(new KachiUSB(device).info(), /Incomplete/);
});
test('corrupt macro padding and wrong protocol are rejected', async () => {
  const device = new FakeDevice();
  device.keys[2][35] = 65;
  await assert.rejects(new KachiUSB(device).key(2), /Invalid/);
  device.controlTransferIn = async () => ({status: 'ok', data: new DataView(new ArrayBuffer(8))});
  await assert.rejects(new KachiUSB(device).info(), /Unsupported/);
});

test('reads full 40-bit factory ID and tolerates only explicit legacy stalls', async () => {
  const device = new FakeDevice();
  const usb = new KachiUSB(device);
  assert.equal((await usb.state()).chipId, '12-34-56-78-9A');
  device.controlTransferIn = async () => ({status: 'stall'});
  assert.equal(await usb.chipId(), null);
  device.controlTransferIn = async () => ({status: 'ok', data: new DataView(new ArrayBuffer(7))});
  await assert.rejects(usb.chipId(), /Incomplete/);
  device.controlTransferIn = async () => ({status: 'ok', data: new DataView(new ArrayBuffer(8))});
  await assert.rejects(usb.chipId(), /Unsupported/);
  device.controlTransferIn = async () => { throw new Error('Disconnected'); };
  await assert.rejects(usb.chipId(), /Disconnected/);
});

test('manufacturer migration accepts old boards but rejects unrelated identities', async () => {
  for (const manufacturerName of ['HAPT Lab, LLC', 'CreatorKanata']) {
    const device = new FakeDevice();
    device.manufacturerName = manufacturerName;
    await new KachiUSB(device).open();
    assert.equal(device.opened, true);
  }
  const device = new FakeDevice();
  device.manufacturerName = 'Other';
  await assert.rejects(new KachiUSB(device).open(), /not a matching/);
  assert.equal(device.opened, false);
});
