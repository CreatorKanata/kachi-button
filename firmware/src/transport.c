/* CH552 EP1 transport, based on Deqing Sun's CH55xduino HID example (LGPL-2.1). */
#include "transport.h"
#include "settings.h"
void delayMicroseconds(__data uint16_t us);
static volatile __xdata uint8_t busy;
volatile __xdata uint8_t kachi_usb_epoch;
volatile __xdata uint8_t kachi_waiting;
volatile __xdata uint8_t kachi_boot_requested;
/* Match CH55xduino 0.0.26 USBCDC.c's CH552 software ISP entry sequence. */
void enter_bootloader(void) {
#if !defined(CH552) || BOOT_LOAD_ADDR != 0x3800
#error "Software ISP entry is only verified against the CH552 core source"
#endif
    USB_CTRL = 0;
    EA = 0;
    TMOD = 0;
    delayMicroseconds(50000);
    delayMicroseconds(50000);
    __asm__("lcall #0x3800");
    while (1) {} /* The ROM bootloader owns execution from this point. */
}

void USBInit(void) {
    USBDeviceCfg();
    USBDeviceEndPointCfg();
    UEP0_T_LEN = 0;
    UEP1_T_LEN = 0;
    UEP2_T_LEN = 0;
    USBDeviceIntCfg();
}

void USB_EP1_IN(void) {
    UEP1_T_LEN = 0;
    UEP1_CTRL = (UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
    busy = 0;
}

void USB_EP1_OUT(void) {}

/* Called inside USB ISR at reset/configuration/suspend to drop stale input. */
void KachiUsbReset(void) {
    KachiControlCancel();
    settings_cancel_edit();
    kachi_boot_requested = 0;
    UEP1_T_LEN = 0;
    UEP1_CTRL = (UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
    busy = 0;
    ++kachi_usb_epoch;
}

uint8_t hid_ready(void) {
    return UsbConfig && !usbSuspended && !busy;
}

uint8_t hid_send(uint8_t *report, uint8_t expected_epoch) {
    uint8_t i, sent = 0;
    /* Keep reset and report scheduling atomic without blocking on the host. */
    uint8_t saved = IE_USB;
    IE_USB = 0;
    if (hid_ready() && kachi_usb_epoch == expected_epoch) {
        for (i = 0; i < 8; ++i)
            Ep1Buffer[64 + i] = report[i];
        UEP1_T_LEN = 8;
        busy = 1;
        UEP1_CTRL = (UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
        sent = 1;
    }
    IE_USB = saved;
    return sent;
}
