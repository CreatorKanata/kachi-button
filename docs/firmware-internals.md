# Keyboard firmware internals

Source map and wire/storage contracts for protocol v2, updated September 12, 2026.
Start with the [development guide](firmware-development.md) for routine use.

## Module ownership

| Source | Responsibility |
| --- | --- |
| [config.py](../firmware/config.py) | Board pins, defaults, limits, timers, request IDs and tool requirements; generates C config |
| [build.py](../firmware/build.py) | Stage pinned upstream USB sources, apply checked hooks, run tests, compile CH552 |
| [kachi_button.ino](../firmware/kachi_button.ino) | Coordinate input, settings-save job, boot state, LED and HID scheduling |
| [keys.c](../firmware/src/keys.c) | Debounced physical edges and one-action-at-a-time selection |
| [text.c](../firmware/src/text.c) | Snapshot text/count/interval; queue key-down/release reports and timed repetitions |
| [key_led.c](../firmware/src/key_led.c) | 2 s HID-entry indication, idle off, and 30 ms key-on pulses |
| [boot_gesture.c](../firmware/src/boot_gesture.c) | Startup-only two-second hold and latched write-wait |
| [boot_command.c](../firmware/src/boot_command.c) | Strict boot/status USB request validation |
| [device_control.c](../firmware/src/device_control.c) | EP0 control protocol and commit-after-ACK dispatch |
| [transport.c](../firmware/src/transport.c) | Nonblocking EP1, lifecycle epoch, native ISP jump |
| [settings.c](../firmware/src/settings.c) | RAM profiles, one-key staging, validation and save state |
| [storage.c](../firmware/src/storage.c) | CRC record, load, verify-each-byte save and commit marker |
| [nv_ch552.c](../firmware/src/nv_ch552.c) | Pinned core EEPROM calls with per-byte interrupt protection |
| [usb_control.py](../firmware/usb_control.py) | libusb ABI, identity checks and control transfers |
| [configure.py](../firmware/configure.py) | User-facing get/set/save, validation and read-back |
| [flash.py](../firmware/flash.py) | Version/state check, native entry and verified wchisp upload |

## Runtime flow

After pin initialization, firmware selects the startup gesture state, loads saved
settings or defaults, then starts USB. In each loop, a queued save runs outside
the USB ISR. A host boot request hands over to native ISP. Startup/write-wait
states take priority over normal keyboard and LED behavior.

Normal mode scans physical edges, updates press feedback and accepts a macro
only when the sequencer and HID endpoint are available. Snapshot copying masks
USB interrupts to prevent a settings apply from mixing old and new values.
The sequencer advances only when the endpoint accepts a report. USB lifecycle
epoch changes discard stale scheduled input and require physical key release.

USB mutation requests are validated during SETUP and applied after status-stage
ACK. A new SETUP cancels an unacknowledged operation. Cache setup fields before
writing EP0 response bytes: request and response share the same eight-byte buffer.
The adapter has a native regression test for this aliasing boundary.

## USB identity and control protocol

The keyboard uses development VID:PID `1209:c55d`, product `Kachi Button`,
manufacturer `CreatorKanata`, no device serial string, one boot-keyboard interface,
endpoint `0x81`, eight-byte reports and a 10 ms polling interval. Native ISP uses
`4348:55e0` on the tested board. There is no CDC or separate vendor HID interface.

The [canonical request table](../firmware/README.md#usb-control-protocol-v2) lists
fixed command IDs. All requests use wValue `0x4b42`; IN type is `0xc0`, OUT type
is `0x40`. Configuration OUT commands intentionally have zero data length: each
staged byte is encoded in wIndex, avoiding multi-packet OUT state.

Example edit transaction for key 0:

1. `0x62`, wIndex 0: begin a new staged key.
2. `0x63` for offsets 0–35 in order: wIndex = `(value << 8) | offset`.
3. `0x64`, wIndex 0: validate all bytes and atomically apply the key.
4. Read `0x61` slices to confirm the applied record.
5. Optionally `0x65`, then poll `0x60` until saving is false and result is 0.

An incomplete edit cannot apply. Begin replaces any previous stage; there is no
multi-client transaction identifier. Serialize writers on the host. During save,
mutating requests and ISP entry stall; reads remain available. No edit/save
request itself emits keyboard input. Local host control is intentionally allowed;
these request constants are protocol tags, not authentication secrets.

## Macro wire format

Each key has exactly 36 bytes:

| Offset | Size | Meaning |
| --- | --- | --- |
| 0 | 1 | Repetitions, 1–99 |
| 1 | 1 | Text length, 0–32 |
| 2 | 2 | Interval in ms, little endian, 0–60,000 |
| 4 | 32 | Printable ASCII text followed by zero padding |

Length 0 disables output. Padding must be zero. Nonprintable bytes, unsupported
ranges and oversized text are rejected by firmware and the host encoder.

## DataFlash format

CH552 exposes 128 logical EEPROM bytes; the core maps logical byte addresses to
even physical addresses. The application uses 114 bytes:

| Address | Size | Meaning |
| --- | --- | --- |
| 0 | 2 | ASCII `KC` |
| 2 | 1 | Storage schema 1 |
| 3 | 108 | Three 36-byte macro records in key-ID order |
| 111 | 2 | CRC-16/CCITT-FALSE over bytes 0–110, stored little endian |
| 113 | 1 | Commit marker `0xa5` |
| 114 | 14 | Unused |

CRC parameters: polynomial `0x1021`, initial value `0xffff`, no reflection,
no final XOR. Save first clears the marker, updates/reads back changed bytes,
then commits last. Load validates marker, header, CRC and all macro bounds.
There is no schema migration: an unknown schema loads defaults. Keep this format
and `SETTINGS_SCHEMA` synchronized when changing profile layout.

## Tests and change boundaries

Portable suites cover scanner edges, text reports, repeat gaps/snapshots, LED
pulse timing, startup gesture, boot validation, actual EP0 adapter behavior, and
storage failure injection. Python tests cover wire encoding, uploader sequencing
and the macOS libusb guard. See [tests](../firmware/tests) and the [validation record](../firmware/bring-up.md).

Changing USB hooks requires checking the pinned upstream source and ACK/reset
ordering. Changing limits requires checking the 128-byte record and 876-byte
application XRAM budget. Changing timing requires wrap/cancellation tests.
Changes to the storage layout or USB protocol need explicit version handling;
changing text/count/interval settings through the existing CLI needs no rebuild.
