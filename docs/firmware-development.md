# Keyboard firmware development

Operational guide for CH552E PCB v1, updated September 12, 2026.
Commands below run from the repository root. See the [feature inventory](firmware-features.md)
for current behavior and [internals](firmware-internals.md) for the implementation.

## Prerequisites

| Tool | Version / purpose |
| --- | --- |
| Python | Python 3, host tools and build orchestration |
| Native C compiler | `cc`, portable regression tests |
| Arduino CLI | Tested 1.3.1 |
| CH55xduino | Pinned 0.0.26; SDCC build.13407_4, MCS51Tools 2026.07.10 |
| wchisp | Tested 0.3.0 at `cefd8707df345f1fbd7795e15367281f440bbf05` |
| libusb | Shared library; macOS requires 1.0.30 or newer |

Install the pinned Arduino core using the [build instructions](../firmware/README.md#build-and-test).
Point these variables at your installed tools when they are outside default paths:

```sh
export ARDUINO_DIRECTORIES_DATA="/path/to/arduino-data"
export KACHI_LIBUSB="/path/to/libusb-1.0.dylib"
```

Use a libusb library matching the Python process architecture. The host tools
reject macOS libusb older than 1.0.30 before opening USB: 1.0.29 deadlocked during
the observed detach/exit sequence. See the [hardware record](../firmware/bring-up.md).
Changing the library path does not require changing or reflashing device firmware.

## Inspect before editing

```sh
python3 firmware/flash.py --status
python3 firmware/configure.py get
```

`NORMAL / HOLDING` means the firmware has not latched write-wait. The normal LED
is on; a startup candidate blinks slowly. `WRITE_WAIT (no timeout)` means the
application is waiting indefinitely with fast blinking. Both retain the same
keyboard USB identity. An unresponsive device is not evidence of native ISP;
use `wchisp info --chip CH552` to identify an already connected bootloader.

## Change text, count, and interval

```sh
python3 firmware/configure.py set 0 --text 'Go!' --repeat 3 --interval-ms 250 --save
python3 firmware/configure.py get
```

One top-key press now sends `Go!Go!Go!`, with the specified gap between copies.
The interval follows the last release report of each copy; it is not a delay
between individual characters or a guaranteed start-to-start period.

Without `--save`, the edit applies in RAM. Persist all three keys later with
`python3 firmware/configure.py save`. `set` replaces the selected key's entire
configuration: omitted repeat and interval options become 1 and 0. Use one writer
at a time. Configuration changes never type the new text by themselves.

To disable the lower-right key, use an empty string:

```sh
python3 firmware/configure.py set 2 --text '' --repeat 1 --interval-ms 0 --save
```

## Build and update from normal mode

```sh
python3 firmware/build.py --data "$ARDUINO_DIRECTORIES_DATA"
python3 firmware/flash.py firmware/build/compiled/kachi_button.ino.hex --wchisp /path/to/wchisp
python3 firmware/configure.py get
```

Pass `--cli /path/to/arduino-cli` to build if necessary. Protocol v2 accepts the
upload request from normal mode: no unplugging or button gesture is required.
The tool requests native ISP, waits for USB re-enumeration, then programs and
verifies. `Verify OK` confirms firmware contents; reading configuration afterward
checks application communication and retained settings. Saved settings live
outside application flash; unsaved RAM edits are lost on reboot.

### Enter native ISP without uploading

```sh
python3 firmware/flash.py --enter
wchisp info --chip CH552
```

This enters the native bootloader immediately. Its timeout applies, and the
application no longer controls the LED. Prefer the integrated upload command
when writing a HEX so time spent between commands does not consume that window.

### Wait as long as needed before uploading

1. Disconnect USB.
2. Hold all three keys, reconnect USB, and continue holding for 2 seconds.
3. Slow blinking changes to fast blinking. Release the keys.
4. Run the integrated upload command whenever ready.

Fast blinking is an application write-wait state, not native ISP. It has no
software timeout and suppresses typing. Reconnect without holding keys to return
to normal use. Protocol-v1 boards need this gesture once to install v2; the older
K/N/A-only build needs hardware entry. Native ISP does not itself blink forever.

## Validation during development

Run portable tests without an embedded compilation:

```sh
python3 firmware/build.py --data "$ARDUINO_DIRECTORIES_DATA" --test-only
```

The full build runs the same tests before compiling the CH552 target. Review its
flash/XRAM summary and retain the HEX, map and memory report. Current measured
sizes and SHA-256 are in [bring-up.md](../firmware/bring-up.md#physical-key-led-feedback-revision).

For an intentional hardware regression test, first record all existing settings.
Use a harmless probe text/count/interval, explicitly save, then verify it after
normal-mode upload/reset. Restore and save the original settings afterward.
Physical repetition count, host-observed interval, and LED waveform measurements
are separate from portable tests; record exactly which observations were made.

## Troubleshooting

| Symptom | Check / action |
| --- | --- |
| libusb version rejected or detach hangs | Select libusb 1.0.30+ via `KACHI_LIBUSB`; do not repeatedly flash with the affected library |
| `dlopen` architecture mismatch | Match library architecture to Python; Apple Silicon can also run x86_64 Python |
| No matching target / multiple targets | Connect only the intended Kachi Button; inspect USB identity before retrying |
| Target is already WCH ISP | Use native `wchisp flash` directly; the application control endpoint is absent |
| Old firmware refuses normal-mode entry | Use the three-key gesture for the one-time v2 upgrade |
| Fast blink but `wchisp` sees nothing | Expected during application wait; use `firmware/flash.py` to request native ISP |
| Save reports an error | Read state again; do not assume settings persisted until save completes successfully |
| Key input is unexpected | Select a US/ABC-compatible input source and release other keyboard modifiers |
| Keyboard Setup Assistant appears | Quit the assistant; this device cannot perform full-keyboard identification |

If application flash is unusable, follow the [documented hardware recovery procedure](../firmware/README.md#native-isp-and-recovery).
That D+ pull-up procedure has not been physically tested on this board.
