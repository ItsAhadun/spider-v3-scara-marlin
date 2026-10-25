/* Config (b): defaults, no TMC UART (HAS_TMC_UART undefined -> 0 in #if). */
#define MOTHERBOARD BOARD_FYSETC_SPIDER_V2_2
#define X_DRIVER_TYPE  A4988
#define Y_DRIVER_TYPE  A4988
#define Z_DRIVER_TYPE  A4988
#define E0_DRIVER_TYPE A4988
#define EXTRUDERS 1
#define HOTENDS 1
#define E_STEPPERS 1
#define EEPROM_SETTINGS         /* assumed enabled, with no EEPROM type selected -> pin-file default applies */
#include "test_common_tail.inc"
