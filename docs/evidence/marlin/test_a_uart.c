/* Config (a): TMC2209 UART drivers on X, Y, Z, E0 (HAS_TMC_UART true). */
#define MOTHERBOARD BOARD_FYSETC_SPIDER_V2_2
#define X_DRIVER_TYPE  TMC2209
#define Y_DRIVER_TYPE  TMC2209
#define Z_DRIVER_TYPE  TMC2209
#define E0_DRIVER_TYPE TMC2209
#define HAS_TMC_UART 1          /* what Marlin's Conditionals derive from TMC2209 drivers; the pin files only test this */
#define EXTRUDERS 1
#define HOTENDS 1
#define E_STEPPERS 1
#define EEPROM_SETTINGS         /* assumed enabled, with no EEPROM type selected -> pin-file default applies */
#include "test_common_tail.inc"
