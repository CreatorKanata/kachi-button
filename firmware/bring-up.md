# Bring-up record — September 12, 2026

Target: manufactured Kachi Button PCB v1, connected by USB-C to macOS.

- Initial enumeration: WCH ISP `4348:55e0`.
- Read-only identification: CH552 (`0x5211`), bootloader 2.50.
- Native scanner/report tests: passed.
- CH552 compilation: passed, 24 MHz internal clock.
- Flash: 5,283 / 14,336 bytes; application XRAM: 134 / 876 bytes.
- USB XRAM reservation: 148 bytes; internal stack region: 210 bytes available
  according to the linker (not a measured stack high-water mark).
- `wchisp flash`: programmed and verified successfully (`Verify OK`).
- HEX SHA-256: `45192d3b7dbb4032d6425727c106f8bc6fcfbf0af0fda734f75891331da0c44d`.
- Post-reset USB descriptor: `Kachi Button`, manufacturer `CreatorKanata`,
  VID:PID `1209:c55d`.
- HID driver registration: confirmed via `hidutil list`, USB usage page 1,
  usage 6 (keyboard), AppleUserUSBHostHIDDevice.
- macOS Keyboard Setup Assistant appeared for CreatorKanata; dismiss it with
  Quit because this three-key device has no full keyboard layout to identify.
- Physical key input: user confirmed correct operation by typing `KNAKNA`
  with the device, then explicitly confirming successful input. Top = K,
  lower left = N, lower right = A; repeated press cycles worked.

Not yet tested: recovery entry via D+ pull-up, other operating systems,
power consumption/suspend current, ESD, and physical switch bounce measurements.

## Fixed-text revision

Requested mappings: ID 0 = `Go Go!`, ID 1 = `Hi!`, ID 2 = `Thx`.
Each press sends exactly the text without Enter or a trailing separator.

- Native key scanner and text report sequence tests: passed.
- Build: 5,652 / 14,336 flash bytes; 137 / 876 application XRAM bytes.
- HEX SHA-256: `7ceda7eee9ffd166e1b8441782e257fdebf88fc0687c93b99072eba7705f4778`.
- Programming: pending hardware entry into WCH USB ISP mode.
- Physical text input: not yet verified. Earlier KNA results apply to the old build.

## Startup button ISP revision (2-second hold)

- Hold all three keys from power-up continuously for 2 seconds to enter USB ISP.
- LED blinks while holding; turns off before handing control to the bootloader.
- Any observed release cancels; normal-use presses cannot rearm the gesture.
- Text is suppressed during the gesture. Fixed texts remain Go Go! / Hi! / Thx.
- Native scanner, text sequencer, and boot gesture tests: passed.
- CH552 build: 6,014 / 14,336 flash bytes; 146 / 876 application XRAM bytes.
- HEX SHA-256: `06889d98cf661207ba4259854f3e2810d333aad8143e06df87fb17507affc346`.
- Flashed to CH552 UID BE-A2-CA-BE on September 12, 2026; `wchisp` returned
  `Verify OK`. After reset, macOS registered Kachi Button as a USB HID keyboard.
- User confirmed correct text input. User observed hold blinking, then off, then
  steady on after attempting ISP entry. A later USB inspection found normal HID;
  the transition into native ISP was not captured.


## Persistent write-wait revision

- Two-second startup gesture latches write-wait; release cannot cancel it.
- Slow hold blinking: 250 ms half-period; fast confirmed waiting: 75 ms half-period.
- No timeout in application waiting; native ISP begins only on uploader command.
- Existing Go Go! / Hi! / Thx mapping preserved.
- Four native C test programs and three Python uploader tests passed, including
  simulated one-hour waiting, timer wrap, and malformed/unarmed command refusal.
- CH552 build: 6,432 / 14,336 flash bytes; 153 / 876 application XRAM bytes.
- HEX SHA-256: `4d3b7866aa18793b688b0b8b99e48a0852dd5e14663c997cf52f2cf1358a8408`.
- macOS libusb opened the existing keyboard without detaching its HID driver;
  the old firmware correctly stalled the unsupported status request.
- Installed on CH552 UID BE-A2-CA-BE at 05:44:25 UTC, September 12, 2026;
  wchisp reported `Verify OK`. The previous two-second gesture successfully
  exposed native ISP for this installation.
- macOS enumerated the new firmware as Kachi Button HID. The new EP0 status
  command returned `WRITE_WAIT (no timeout)` after reset, and again more than
  80 seconds later, confirming wait persistence beyond the 16-bit timer wrap.
- Physical fast-blink appearance and the new uploader-to-ISP command followed
  by flashing remain unverified. No second flash was performed for this check.
