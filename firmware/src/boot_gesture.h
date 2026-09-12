/* Startup-only ISP gesture; kept portable for native boundary tests. */
#ifndef KACHI_BOOT_GESTURE_H
#define KACHI_BOOT_GESTURE_H
#include <stdint.h>
#include "config.h"
enum { BOOT_NORMAL, BOOT_HOLDING, BOOT_WAIT };
void boot_gesture_init(uint8_t pressed, uint16_t now);
uint8_t boot_gesture_poll(uint8_t pressed, uint16_t now);
#endif
