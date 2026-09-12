/* CH552 endpoint adapter: nonblocking reports and USB lifecycle tracking. */
#ifndef KACHI_TRANSPORT_H
#define KACHI_TRANSPORT_H
#include <stdint.h>
#include "userUsbHidKeyboard/USBhandler.h"
extern volatile __xdata uint8_t kachi_usb_epoch;
extern volatile __xdata uint8_t kachi_waiting;
extern volatile __xdata uint8_t kachi_boot_requested;
void USBInit(void);
void enter_bootloader(void);
void KachiUsbReset(void);
uint8_t hid_ready(void);
uint8_t hid_send(uint8_t *report, uint8_t expected_epoch);
#endif
