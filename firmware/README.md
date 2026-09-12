# Kachi Button firmware

Fixed-mapping USB HID bring-up for PCB v1 (CH552E), September 12, 2026.
This implements the requested first firmware; browser configuration and stored
macros remain future work.

| Physical button | MCU GPIO | Output |
| --- | --- | --- |
| Top / SW1 | P1.5 | `Go Go!` |
| Lower left / SW2 | P1.6 | `Hi!` |
| Lower right / SW3 | P1.7 | `Thx` |

Each debounced press sends its full text, with a key-down and all-released
report for every character. Physical IDs are 0 (Top), 1 (Left), and 2 (Right). Holding a key does not repeat. Use a Latin keyboard input source (for
example ABC) and release other modifiers during testing. Caps Lock is accounted
for using the host's LED report; IME conversion and arbitrary layouts are host
behavior. Inputs have internal pull-ups and a 15 ms stable debounce interval.
Keys held at connection, USB reset or resume must be released before use.
Simultaneous presses use Top, Left, Right priority; a press accepted while another
text macro is active is discarded until that key is released. No Enter is sent.

P1.4 drives the active-low status LED: on when configured and active, off before
configuration or during suspend. Remote wakeup is not advertised. Suspend current
has not been measured; this bring-up build does not put the CPU into power-down.

## Build and test

Dependencies: Python 3, a native C compiler, Arduino CLI, and CH55xduino 0.0.26
(SDCC build.13407_4, MCS51Tools 2026.07.10). The tested Arduino CLI is 1.3.1.
Install the pinned core into your preferred Arduino data directory:

```sh
export ARDUINO_DIRECTORIES_DATA="$PWD/.arduino-data"
arduino-cli core update-index --additional-urls https://raw.githubusercontent.com/DeqingSun/ch55xduino/ch55xduino/package_ch55xduino_mcs51_index.json
arduino-cli core install CH55xDuino:mcs51@0.0.26 --additional-urls https://raw.githubusercontent.com/DeqingSun/ch55xduino/ch55xduino/package_ch55xduino_mcs51_index.json
python3 firmware/build.py --data "$ARDUINO_DIRECTORIES_DATA"
```

Pass `--cli /path/to/arduino-cli` if it is not on PATH. `--test-only` runs the
native scanner/report tests without compiling for CH552. `--out /path/to/build`
selects the output directory. All board/timing/identifier settings live in
`config.py`, which generates `src/config.h` for both native and device builds.

The script stages the pinned upstream HID control/descriptor sources into the
build directory and applies checked substitutions. Local changes set the product
strings and power declaration, remove remote wakeup, and clear pending reports
on USB lifecycle events. The custom EP1 sender is nonblocking. Build staging
avoids maintaining a duplicate copy of the upstream USB control implementation.
CH55xduino's upstream source is LGPL-2.1; retain upstream license/source and
relinkable build objects if redistributing a firmware binary.

Output: `firmware/build/compiled/kachi_button.ino.hex`, with `.map`, `.mem` and
link objects beside it. Application flash is limited to 14,336 bytes (bootloader
excluded). 148 bytes of XRAM are reserved for USB; application XRAM limit is 876.
USB uses development VID:PID `1209:c55d`, one boot keyboard interface, endpoint
`0x81`, eight-byte reports, and a 10 ms polling interval. This is the upstream
CH55xduino keyboard identity, not an independently allocated product VID/PID.

## Flash

The first blank board was already in WCH USB ISP mode (`4348:55e0`). Its chip
identified as CH552 and bootloader 2.50. Flash using
[wchisp](https://github.com/ch32-rs/wchisp), tested version 0.3.0 at commit
`cefd8707df345f1fbd7795e15367281f440bbf05`:

```sh
wchisp info --chip CH552
wchisp flash firmware/build/compiled/kachi_button.ino.hex
```

Only attach the intended WCH ISP target when flashing. The command erases the
application area, programs, verifies, then resets. Keep verification enabled.
The implementation does not write EEPROM, change boot-pin configuration, or
include a CDC interface for automatic upload reset.

### Button entry and unlimited write waiting (after installing this revision)

Hold all three keys before connecting USB, then keep them pressed for 2 seconds.
The LED blinks slowly during the hold (250 ms on / 250 ms off). At confirmation,
it changes to fast blinking (75 ms on / 75 ms off) and stays in write-wait mode
without a timeout. Release the keys and take as long as needed before uploading.
No text is emitted in either gesture or write-wait state. Early release cancels
the gesture; pressing all keys later during normal operation cannot rearm it.
USB bus reset, reconfiguration, suspend/resume, button changes, and clock wrap
do not clear the confirmed wait. Disconnect power and reconnect without holding
keys to return to normal use. MCU reset also clears the wait.

The native WCH bootloader cannot run our LED loop. Therefore write-wait runs in
the application, retaining USB identity `1209:c55d`, with keyboard input suppressed.
Use the provided uploader when ready; it checks the wait state, requests native
ISP, and immediately invokes wchisp while the bootloader is available:

```sh
python3 firmware/flash.py --status
python3 firmware/flash.py firmware/build/compiled/kachi_button.ino.hex --wchisp /path/to/wchisp
```

The uploader requires Python 3, libusb 1.0 (a shared library), and wchisp. Use
`--libusb /path/to/libusb-1.0.dylib` if automatic library discovery fails. It uses
EP0 vendor control requests and does not claim or detach the keyboard interface.
Only one matching Kachi device and no other WCH ISP devices may be attached.
Direct `wchisp flash` cannot switch the application out of write-wait mode.

Fast blinking continues throughout user waiting, but stops when the uploader
hands control to native ISP. After successful programming and reset, the new
firmware starts. If upload fails after ISP entry, reconnect with the gesture to
retry; if the application was erased, use native ISP directly for recovery.

The vendor-control protocol uses device recipient, wValue `0x4b42`, wIndex 0:
- IN `0xc0`, request `0x5a`, length 4 returns `4b 42 01 <waiting>`; waiting is 0 or 1.
- OUT `0x40`, request `0x5b`, length 0 starts ISP only when waiting is 1.
Malformed requests stall. Entry is queued only after the status-stage ACK, and
new SETUP/reset cancels an incomplete request. Identifiers are not authentication;
the physical startup gesture gates entry.

Native entry follows CH55xduino 0.0.26 `USBCDC.c`: disable USB and interrupts,
clear timer mode, allow 100 ms USB detach, then call the CH552 bootloader at
`0x3800`. No flash configuration, EEPROM, or bootloader code is rewritten.
The software-only waiting approach avoids depending on the native bootloader's
[documented timeout](https://github.com/DeqingSun/ch55xduino#driver-for-windows).
USB transfers follow the [libusb control-transfer API](https://libusb.sourceforge.io/api-1.0/group__libusb__syncio.html).

### First installation on an already programmed board / recovery

The older K/N/A firmware does not have the button-entry feature. Installing this
revision on that board once requires hardware ISP entry (or a blank board).
The retained factory boot-pin setting uses D+.
[CH55xduino's documented hardware entry procedure](https://github.com/DeqingSun/ch55xduino#installation)
is to disconnect power, connect D+ to 5 V **through a 10 kOhm resistor**, and
reconnect USB. Remove the temporary pull-up after entering ISP mode. PCB v1 has
marked D+ and 5V test pads. Never directly short these pads. This recovery
procedure has not yet been physically tested on this board; the first write used
the blank chip's automatic bootloader entry. The three-key gesture only works after installing the new revision. Do not
treat RST as an active-low reset.

## Validation

Native tests cover the actual C scanner and report builder: all three mappings,
case/Caps Lock, spaces, punctuation, per-character release, retry, cancellation, debounce boundaries/bounce, startup-held
keys, repeat suppression, busy-time presses, simultaneous priority, and timer
wrap. Startup-only boot entry, the 2-second boundary, cancellation, and no
rearming during normal use, and a simulated one-hour latched wait are covered by
`tests/test_boot_gesture.c`. `tests/test_boot_command.c` checks command validation
and the physical gate; `tests/test_flash.py` checks uploader refusal and ordering. See also
`tests/test_keys.c` and `tests/test_text.c`.

See `bring-up.md` for the measured build size and actual hardware verification.
On first macOS connection, dismiss Keyboard Setup Assistant with Quit; this
three-key device cannot perform its full-keyboard identification sequence. Then
select a Latin input source and test Top, Left, Right for `Go Go!Hi!Thx` (no separator or Enter is added).
Host tests do not prove physical switch mapping, signal quality or power behavior.
