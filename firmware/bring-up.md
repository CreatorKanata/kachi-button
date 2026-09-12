# Bring-up record — September 12, 2026

Target: manufactured Kachi Button PCB v1, connected by USB-C to macOS.

Sections are historical snapshots for individual builds. Later sections supersede
earlier pending checks when explicitly verified. The [current feature inventory](../docs/firmware-features.md) summarizes current evidence; the latest flashed build is the normally-off HID LED revision.

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


## Configurable macros, ms intervals, and remote ISP revision

- Per-key printable ASCII text (0–32 characters), repetitions (1–99), and
  repeat interval (0–60,000 ms). Empty text disables the key.
- Settings can be read/edited in normal mode and explicitly saved to DataFlash.
- Protocol v2 permits host-initiated ISP from normal or write-wait mode.
- Seven native C suites and five Python tests passed, including actual EP0
  adapter tests, timer wrap, settings snapshots, and all partial EEPROM writes.
- CH552 build: 9,247 / 14,336 flash bytes; 382 / 876 application XRAM bytes.
- HEX SHA-256: `9382f03136e373130f44ca30a1575828af89f9475c3d498d21f8fbe302dac127`.
- Installed on CH552 UID BE-A2-CA-BE at 06:36:36 UTC, September 12, 2026;
  wchisp returned `Verify OK`. Status returned protocol v2, normal mode.
- Normal-mode USB configuration read/edit/read-back and EEPROM save succeeded.
  A temporary top-key configuration used the original text, repeat 3, and 250 ms.
- Without any button gesture, the host requested ISP and reflashed the same HEX;
  `Verify OK` at 06:38:01 UTC. The saved test configuration loaded after reset.
- Restored all three original texts, repeat 1, interval 0 ms; saved and read back.
- USB detach exposed a libusb 1.0.29 macOS exit/hotplug deadlock before the first
  upload began. Process sampling localized it to libusb_exit/darwin_exit waiting
  on a detached-device event. Built official libusb 1.0.30 locally, then both
  uploads completed. The host tool now rejects macOS libusb older than 1.0.30.
- Seven C suites and six Python tests pass, including the new dependency guard.
- Physical repeated key output and host-observed ms timing remain unmeasured.
  EEPROM retention across application reset/reflash is verified; unplug/replug
  retention and deliberately interrupted saves have not been physically tested.


## Physical-key LED feedback revision

- Each debounced physical press causes one 10 ms off pulse of the normally lit
  LED, including presses discarded during busy macros. No hold/repeat pulses.
- Startup/write-wait patterns and suspend handling take priority.
- Eight native C suites and six Python tests passed, including pulse boundaries,
  wrap, reset, busy-time press edges, and no timer-wrap retrigger.
- Build: 9,425 / 14,336 flash bytes; 389 / 876 application XRAM bytes.
- HEX SHA-256: `7fca7951f728ed177832d96a7353cc2c476be5078eec526cd1784816ed6a148e`.
- Programmed CH552 BE-A2-CA-BE via normal-mode remote ISP at 07:09:41 UTC,
  September 12, 2026. `Verify OK`; configuration reads succeeded after reset.
- Saved Go Go! / Hi! / Thx, repeat 1, interval 0 ms remained intact.
- Physical LED waveform duration has not been measured; 10 ms is the software
  timer target, subject to main-loop scheduling and interrupt latency.


## 30 ms LED feedback revision

- Changed the physical-key off pulse from 10 ms to 30 ms; USB polling remains 10 ms.
- Eight C suites and six Python tests passed. Build: 9,425 flash bytes, 389 XRAM bytes.
- HEX SHA-256: `1060a58a6579e76caaa09e9d5efdf2fdc0ee0b9bdebf015477dcf6402ec81e2f`.
- Flashed CH552 BE-A2-CA-BE through host-only ISP at 08:19:01 UTC on September 12,
  2026; `Verify OK`. Post-reset configuration reads succeeded with the saved
  Go Go! / Hi! / Thx, repeat 1, interval 0 ms intact.
- LED duration is a software timer target; physical waveform remains unmeasured.


## Normally-off HID LED revision

- Active HID entry lights the LED for 2 seconds, followed by idle off.
- Each subsequent debounced physical press lights it for 30 ms, then off.
- Entry indication takes priority over press feedback without blocking typing;
  USB resume/reconfiguration restarts it. Boot hold/write-wait patterns retained.
- Eight C suites and six Python tests passed, including both duration boundaries,
  startup priority, lifecycle reset, and timer wrap.
- Build: 9,519 flash bytes / 14,336; 389 application XRAM bytes / 876.
- HEX SHA-256: `e7699e02404883d993054a9ebe61ae6ce0badbfe94b544e6bec63ef76e48584c`.
- Flashed CH552 BE-A2-CA-BE via normal-mode host upload at 08:32:15 UTC,
  September 12, 2026; `Verify OK`. Post-reset configuration reads succeeded;
  original text/count/interval settings remained intact.
- Physical LED polarity/appearance and durations have not been instrumented.


## Save-completion LED revision

- A verified settings save in normal HID mode lights the LED for 1 second.
  Temporary apply and failed saves do not trigger success feedback.
- Eight C suites and six Python tests passed; timing tests include wrap, failure,
  repeated saves, key/entry priority, and lifecycle reset.
- Build: 9,609 / 14,336 flash bytes; 394 / 876 application XRAM bytes.
- HEX SHA-256: `7c758311f587bace7bd079b28a5f24cb3a178d07c1341a0a2a14b589611ddb01`.
- Flashed CH552 BE-A2-CA-BE through normal-mode remote ISP; `Verify OK` at
  18:42:12 UTC September 12, 2026 (September 13 JST).
- Post-reset unchanged save completed with result 0, saving false, unsaved false.
  Go Go! / Hi! / Thx, repeat 1, interval 0 ms remained intact.
- Browser save interaction and physical LED duration were not observed in this
  test; the host tool exercised the same firmware save command.


## Factory chip ID revision

- Added optional read-only request 0x66: KI, format 1, 40-bit ID (MSB first).
- Eight native C suites, six Python tests, and eight browser protocol tests passed.
- Build: 9,753 / 14,336 flash bytes; 397 / 876 application XRAM bytes.
- HEX SHA-256: `41e0231dd8898f3ff388bb77b562c7fec474094cf40df290d899522911c38e1e`.
- Flashed CH552 BE-A2-CA-BE; Verify OK at 18:47:30 UTC September 12, 2026
  (September 13 JST). Normal-mode reads returned factory ID `79-BE-CA-A2-BE`
  twice. The lower four bytes, reversed, match the ISP UID BE-A2-CA-BE.
- Go Go! / Hi! / Thx, count 1, interval 0 remained saved after reset.
- Host USB verified the same new request used by Read device; actual browser
  rendering/interaction remains unverified due to the earlier automation restriction.


## USB manufacturer revision

- New USB manufacturer: `HAPT Lab, LLC`; product remains `Kachi Button`.
- Host tools and HTML accept the former `CreatorKanata` identity for upgrades.
  Other manufacturers remain rejected.
- Eight C suites, seven Python tests, and nine browser protocol tests passed.
- Build: 9,753 / 14,336 flash bytes; 397 / 876 application XRAM bytes.
- HEX SHA-256: `6ea7d1b452e13cd46a855ca137f394a4a315296f22dea3562baf91eca48ac550`.
- Flashed with Verify OK at 19:04:09 UTC September 12, 2026 (September 13 JST).
- Read the actual USB manufacturer descriptor after reset: `HAPT Lab, LLC`.
  Factory ID remains `79-BE-CA-A2-BE`. Saved Go Go!!! / Hi! / Thx, count 1,
  interval 0 ms remained intact, with no unsaved settings.
