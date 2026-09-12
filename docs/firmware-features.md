# Keyboard firmware feature inventory

Implemented behavior for CH552E PCB v1, updated September 12, 2026.
This inventory describes the current code. The [product specifications](spec/firmware-spec.md)
also include future requirements; those are not automatically implemented features.

## Key assignments and limits

| ID | Position | GPIO | Default text | Default count | Default gap |
| --- | --- | --- | --- | --- | --- |
| 0 | Top | P1.5 | `Go Go!` | 1 | 0 ms |
| 1 | Lower left | P1.6 | `Hi!` | 1 | 0 ms |
| 2 | Lower right | P1.7 | `Thx` | 1 | 0 ms |

Each key stores up to 32 printable ASCII characters, 1–99 repetitions and a
0–60,000 ms gap. Empty text disables the action. All values are configurable over
USB and explicitly savable. No Enter, separator or trailing space is appended.
Defaults apply only if no valid saved record is present.

## Implemented features

| Feature | Behavior | Verification |
| --- | --- | --- |
| USB HID keyboard | Standard eight-byte boot-keyboard reports, 10 ms polling | Enumerated on macOS; initial and fixed-text input confirmed by user |
| Physical key scanner | 15 ms stable debounce, release required after startup, no held-key repeat | Native tests; initial physical mapping confirmed |
| Text macros | Complete key-down/release per character; US printable ASCII, Caps Lock-aware case | Native report tests; initial phrases confirmed by user |
| Repetition | One physical press emits the entire text the selected number of times | Native tests, including maximum count |
| Repeat interval | Nonblocking ms gap between copies; 0 adds no extra wait | Native boundary/wrap tests; physical timing not measured |
| Concurrent presses | One active macro; additional presses discarded, simultaneous priority 0 → 1 → 2 | Native scanner tests |
| Settings snapshot | Edits affect future presses; an active macro uses its original snapshot | Native test |
| USB settings control | Get, staged single-key edit, atomic apply, explicit save and read-back | Normal-mode read/edit/read-back verified on hardware |
| Persistent settings | 114-byte versioned/CRC-checked record in 128-byte DataFlash | Save and reload after application reset/reflash verified |
| Save interruption handling | Invalid/incomplete record loads defaults; prior record is not guaranteed retained | Native failure injection at every changed-byte write |
| Host-only firmware update | Protocol v2 can request native ISP from normal mode or write-wait | Normal-mode request, flash and verify succeeded on hardware |
| Unlimited write-wait | Startup three-key 2-second hold latches fast blinking until power/reset or upload | Native one-hour simulation; more than 80 seconds observed by USB state |
| Press feedback LED | One 10 ms off pulse for each debounced physical press in normal mode | Native tests; firmware flashed; waveform not measured |
| Lifecycle handling | USB reset/suspend cancels pending text and staged edit; held keys require release | Code and portable cancellation tests; full power testing pending |
| macOS library guard | Reject libusb below 1.0.30 to avoid the observed detach/exit deadlock | Regression test and successful uploads with 1.0.30 |

## LED meanings

| State | LED | Key input |
| --- | --- | --- |
| Normal, configured USB | Steady on | Enabled |
| Physical press in normal mode | Off for 10 ms, then on | Normal macro rules apply |
| Startup three-key hold | 250 ms on / 250 ms off | Suppressed |
| Confirmed write-wait | 75 ms on / 75 ms off, no timeout | Suppressed |
| Unconfigured/suspended normal mode | Off | Suppressed |
| Native WCH ISP | Application cannot control it | Keyboard application is not running |

Press feedback includes presses discarded because a macro is busy. A held key
and software repetitions do not cause extra pulses. Simultaneous physical edges
share one pulse; an overlapping new edge restarts its 10 ms window. Startup and
write-wait patterns take priority. Durations are firmware timer targets subject
to main-loop and interrupt latency.

## Persistence and timing boundaries

The gap starts when the final release report is queued, not when the host editor
renders text. USB polling and host scheduling affect observed timing. A setting
of 250 ms is a requested software wait, not a precision external waveform.

Saving is explicit. A repeated identical save does not write EEPROM again. A
single record fits the device; interruption after invalidation can discard the
old saved settings and load defaults. CRC plus structural validation rejects
incomplete or invalid records. Hardware power-cut testing remains outstanding.

## Not implemented

- Browser configuration UI, WebHID configuration interface, or mobile UI.
- USB CDC serial console, diagnostic serial logs, or a human-readable serial protocol.
- Unicode/Japanese text input, IME control, arbitrary host keyboard layouts.
- Configurable Enter, modifier shortcuts, mouse actions, or multi-step scripts.
- Double-click/long-press actions or an action queue during a running macro.
- Two-record transactional storage, wear leveling, or guaranteed old-setting recovery.
- Production VID/PID allocation, remote wakeup, or verified suspend-power compliance.

## Current release evidence

Latest flashed LED-feedback build: 9,425 / 14,336 flash bytes and 389 / 876
application XRAM bytes. Eight C suites and six Python tests passed. The hardware
record identifies target, timestamps, image hashes, settings retention and
remaining physical checks. See [bring-up.md](../firmware/bring-up.md) for evidence
and the [development guide](firmware-development.md) for reproducible commands.
