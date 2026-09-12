"""Firmware build configuration: keep board wiring and behavior in one place."""

CORE_VERSION = "0.0.26"
FQBN = "CH55xDuino:mcs51:ch552:clock=24internal,usb_settings=user148,bootloader_pin=p36"
PINS = (15, 16, 17)
TEXTS = ("Go Go!", "Hi!", "Thx")  # Physical IDs 0, 1, 2; no Enter suffix.
LED_PIN = 14
DEBOUNCE_MS = 15
BOOT_HOLD_MS = 2000
BOOT_LED_HALF_PERIOD_MS = 250
BOOT_WAIT_LED_HALF_PERIOD_MS = 75
BOOT_STATUS_REQUEST = 0x5A
BOOT_ENTER_REQUEST = 0x5B
BOOT_COMMAND_MAGIC = 0x4B42
USB_CONTROL_TIMEOUT_MS = 2000
ISP_SCAN_SECONDS = 10
USB_VID = 0x1209
USB_PID = 0xC55D  # Retain CH55xduino's development keyboard identifier.
USB_POWER_MA = 100
USB_PRODUCT = "Kachi Button"
USB_MANUFACTURER = "CreatorKanata"


def header():
    """Generate the same constants for the device build and native tests."""
    allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ !"
    assert len(TEXTS) == len(PINS)
    assert all(text and all(c in allowed for c in text) for text in TEXTS)
    import json
    return f"""/* Generated from config.py: PCB v1 wiring and fixed HID mapping. */
#ifndef KACHI_CONFIG_H
#define KACHI_CONFIG_H
#define KEY_COUNT {len(PINS)}
#define KEY_PINS {{{', '.join(map(str, PINS))}}}
#define KEY_TEXTS {{{', '.join(json.dumps(text) for text in TEXTS)}}}
#define TEXT_CAPACITY {max(map(len, TEXTS)) + 1}
#define LED_PIN {LED_PIN}
#define DEBOUNCE_MS {DEBOUNCE_MS}
#define BOOT_HOLD_MS {BOOT_HOLD_MS}
#define BOOT_LED_HALF_PERIOD_MS {BOOT_LED_HALF_PERIOD_MS}
#define BOOT_WAIT_LED_HALF_PERIOD_MS {BOOT_WAIT_LED_HALF_PERIOD_MS}
#define BOOT_STATUS_REQUEST {BOOT_STATUS_REQUEST}
#define BOOT_ENTER_REQUEST {BOOT_ENTER_REQUEST}
#define BOOT_COMMAND_MAGIC {BOOT_COMMAND_MAGIC}
#define ALL_KEYS_MASK {(1 << len(PINS)) - 1}
#define HID_LEFT_SHIFT 0x02
#define HID_CAPS_LOCK_LED 0x02
#endif
"""
