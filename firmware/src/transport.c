/* CH552 EP1 transport, based on Deqing Sun's CH55xduino HID example (LGPL-2.1). */
#include "transport.h"
#include "boot_command.h"
void delayMicroseconds(__data uint16_t us);
static volatile __xdata uint8_t busy;
volatile __xdata uint8_t kachi_usb_epoch;
volatile __xdata uint8_t kachi_waiting;
volatile __xdata uint8_t kachi_boot_requested;
static uint8_t boot_pending;

/* A new SETUP cancels any request whose status stage was not acknowledged. */
void KachiControlCancel(void) { boot_pending = 0; }

uint8_t KachiControlSetup(void) {
    uint8_t command = boot_command_decode((uint8_t *)UsbSetupBuf, kachi_waiting);
    if (command == BOOT_COMMAND_STATUS) {
        Ep0Buffer[0] = 'K';
        Ep0Buffer[1] = 'B';
        Ep0Buffer[2] = 1; /* Protocol version. */
        Ep0Buffer[3] = kachi_waiting;
        return 4;
    }
    if (command == BOOT_COMMAND_ENTER) {
        boot_pending = 1;
        return 0;
    }
    return 0xff;
}

/* Called after EP0 IN ACK, so detach never interrupts the control transfer. */
void KachiControlComplete(void) {
    if (boot_pending) {
        boot_pending = 0;
        kachi_boot_requested = 1;
    }
}

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
    boot_pending = 0;
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
