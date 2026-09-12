/* Model the real shared setup/response buffer for native EP0 adapter tests. */
#ifndef KACHI_TEST_USB_STUB_H
#define KACHI_TEST_USB_STUB_H
#define KACHI_TRANSPORT_H
#include <stdint.h>
typedef struct {
    uint8_t bRequestType, bRequest, wValueL, wValueH;
    uint8_t wIndexL, wIndexH, wLengthL, wLengthH;
} Setup;
extern uint8_t Ep0Buffer[8];
#define UsbSetupBuf ((Setup *)Ep0Buffer)
extern volatile uint8_t kachi_waiting, kachi_boot_requested;
void KachiControlCancel(void);
uint8_t KachiControlSetup(void);
void KachiControlComplete(void);
#endif
