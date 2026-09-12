# Kachi Button firmware

Configurable text macros for PCB v1 (CH552E), September 12, 2026.

See the [development guide](../docs/firmware-development.md), [feature inventory](../docs/firmware-features.md), and [internals](../docs/firmware-internals.md) for task-oriented documentation.

| ID | Physical key | GPIO | Default text |
| --- | --- | --- | --- |
| 0 | Top / SW1 | P1.5 | `Go Go!` |
| 1 | Lower left / SW2 | P1.6 | `Hi!` |
| 2 | Lower right / SW3 | P1.7 | `Thx` |

Each key has 0–32 printable ASCII characters, 1–99 repetitions and a
0–60,000 ms repeat interval. Defaults use one repetition and 0 ms.
An empty text disables that key. One physical press executes the entire macro;
holding the key does not retrigger it. No Enter or separator is added.

The interval is the software delay after the final release report of one copy
is queued and before the next copy begins. Each character still has a key-down
and release report, paced by the USB endpoint. Actual host-observed timing is
quantized by the 10 ms USB polling interval and host scheduling; this is not a
precision pulse generator. A zero interval adds no extra delay.

Use a US/ABC-compatible Latin keyboard layout with other modifiers released.
The firmware accounts for Caps Lock, but does not implement IME conversion,
Unicode text, arbitrary keyboard layouts, or automatic Enter.

Inputs have pull-ups and 15 ms debounce. Held startup/resume keys require release.
Only one macro runs at a time; presses while busy are discarded until release.
Simultaneous presses use Top, Left, Right priority. Settings updates do not alter
an already running macro; each accepted press snapshots its text/count/interval.
USB reset or suspend cancels the current macro and staged configuration edit.

## Configure from the host

Python 3 and a libusb shared library are required (on macOS, 1.0.30 or newer). No CDC serial port or
browser UI is included. These commands use vendor requests on the keyboard's
USB control endpoint without claiming/detaching its HID interface.

```sh
# Read all keys and save status.
python3 firmware/configure.py get

# A single top-key press types Go! three times with 250 ms gaps; persist it.
python3 firmware/configure.py set 0 --text 'Go!' --repeat 3 --interval-ms 250 --save

# Changes without --save apply in RAM; persist all three keys explicitly.
python3 firmware/configure.py save
```

For a library outside the search path, place
`--libusb /path/to/libusb-1.0.dylib` before the subcommand, or set the
`KACHI_LIBUSB` environment variable. macOS libusb 1.0.29 was observed deadlocking
in exit during USB detach; the tool rejects it before opening USB. Version
1.0.30 contains [Darwin concurrency fixes](https://github.com/libusb/libusb/blob/master/ChangeLog). Set operates on one key;
`--repeat` defaults to 1 and `--interval-ms` to 0. Run one configuration writer
at a time. The tool checks capability, validates input, applies a complete key
atomically, reads it back, and waits for verified save completion when requested.

## Saved settings

CH552 provides 128 logical DataFlash bytes. This version uses bytes 0–113:
`KC`, schema 1, three 36-byte macros, CRC-16/CCITT-FALSE (little endian), and
commit marker `0xa5`. Each macro is count, length, interval-ms low/high bytes,
then 32 zero-padded ASCII bytes. Addresses 114–127 remain unused.

Save invalidates the marker first, updates and reads back every changed byte,
then writes the commit marker last. Identical saves perform no EEPROM writes.
Invalid schema, checksum, text, ranges, or commit marker loads all defaults.
Only explicit save writes EEPROM. Flashing application code leaves it intact.

There is one record, not two: interrupted save can lose the previous saved
configuration and fall back to defaults. The implementation never intentionally
loads a partially committed record. Unchanged valid records remain available if
failure occurs before invalidation. A save error is returned to the host.

The pinned [CH55xduino EEPROM implementation](https://github.com/DeqingSun/ch55xduino/blob/ch55xduino/ch55xduino/ch55x/cores/ch55xduino/eeprom.c)
uses logical byte addresses 0–127 and the CH552 DataFlash write sequence.
The wrapper masks interrupts for each byte's protected register accesses; saving
runs in the foreground, with status requests available between byte operations.

## Firmware upload and waiting

Protocol v2 accepts a host ISP-entry command from normal mode or write-wait mode.
An AI/host tool can now update firmware without a physical key gesture:

```sh
python3 firmware/flash.py --status
python3 firmware/flash.py firmware/build/compiled/kachi_button.ino.hex --wchisp /path/to/wchisp

# Optional: enter native ISP without immediately flashing.
python3 firmware/flash.py --enter
```

The uploader validates the known firmware protocol, requests ISP, waits for the
new USB identity, and invokes wchisp with verification enabled. It refuses
unknown protocols, missing files/tools, multiple keyboard targets, or an already
attached WCH ISP target. Keep only the intended target attached during upload.
These local USB commands are intentionally host-controlled, not authenticated.

The physical alternative remains: hold all three keys before connecting USB,
then keep holding for 2 seconds. Slow blinking (250 ms on/off) changes to fast
blinking (75 ms on/off). After confirmation, release the keys and wait indefinitely.
USB reset/suspend and key changes do not clear confirmed waiting; power/MCU reset
does. Text output is suppressed in this state. Reconnect without keys for normal
operation. Entering active normal HID mode lights the LED for 2 seconds, then
leaves it off. Each debounced physical press then produces one 30 ms on pulse
and returns to off. The entry indication takes priority during its 2 seconds;
presses still type normally but do not interrupt or extend that indication.
Reconfiguration or resume starts a new entry indication.
This includes presses ignored during a running macro; holding a key does not
repeat the pulse. Software-generated repetitions do not trigger extra pulses.
The nonblocking pulse timer yields to startup/write-wait patterns and USB suspend.

The application retains identity `1209:c55d` while waiting. Native ISP starts
only when the host requests it, then the application LED loop stops. Native ISP
has its own timeout; use the integrated uploader when programming. A successful
flash resets the device into the new application. Fast blinking is guaranteed by
the application waiting logic, not during native ISP itself.

Bootloader entry follows pinned CH55xduino 0.0.26 `USBCDC.c`: USB detach, disable
interrupts/timer mode, 100 ms delay, call CH552 address `0x3800`. No configuration
registers or bootloader code are rewritten. Boards still running protocol-v1 firmware
require the three-key wait for the one-time upgrade; this uploader also
supports that legacy wait state. The oldest K/N/A firmware requires hardware ISP.

### Native ISP and recovery

A blank CH552 normally appears as WCH ISP `4348:55e0`. Use tested
[wchisp](https://github.com/ch32-rs/wchisp) 0.3.0, commit
`cefd8707df345f1fbd7795e15367281f440bbf05`:

```sh
wchisp info --chip CH552
wchisp flash firmware/build/compiled/kachi_button.ino.hex
```

The command erases application flash, programs, verifies, and resets. If an
upload fails after erasing the application, use native ISP directly for recovery.
The retained factory hardware-entry setting uses D+. With USB disconnected,
connect D+ to 5 V through a 10 kOhm resistor, then reconnect USB; remove the
pull-up after ISP entry. Never directly short D+ to 5 V. PCB v1 has labeled test
pads. This hardware recovery wiring has not been physically tested on this board.
See [upstream entry instructions](https://github.com/DeqingSun/ch55xduino#installation).

## Build and test

Dependencies: Python 3, a native C compiler, Arduino CLI and CH55xduino 0.0.26
(SDCC build.13407_4, MCS51Tools 2026.07.10); tested CLI version is 1.3.1.

```sh
export ARDUINO_DIRECTORIES_DATA="$PWD/.arduino-data"
arduino-cli core update-index --additional-urls https://raw.githubusercontent.com/DeqingSun/ch55xduino/ch55xduino/package_ch55xduino_mcs51_index.json
arduino-cli core install CH55xDuino:mcs51@0.0.26 --additional-urls https://raw.githubusercontent.com/DeqingSun/ch55xduino/ch55xduino/package_ch55xduino_mcs51_index.json
python3 firmware/build.py --data "$ARDUINO_DIRECTORIES_DATA"
```

Use `--cli /path/to/arduino-cli`, `--out /path/to/build`, or `--test-only` as
needed. All limits, timing and identifiers are centralized in `config.py`.
Output is `firmware/build/compiled/kachi_button.ino.hex` with map/memory files.

Build staging copies pinned upstream HID descriptors/control handlers and makes
checked substitutions for product strings, power, lifecycle hooks and vendor
requests. Application flash is limited to 14,336 bytes; 148 XRAM bytes are
reserved for USB, leaving 876 for the application. CH55xduino's USB code and the
ASCII map are LGPL-2.1; retain the corresponding source, license and relinkable
objects when redistributing binaries.

USB identity is the upstream development VID:PID `1209:c55d`, not an allocated
production identifier. One boot keyboard interface uses endpoint `0x81` with
eight-byte reports at 10 ms. Remote wake is not advertised. Suspend extinguishes
the normal LED; confirmed write-wait continues blinking. Power consumption and
suspend compliance have not been measured.

## USB control protocol v2

All requests use wValue `0x4b42`. IN type is `0xc0`; OUT type is `0x40`.
OUT commands have zero data length; mutation occurs only after status-stage ACK.
A new SETUP cancels an unacknowledged command. Invalid requests stall.

| Request | Direction/length | wIndex | Meaning |
| --- | --- | --- | --- |
| `0x5a` | IN / 4 | 0 | `KB`, version 2, waiting flag |
| `0x5b` | OUT / 0 | 0 | Enter native ISP from either mode |
| `0x60` | IN / 8 | 0 | `KC`, version 2, max text, max count, result, saving, dirty |
| `0x61` | IN / 1–8 | offset high, key ID low | Read a macro slice, within 36 bytes |
| `0x62` | OUT / 0 | key ID | Begin a new single-key staged edit |
| `0x63` | OUT / 0 | byte value high, offset low | Append next sequential byte, 0–35 |
| `0x64` | OUT / 0 | 0 | Validate and atomically apply the complete staged macro |
| `0x65` | OUT / 0 | 0 | Queue save, poll info for completion |

Save result: 0 success, 1 pending, 2 storage error. Edits and ISP entry stall
while saving; read-only requests remain available. Repeating an edit must start
with begin; a second client can replace a staged edit, so writers must serialize.
No configuration command emits keyboard input.

## Validation

Eight native C suites cover scanning, default reports, repetitions and ms gaps,
snapshot behavior, startup waiting, remote ISP validation, the actual EP0 adapter,
settings bounds, EEPROM round-trip, corruption and all injected partial writes.
Six Python tests cover uploader gating/ordering and configuration encoding/wire
commands. Native tests do not prove physical timing, EEPROM or signal quality.
See [bring-up.md](bring-up.md) for build sizes and actual hardware checks.

Dismiss macOS Keyboard Setup Assistant with Quit; three keys cannot complete its
full-keyboard identification procedure.
