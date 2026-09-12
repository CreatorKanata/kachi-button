/* Kachi Button PCB v1: Go Go! / Hi! / Thx, one text macro per press. */
#include "src/keys.h"
#include "src/text.h"
#include "src/transport.h"
#include "src/boot_gesture.h"
#include "src/settings.h"
#include "src/storage.h"
#include "src/key_led.h"

static const uint8_t pins[KEY_COUNT] = KEY_PINS;
static uint8_t report[8];
static uint8_t epoch;

static uint8_t read_keys(void) {
    uint8_t i, pressed = 0;
    for (i = 0; i < KEY_COUNT; ++i)
        if (!digitalRead(pins[i]))
            pressed |= 1 << i;
    return pressed;
}

void setup(void) {
    uint8_t i;
    digitalWrite(LED_PIN, HIGH);
    pinMode(LED_PIN, OUTPUT);
    for (i = 0; i < KEY_COUNT; ++i)
        pinMode(pins[i], INPUT_PULLUP);
    keys_reset(read_keys(), (uint16_t)millis());
    boot_gesture_init(read_keys(), (uint16_t)millis());
    settings_init();
    text_reset();
    USBInit();
}

void loop(void) {
    uint16_t now = (uint16_t)millis();
    uint8_t pressed = read_keys();
    uint8_t action;
    uint8_t boot = boot_gesture_poll(pressed, now);
    /* Save runs outside the USB ISR; EEPROM writes mask interrupts per byte. */
    if (settings_saving)
        settings_finish_save(storage_save());
    if (kachi_boot_requested) {
        digitalWrite(LED_PIN, HIGH);
        enter_bootloader();
    }
    /* Latch waiting before USB lifecycle handling: bus reset cannot cancel it. */
    if (boot != BOOT_NORMAL) {
        keys_reset(pressed, now);
        text_reset();
        key_led_reset();
        kachi_waiting = boot == BOOT_WAIT;
        digitalWrite(LED_PIN, (now / (kachi_waiting ?
            BOOT_WAIT_LED_HALF_PERIOD_MS : BOOT_LED_HALF_PERIOD_MS)) & 1);
        return;
    }
    if (!UsbConfig || usbSuspended || epoch != kachi_usb_epoch) {
        epoch = kachi_usb_epoch;
        keys_reset(pressed, now);
        text_reset();
        key_led_reset();
        digitalWrite(LED_PIN, HIGH);
        return;
    }
    action = keys_scan(pressed, now, !text_busy() && hid_ready());
    digitalWrite(LED_PIN, key_led_off(keys_pressed_edges, now) ? HIGH : LOW);
    if (action) {
        /* Snapshot a whole macro atomically against USB configuration updates. */
        uint8_t saved = IE_USB;
        IE_USB = 0;
        text_start(action);
        IE_USB = saved;
    }
    text_tick(now);
    if (hid_ready() && text_peek(keyboardLedStatus & HID_CAPS_LOCK_LED, report))
        if (hid_send(report, epoch))
            text_accepted(now);
}
