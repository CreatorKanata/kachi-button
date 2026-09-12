/* Runtime macro settings and a single-writer USB staging transaction. */
#ifndef KACHI_SETTINGS_H
#define KACHI_SETTINGS_H
#include <stdint.h>
#include "config.h"
extern uint8_t settings_keys[KEY_COUNT][MACRO_BYTES];
extern volatile uint8_t settings_saving, settings_result, settings_dirty;
void settings_init(void);
uint8_t settings_valid(uint8_t *macro);
uint8_t settings_can(uint8_t request, uint16_t index);
void settings_execute(uint8_t request, uint16_t index);
void settings_cancel_edit(void);
void settings_finish_save(uint8_t ok);
#endif
