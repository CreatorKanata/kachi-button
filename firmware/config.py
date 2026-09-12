"""Firmware build configuration: keep board wiring and behavior in one place."""

CORE_VERSION = "0.0.26"
FQBN = "CH55xDuino:mcs51:ch552:clock=24internal,usb_settings=user148,bootloader_pin=p36"
PINS = (15, 16, 17)
TEXTS = ("Go Go!", "Hi!", "Thx")  # Physical IDs 0, 1, 2; no Enter suffix.
MAX_TEXT_LENGTH = 32
MAX_REPEAT = 99
MAX_INTERVAL_MS = 60000
CONFIG_INFO_REQUEST = 0x60
CONFIG_GET_REQUEST = 0x61
CONFIG_BEGIN_REQUEST = 0x62
CONFIG_BYTE_REQUEST = 0x63
CONFIG_APPLY_REQUEST = 0x64
CONFIG_SAVE_REQUEST = 0x65
SETTINGS_SCHEMA = 1
SETTINGS_COMMIT_MARK = 0xA5
EEPROM_CAPACITY = 128
LED_PIN = 14
DEBOUNCE_MS = 15
KEY_LED_PULSE_MS = 30
HID_START_LED_MS = 2000
BOOT_HOLD_MS = 2000
BOOT_LED_HALF_PERIOD_MS = 250
BOOT_WAIT_LED_HALF_PERIOD_MS = 75
BOOT_STATUS_REQUEST = 0x5A
BOOT_ENTER_REQUEST = 0x5B
BOOT_COMMAND_MAGIC = 0x4B42
MIN_MACOS_LIBUSB = (1, 0, 30)
USB_CONTROL_TIMEOUT_MS = 2000
ISP_SCAN_SECONDS = 10
USB_VID = 0x1209
USB_PID = 0xC55D  # Retain CH55xduino's development keyboard identifier.
USB_POWER_MA = 100
USB_PRODUCT = "Kachi Button"
USB_MANUFACTURER = "CreatorKanata"


def header():
    """Generate the same constants for the device build and native tests."""
    allowed = "".join(chr(c) for c in range(32, 127))
    assert len(TEXTS) == len(PINS)
    assert all(text and len(text) <= MAX_TEXT_LENGTH and all(c in allowed for c in text) for text in TEXTS)
    import json
    return f"""/* Generated from config.py: PCB v1 wiring and fixed HID mapping. */
#ifndef KACHI_CONFIG_H
#define KACHI_CONFIG_H
#define KEY_COUNT {len(PINS)}
#define KEY_PINS {{{', '.join(map(str, PINS))}}}
#define KEY_TEXTS {{{', '.join(json.dumps(text) for text in TEXTS)}}}
#define MAX_TEXT_LENGTH {MAX_TEXT_LENGTH}
#define MAX_REPEAT {MAX_REPEAT}
#define MAX_INTERVAL_MS {MAX_INTERVAL_MS}
#define MACRO_BYTES (MAX_TEXT_LENGTH + 4)
#define CONFIG_INFO_REQUEST {CONFIG_INFO_REQUEST}
#define CONFIG_GET_REQUEST {CONFIG_GET_REQUEST}
#define CONFIG_BEGIN_REQUEST {CONFIG_BEGIN_REQUEST}
#define CONFIG_BYTE_REQUEST {CONFIG_BYTE_REQUEST}
#define CONFIG_APPLY_REQUEST {CONFIG_APPLY_REQUEST}
#define CONFIG_SAVE_REQUEST {CONFIG_SAVE_REQUEST}
#define SETTINGS_SCHEMA {SETTINGS_SCHEMA}
#define SETTINGS_COMMIT_MARK {SETTINGS_COMMIT_MARK}
#define EEPROM_CAPACITY {EEPROM_CAPACITY}
#define LED_PIN {LED_PIN}
#define DEBOUNCE_MS {DEBOUNCE_MS}
#define KEY_LED_PULSE_MS {KEY_LED_PULSE_MS}
#define HID_START_LED_MS {HID_START_LED_MS}
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
