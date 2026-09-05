# Firmware and Configuration Specification

Status: required behavior and proposed implementation baseline, September 6, 2026. Firmware and the configuration website have not been implemented or validated in this hardware release. See [product requirements](product-spec.md) and [the PCB pin map](pcb-spec.md).

## Platform and build

Target the WCH CH552E on PCB v1. Use its internal clock and native USB peripheral. CH55xduino and an SDCC-based implementation are candidates from the concept phase; select a toolchain after proving USB descriptors, bootloader compatibility, code size, RAM use, and nonvolatile storage access. QMK compatibility is not a requirement.

Keep pin assignments, debounce timing, limits, and USB identifiers in a shared configuration module appropriate to the firmware language. Record toolchain/library versions and a reproducible build command. Retain a linker map and memory-budget summary with release builds.

Do not derive application flash capacity from the total advertised chip memory without accounting for the bootloader and reserved regions. Resolve exact erase/write semantics and available data storage against the [WCH documentation](https://www.wch-ic.com/downloads/CH552DS1_PDF.html) and selected toolchain before implementing persistence.

## Startup and key scanning

Initialize P1.5, P1.6, and P1.7 as pulled-up inputs for Top, Lower left, and Lower right. A closure to GND is a press. Initialize P1.4 to the inactive LED state; LOW lights D1.

Load a valid saved configuration or documented defaults, then enumerate USB. Do not emit keyboard input just because the device was plugged in, resumed, reset, or configured. Require a release before accepting a key already held at startup.

Implement nonblocking per-key debounce and keep USB servicing responsive. A 10–20 ms stable interval is the initial tuning range, not a validated switch measurement. One debounced press produces one action; holding the button does not repeatedly launch text. Rearm after a debounced release. Multi-click and long-press recognition are deferred.

Proposed initial concurrency policy: execute one action at a time and ignore new presses while it is active, requiring release before rearming those keys. Document and test this bounded behavior rather than allowing partial actions to interleave. Revisit only if user testing establishes a need for queuing.

## Actions and mapping

| Stage | Supported behavior |
| --- | --- |
| Bring-up | Fixed text on each key, no persistent configuration dependency |
| Initial configurable release | Three text mappings, each up to 32 ASCII bytes; optional explicit Enter suffix |
| Subsequent extension | Single HID keys and modifier shortcuts using a versioned action type |
| Deferred | Multi-step scripts, Unicode/emoji, double/long press, and application-specific automation |

Use the latest product image for the proposed demonstration defaults: Top = `Let's go!`, Lower left = `Hi!`, Lower right = `Thanks!`. This physical order differs from earlier numbered examples. Default phrases and legends are editable, not hardwired identities. The AI preset is `OK`, `Continue.`, and `Yes, do it.` in the same physical order.

Target printable US-layout ASCII for text. The host must use a matching keyboard layout; clearly disclose this in the UI. Reject unsupported characters rather than silently corrupting text. Unicode input methods and arbitrary host layout conversion are outside the initial release.

Generate complete key-down/modifier and release reports for each character or shortcut. Always clear modifiers and pressed keys at action completion or cancellation while connected. On disconnect, discard pending actions and restart in an all-released state. Respect endpoint readiness rather than emitting reports with unbounded blocking delays. Test capitalization, punctuation, repeated letters, and rapid button presses.

## USB interfaces

The configurable target is a composite HID device with two interfaces:

| Interface | Purpose |
| --- | --- |
| 0: HID Keyboard | Normal host keyboard input |
| 1: Vendor-defined HID | Read, edit, and persist device configuration through WebHID |

The vendor collection must be separate from keyboard usage collections. Chrome restricts direct website access to protected keyboard collections; its [WebHID guide](https://developer.chrome.com/docs/capabilities/hid) documents the distinction and device selection/report APIs.

Define the complete descriptors and endpoint allocation after checking the selected USB stack and CH552E resource limits. VID/PID, usage page/usage, report IDs, endpoint numbers, polling intervals, protocol version, and exact byte layout are TBD implementation deliverables, not established by the concept's example commands.

Target reports no larger than 64 bytes, subject to the descriptor and stack constraints. Specify whether a stated size includes the report ID. A full three-key configuration may require multiple transfers; do not assume three 32-byte strings fit in one report. Normal keyboard operation must continue without a browser connection.

The device consumes 5 V USB power without PD negotiation. Validate descriptor power declarations, suspend behavior, and measured current; do not enable remote wake unless explicitly implemented and tested.

## Configuration operations

The following operation names describe semantics, not a frozen binary protocol:

| Operation | Required semantics |
| --- | --- |
| GET_INFO | Return firmware/protocol version, supported action types, and limits |
| GET_CONFIG | Read current mappings and persistence state |
| SET_KEY | Validate key index, action type, length, and content; stage an edit without writing flash |
| SAVE_CONFIG | Validate the complete staged configuration, persist it, and acknowledge only after verification |
| RESET_CONFIG | Explicitly restore documented defaults; UI confirms this user action |

Use bounded payloads and reject malformed, oversized, unsupported-version, and invalid-index requests. Each request needs an unambiguous success or error result. Define transaction correlation, timeouts, reconnect behavior, and retries before freezing the protocol. Retrying a save must not unexpectedly multiply flash writes. Editing/saving must not itself type the configured phrase.

## Persistence

The target payload is three mappings of up to 32 bytes each, plus lengths, action metadata, format version, and integrity information. Build an explicit byte budget for the real writable region. Do not assume there is space for two complete configuration copies or general wear leveling.

Write only on explicit Save or reset, not every keystroke or switch press. Validate lengths and integrity before accepting stored bytes. Corrupt or unsupported records must fall back to defaults without unsolicited typing. Specify the power-loss behavior: never execute a partially written mapping; after interruption, recover either a valid configuration or known defaults. Whether the previous configuration survives an interrupted save depends on the selected storage method and must be tested.

## Browser configuration

Provide a hosted configuration page with three physical-position fields, a clear ASCII limit, connection state, unsaved-change indication, Save, and reset-to-defaults. Do not assume a deployment domain has been selected from the concept examples.

Require a secure context, check WebHID availability, and start device selection from a user interaction. Filter using the implemented device identifiers and vendor collection; verify the protocol before enabling editing. Device identifiers select compatible hardware but are not authentication credentials.

On connection, read the current configuration. On Save, stage changes, request persistence, and read back before showing success. Preserve or clearly explain unsaved edits after cancellation, device removal, permission refusal, or write failure. The user must be able to close the browser after a successful save and retain normal operation. Test Linux host access permissions as part of the supported-platform matrix rather than promising universal zero-setup browser access.

## Status LED

Proposed pattern baseline: off during startup/unconfigured USB; steady on while USB is configured; brief off pulse after an accepted key action; slow blink while an explicit configuration transaction is active. Resolve timing in firmware and ensure LED updates never block USB. Suspend should extinguish the indicator unless the validated power budget permits otherwise.

These are proposed state meanings, not behavior demonstrated by the current PCB. Save success must also be reported in the UI; the LED is not the sole persistence confirmation.

## Programming and acceptance

Validate the CH552E boot-entry method and compatible flashing tool on actual hardware. The PCB exposes RESET and USB test pads, not an SWIO debug port. Do not invent a reset polarity or boot-button sequence from another MCU family. Document fixture wiring, voltage levels, recovery, and programming steps once proven.

| Test | Pass condition |
| --- | --- |
| Enumeration | Correct keyboard and vendor interfaces on targeted desktop hosts |
| Inputs | Correct physical mapping, one action per press, debounce and startup-held-key behavior |
| Reports | Text, repeated characters, case, and explicit Enter match expectations; modifiers release |
| Persistence | Three maximum-length mappings survive unplug/replug and browser closure |
| Invalid input | Bad reports and unsupported text are rejected without corruption or input emission |
| Interrupted save | Reconnect yields a valid configuration or defaults, never partial text |
| Browser lifecycle | Permission refusal, disconnect, reconnect, and save errors have clear outcomes |
| Power/cable | Purchased 10 cm cable supports enumeration and stable use; suspend current is verified |

Unit tests should cover mapping, debounce, report sequencing, validation, and stored-record decoding. USB and persistence tests on real hardware are required in addition to host-side tests.
