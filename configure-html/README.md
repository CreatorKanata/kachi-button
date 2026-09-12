# Browser keyboard settings

A static HTML/CSS/JavaScript settings page for Kachi Button protocol v2.
No server, Python process, libusb installation, npm packages, or network requests
are needed to use the page. Open `index.html` in desktop Chrome, keeping the
adjacent JavaScript and CSS files in the same folder.
The project-root `configure-html/` directory can be copied on its own for use;
only regenerating protocol constants requires the sibling `firmware/` sources.

## Use

1. Connect the intended Kachi Button with USB.
2. Open `index.html` in Chrome and click **Connect USB**.
3. Select Kachi Button in Chrome's USB permission picker.
4. Edit Go Go! (top, ID 0), Hi! (lower left, ID 1), and Thx (lower right, ID 2).
5. Choose **Apply & save to device** for persistent settings, or **Apply temporarily**
   for RAM-only changes. Success is shown only after device read-back verification.
6. Click **Try your buttons**, select a US/ABC input source, and press a physical key.

Key names match the printed keycaps and remain fixed when the assigned text changes.
Each key heading uses three lines: small switch position, keycap name, then small ID.
This order is retained on narrow screens.

Text supports 0–32 printable ASCII characters; empty text disables a key.
Repetitions are 1–99, and the gap between full copies is 0–60,000 ms.
No Enter or separator is added. A count of 1 sends the text once. The interval
is not a per-character delay or a guaranteed start-to-start period.

The form does not write on each keystroke. Re-reading asks before discarding
unsubmitted edits. Disconnects and errors keep form values but require a fresh
connection before further writes. A failed multi-key operation may have applied
earlier keys; read the device before retrying. Updates are atomic per key, not
across all three keys. Save persists all keys, with the firmware's existing
single-record power-loss limitations. Run only one settings writer at a time,
including other browser tabs and Python tools.

## Browser transport

The page uses `navigator.usb.requestDevice`, checks the USB identity and protocol,
and sends vendor, device-recipient control transfers on endpoint zero. It does
not claim the HID interface, reset the device, use WebHID keyboard reports, or
change firmware. Chrome requires user permission for USB selection. Browser/OS
USB access restrictions still apply; this page does not disable them.

The [WebUSB control-transfer specification](https://wicg.github.io/webusb/#usbcontroltransferparameters)
distinguishes vendor device requests from protected HID interface requests.
See also [Chrome's WebUSB documentation](https://developer.chrome.com/docs/capabilities/usb).
File-origin USB availability depends on the browser's policy. If WebUSB is absent
or permission is denied, the page shows an error; it does not start a server or
silently substitute a native bridge. Safari/Firefox support is not assumed.

## Development and validation

Protocol constants are generated from `firmware/config.py`:

```sh
python3 configure-html/generate_config.py
node --test configure-html/tests/protocol.test.cjs
```

Seven Node tests cover encoding and bounds, device identity/read protocol, exact
staged commands, temporary changes, skipped unchanged keys, stalls, bad responses,
and save failures. Node is only required for these development tests.

On September 12, 2026, these tests passed. Chrome rendered the earlier local-bridge
prototype, but the browser automation policy rejected its connect-button action.
The final server-free page's Chrome USB read/edit/save checks remain pending;
simulator tests do not demonstrate actual browser/device compatibility. No device
settings were changed by the browser test and no firmware was flashed.

## Save-completion LED

Firmware with the September 13, 2026 save-feedback change lights the LED for
1 second after a successful, verified settings save in normal HID mode. This
also applies to an unchanged save and saves from the Python tool. Temporary
apply and failed saves do not trigger it. This device-side change requires a
firmware update; changing the HTML alone cannot enable it on older firmware.

Save feedback takes priority over HID-entry and key-press indications; typing
continues and presses do not extend the 1-second duration. Another successful
save restarts it. Startup/write-wait blinking, USB suspend/reset, and ISP entry
retain priority and can cancel the indication. Native tests verify timer bounds,
wrap, failure, retrigger, and priority; physical LED timing is not measured.

## Factory chip ID

Connect USB and Read device show **Chip ID**, the CH552 factory 40-bit ID,
formatted as five uppercase hexadecimal bytes, most significant first. It is
fixed by the chip and independent of key names, assigned text, and saved settings.
The browser reads it directly with a read-only USB request; it does not enter ISP.
Older firmware returns an unsupported-request stall and shows an update notice,
while the existing settings controls remain available. Transport errors and
malformed replies are not treated as a valid ID; disconnect clears the display.

The implementation follows the [WCH CH552 datasheet, section 6.7](https://cdn-learn.adafruit.com/assets/assets/000/129/847/original/CH552DS1.PDF?1715004485=).
It reads code addresses 0x3FFA, 0x3FFF, 0x3FFE, 0x3FFD, 0x3FFC, skipping the
reserved 0x3FFB. This is different from the shared chip-model register 0x52 or
USB VID/PID. Some ISP tools show only the lower four bytes in another order.
Eight browser protocol tests now include ID formatting, legacy stalls, malformed
responses, and disconnection. Native EP0 tests verify the byte order and request
validation while saving is active.

The September 13 JST firmware was flashed and verified. Host USB reads returned
`79-BE-CA-A2-BE` twice on the connected board and confirmed saved settings were
retained. The browser uses the same request; actual browser display is unverified.

## USB manufacturer

New firmware advertises `HAPT Lab, LLC` as its USB manufacturer. The product
remains `Kachi Button`. Host tools and the browser accept both this name and
legacy `CreatorKanata` devices so existing firmware can still be upgraded.
Product/VID/PID and protocol checks remain in place; other manufacturers are
not accepted. Historical bring-up records retain the names observed at the time.
