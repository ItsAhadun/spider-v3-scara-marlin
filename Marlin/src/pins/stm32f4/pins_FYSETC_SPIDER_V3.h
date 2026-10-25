/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2026 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#pragma once

/**
 * FYSETC Spider V3.0 (STM32F446VET6, 12MHz crystal)
 *
 * Pins taken from the FYSETC "Spider V3.0" pinout sheet and the v3.0 schematic (June 2023 set).
 * Comments name the pinout label and/or schematic net for each pin.
 *
 * This board is NOT pin-compatible with the Spider V1.x/V2.x files (pins_FYSETC_SPIDER*.h):
 * X_ENABLE is PE9 and every TMC2208/2209 UART line is the socket's own CS pin.
 *
 * Stepper sockets:  M0=X  M1=Y  M2=Z  M3=E0  M4=E1  M5=E2  M6=E3  M7=E4
 * M0/M1 are the HV sockets (up to 60V). EN/STEP/DIR are buffered by 74HCT365 (non-inverting).
 */

#include "env_validate.h"

#if HOTENDS > 3 || E_STEPPERS > 5
  #error "FYSETC Spider V3 supports up to 3 hotends / 5 E steppers."
#endif

#ifndef BOARD_INFO_NAME
  #define BOARD_INFO_NAME "FYSETC Spider V3"
#endif
#define BOARD_WEBSITE_URL "github.com/FYSETC/FYSETC-SPIDER"
#ifndef DEFAULT_MACHINE_NAME
  #define DEFAULT_MACHINE_NAME BOARD_INFO_NAME
#endif

// Avoid conflict with TIMER_TONE (TIM6) defined in the MARLIN_F446VE_FYSETC variant
#define STEP_TIMER                            10

#define USES_DIAG_JUMPERS

//
// EEPROM
//
#if NO_EEPROM_SELECTED
  #define I2C_EEPROM                              // Onboard AT24C32 (U7)
  //#define FLASH_EEPROM_EMULATION
#endif

#if ENABLED(I2C_EEPROM)
  #define MARLIN_EEPROM_SIZE             0x1000U  // 4K (AT24C32, A2..A0 = 000 -> 0x50)
#elif ENABLED(FLASH_EEPROM_EMULATION)
  // Decrease delays and flash wear by spreading writes across the
  // 128 kB sector allocated for EEPROM emulation.
  #define FLASH_EEPROM_LEVELING
#endif

//
// I2C (I2C1: AT24C32 EEPROM U7 and the 4-pin I2C header J7)
//
#define I2C_SCL_PIN                         PB8   // PB8/SCL
#define I2C_SDA_PIN                         PB9   // PB9/SDA

//
// Servos
// Probe header J36 "SIG" (net BL_PWM). Shares PA2 with Y-MAX and the M4 (E1) DIAG jumper.
//
#ifndef SERVO0_PIN
  #define SERVO0_PIN                        PA2   // BL_PWM / Y-MAX
#endif

//
// Probe enable
//
#if ENABLED(PROBE_ENABLE_DISABLE) && !defined(PROBE_ENABLE_PIN)
  #define PROBE_ENABLE_PIN            SERVO0_PIN
#endif

//
// Trinamic StallGuard pins
// Each 2-pin DIAG jumper connects a driver's DIAG output to an endstop connector:
//   M0 (X)  -> X-MIN    M3 (E0) -> Z-MAX (probe port)
//   M1 (Y)  -> Y-MIN    M4 (E1) -> Y-MAX (also probe header SIG / SERVO0)
//   M2 (Z)  -> Z-MIN    M5 (E2) -> X-MAX
// M6 (E3) and M7 (E4) have no DIAG connection.
// Remove a socket's DIAG jumper when a switch, probe or servo uses that connector.
//
#define X_DIAG_PIN                          PB14  // X-MIN
#define Y_DIAG_PIN                          PB13  // Y-MIN
#define Z_DIAG_PIN                          PA0   // Z-MIN
#define E0_DIAG_PIN                         PA3   // Z-MAX
#define E1_DIAG_PIN                         PA2   // Y-MAX
#define E2_DIAG_PIN                         PA1   // X-MAX

//
// Limit Switches
// With sensorless homing the STOP pin is the DIAG pin, whatever the homing direction.
//
#ifdef X_STALL_SENSITIVITY
  #define X_STOP_PIN                  X_DIAG_PIN
  #define X_OTHR_PIN                        PA1   // X-MAX
#else
  #define X_MIN_PIN                         PB14  // X-MIN
  #define X_MAX_PIN                         PA1   // X-MAX
#endif

#ifdef Y_STALL_SENSITIVITY
  #define Y_STOP_PIN                  Y_DIAG_PIN
  #define Y_OTHR_PIN                        PA2   // Y-MAX
#else
  #define Y_MIN_PIN                         PB13  // Y-MIN
  #define Y_MAX_PIN                         PA2   // Y-MAX
#endif

#ifdef Z_STALL_SENSITIVITY
  #define Z_STOP_PIN                  Z_DIAG_PIN
  #define Z_OTHR_PIN                        PA3   // Z-MAX
#else
  #define Z_MIN_PIN                         PA0   // Z-MIN (also probe header J36 "Z-")
  #define Z_MAX_PIN                         PA3   // Z-MAX
#endif

//
// Z Probe (when not Z_MIN_PIN)
// "Z-MAX" connector J40: 100R + BAT85 + 10k pull-up, supply V_PROBE (24V or 5V jumper).
// Input only. The probe must pull the signal low (NPN / switch to GND).
//
#ifndef Z_MIN_PROBE_PIN
  #define Z_MIN_PROBE_PIN                   PA3   // Z-MAX / PROBE
#endif

//
// Filament Runout Sensor (no dedicated connector, use X-MAX)
//
#ifndef FIL_RUNOUT_PIN
  #define FIL_RUNOUT_PIN                    PA1   // X-MAX
#endif

//
// Steppers
//
#define X_STEP_PIN                          PE11  // M0 STEP (PE11_X_STEP)
#define X_DIR_PIN                           PE10  // M0 DIR  (PE10_X_DIR)
#define X_ENABLE_PIN                        PE9   // M0 EN   (PE9_X_EN)
#ifndef X_CS_PIN
  #define X_CS_PIN                          PE7   // M0 CS/PDN (X_CS)
#endif

#define Y_STEP_PIN                          PD8   // M1 STEP (PD8_Y_STEP)
#define Y_DIR_PIN                           PB12  // M1 DIR  (PB12_Y_DIR)
#define Y_ENABLE_PIN                        PD9   // M1 EN   (PD9_Y_EN)
#ifndef Y_CS_PIN
  #define Y_CS_PIN                          PE15  // M1 CS/PDN (Y_CS)
#endif

#define Z_STEP_PIN                          PD14  // M2 STEP (PD14_Z_STEP)
#define Z_DIR_PIN                           PD13  // M2 DIR  (PD13_Z_DIR)
#define Z_ENABLE_PIN                        PD15  // M2 EN   (PD15_Z_EN)
#ifndef Z_CS_PIN
  #define Z_CS_PIN                          PD10  // M2 CS/PDN (Z_CS)
#endif

#define E0_STEP_PIN                         PD5   // M3 STEP (PD5_E0_STEP)
#define E0_DIR_PIN                          PD6   // M3 DIR  (PD6_E0_DIR)
#define E0_ENABLE_PIN                       PD4   // M3 EN   (PD4_E0_EN)
#ifndef E0_CS_PIN
  #define E0_CS_PIN                         PD7   // M3 CS/PDN (E0_CS)
#endif

#define E1_STEP_PIN                         PE6   // M4 STEP (PE6_E1_STEP)
#define E1_DIR_PIN                          PC13  // M4 DIR  (PC13_E1_DIR)
#define E1_ENABLE_PIN                       PE5   // M4 EN   (PE5_E1_EN)
#ifndef E1_CS_PIN
  #define E1_CS_PIN                         PC14  // M4 CS/PDN (E1_CS)
#endif

#define E2_STEP_PIN                         PE2   // M5 STEP (PE2_E2_STEP)
#define E2_DIR_PIN                          PE4   // M5 DIR  (PE4_E2_DIR)
#define E2_ENABLE_PIN                       PE3   // M5 EN   (PE3_E2_EN)
#ifndef E2_CS_PIN
  #define E2_CS_PIN                         PC15  // M5 CS/PDN (E2_CS)
#endif

#define E3_STEP_PIN                         PD12  // M6 STEP (PD12_E3_STEP)
#define E3_DIR_PIN                          PC4   // M6 DIR  (PC4_E3_DIR)
#define E3_ENABLE_PIN                       PE8   // M6 EN   (PE8_E3_EN)
#ifndef E3_CS_PIN
  #define E3_CS_PIN                         PA15  // M6 CS/PDN (E3_CS)
#endif

#define E4_STEP_PIN                         PE1   // M7 STEP (PE1_E4_STEP)
#define E4_DIR_PIN                          PE0   // M7 DIR  (PE0_E4_DIR)
#define E4_ENABLE_PIN                       PC5   // M7 EN   (PC5_E4_EN)
#ifndef E4_CS_PIN
  #define E4_CS_PIN                         PD11  // M7 CS/PDN (E4_CS)
#endif

//
// Software SPI pins for TMC2130/TMC2160/TMC2240/TMC5160 stepper drivers
// "MotorSPI" bus (SPI4 pins, not Marlin's default SPI) reaches each socket through its jumper block
//
#ifndef TMC_USE_SW_SPI
  #define TMC_USE_SW_SPI
#endif
#if ENABLED(TMC_USE_SW_SPI)
  #ifndef TMC_SPI_MOSI
    #define TMC_SPI_MOSI                    PE14  // SPI4_MOSI
  #endif
  #ifndef TMC_SPI_MISO
    #define TMC_SPI_MISO                    PE13  // SPI4_MISO
  #endif
  #ifndef TMC_SPI_SCK
    #define TMC_SPI_SCK                     PE12  // SPI4_SCK
  #endif
#endif

#if HAS_TMC_UART
  /**
   * TMC2208/TMC2209 stepper drivers
   *
   * Single-wire UART: each socket's CS pin -> 100R -> CS jumper -> module pin 4 (PDN_UART).
   * Only TX is defined; pins_postprocess.h sets each *_SERIAL_RX_PIN to the same pin.
   */
  #define X_SERIAL_TX_PIN                   PE7   // M0 CS/PDN
  #define Y_SERIAL_TX_PIN                   PE15  // M1 CS/PDN
  #define Z_SERIAL_TX_PIN                   PD10  // M2 CS/PDN
  #define E0_SERIAL_TX_PIN                  PD7   // M3 CS/PDN
  #define E1_SERIAL_TX_PIN                  PC14  // M4 CS/PDN
  #define E2_SERIAL_TX_PIN                  PC15  // M5 CS/PDN
  #define E3_SERIAL_TX_PIN                  PA15  // M6 CS/PDN
  #define E4_SERIAL_TX_PIN                  PD11  // M7 CS/PDN

  // Reduce baud rate to improve software serial reliability
  #ifndef TMC_BAUD_RATE
    #define TMC_BAUD_RATE                  19200
  #endif
#endif

//
// Temperature Sensors
//
#define TEMP_0_PIN                          PC0   // T0 (THERM_END0)
#define TEMP_1_PIN                          PC1   // T1 (THERM_END1)
#define TEMP_2_PIN                          PC2   // T2 (THERM_END2)
#define TEMP_3_PIN                          PC3   // T3 (THERM_END3)
#define TEMP_4_PIN                          PB1   // T4 (THERM_END4)
#ifndef TEMP_BED_PIN
  #define TEMP_BED_PIN                      PB0   // TB (THERM_BED)
#endif

//
// Heaters / Fans
//
#ifndef HEATER_0_PIN
  #define HEATER_0_PIN                      PB15  // HEAT0 (E0_Heater)
#endif
#ifndef HEATER_1_PIN
  #define HEATER_1_PIN                      PC8   // HEAT1 (E1_Heater)
#endif
#ifndef HEATER_2_PIN
  #define HEATER_2_PIN                      PB3   // HEAT2 (E2_Heater)
#endif
#ifndef HEATER_BED_PIN
  #define HEATER_BED_PIN                    PB4   // BED-OUT (BED_Heater)
#endif

// FAN0 (PA13) and FAN1 (PA14) are the SWD pins and have no timer channel.
// Using them as fans disables SWD debugging once Marlin is running.
#define FAN_SOFT_PWM_REQUIRED

#ifndef FAN0_PIN
  #define FAN0_PIN                          PA13  // FAN0 (PWM_FAN0 / SWDIO)
#endif
#ifndef FAN1_PIN
  #define FAN1_PIN                          PA14  // FAN1 (PWM_FAN1 / SWCLK)
#endif
#ifndef FAN2_PIN
  #define FAN2_PIN                          PB2   // FAN2 (PWM_FAN2 / BOOT1)
#endif

/**
 *                 ------                                      ------
 *  (BEEPER) PC9  | 1  2 | PA8  (BTN_ENC)         (MISO) PA6  | 1  2 | PA5  (SCK)
 *  (LCD_EN) PC11 | 3  4 | PD2  (LCD_RS)       (BTN_EN1) PC6  | 3  4 | PA4  (SD_SS)
 *  (LCD_D4) PC10   5  6 | PC12 (LCD_D5)       (BTN_EN2) PC7    5  6 | PA7  (MOSI)
 *  (LCD_D6) PD0  | 7  8 | PD1  (LCD_D7)     (SD_DETECT) PB10 | 7  8 | RESET
 *            GND | 9 10 | 5V                             GND | 9 10 | 5V
 *                 ------                                      ------
 *                  EXP1                                        EXP2
 *
 * EXP1-7 (PD0) and EXP1-8 (PD1) are also the CAN transceiver RXD / TXD (CAN_RX / CAN_TX).
 * The transceiver always drives PD0, so a display that drives EXP1-7 as an output will fight it.
 * EXP2 shares SPI1, SD_SS (PA4) and SD_DETECT (PB10) with the onboard TF card.
 */
#define EXP1_01_PIN                         PC9   // BEEP
#define EXP1_02_PIN                         PA8   // BTN_ENC
#define EXP1_03_PIN                         PC11  // LCD_EN
#define EXP1_04_PIN                         PD2   // LCD_RS
#define EXP1_05_PIN                         PC10  // LCD_D4
#define EXP1_06_PIN                         PC12  // LCD_D5
#define EXP1_07_PIN                         PD0   // LCD_D6 / CAN_RX
#define EXP1_08_PIN                         PD1   // LCD_D7 / CAN_TX

#define EXP2_01_PIN                         PA6   // SPI1.MISO
#define EXP2_02_PIN                         PA5   // SPI1.SCK
#define EXP2_03_PIN                         PC6   // BTN_EN1
#define EXP2_04_PIN                         PA4   // SPI1.SS
#define EXP2_05_PIN                         PC7   // BTN_EN2
#define EXP2_06_PIN                         PA7   // SPI1.MOSI
#define EXP2_07_PIN                         PB10  // CD (SD_DET)
#define EXP2_08_PIN                         -1    // RESET (MCU NRST)

//
// SD Connection
//
#ifndef SDCARD_CONNECTION
  #define SDCARD_CONNECTION              ONBOARD
#endif

//
// SPI / SD Card
// The onboard TF card and EXP2 share SPI1, SS (PA4) and card detect (PB10),
// so ONBOARD and LCD use the same pins. Use only one card at a time.
//
#if SD_CONNECTION_IS(ONBOARD) || SD_CONNECTION_IS(LCD)
  #define SD_SCK_PIN                 EXP2_02_PIN
  #define SD_MISO_PIN                EXP2_01_PIN
  #define SD_MOSI_PIN                EXP2_06_PIN
  #define SD_SS_PIN                  EXP2_04_PIN
  #define SD_DETECT_PIN              EXP2_07_PIN
#elif SD_CONNECTION_IS(CUSTOM_CABLE)
  #error "No custom SD drive cable defined for this board."
#endif

//
// LCD / Controller
//
#if ENABLED(FYSETC_242_OLED_12864)

  #define BTN_EN1                    EXP1_01_PIN
  #define BTN_EN2                    EXP1_08_PIN
  #define BTN_ENC                    EXP1_02_PIN

  #define BEEPER_PIN                 EXP2_03_PIN

  #define LCD_PINS_DC                EXP1_06_PIN
  #define LCD_PINS_RS                EXP2_05_PIN  // LCD_RST
  #define DOGLCD_CS                  EXP1_04_PIN
  #define DOGLCD_MOSI                EXP1_05_PIN
  #define DOGLCD_SCK                 EXP1_03_PIN
  #define DOGLCD_A0                  LCD_PINS_DC
  #define FORCE_SOFT_SPI

  #define KILL_PIN                          -1    // NC
  #define NEOPIXEL_PIN               EXP1_07_PIN

#elif HAS_WIRED_LCD

  #define BEEPER_PIN                 EXP1_01_PIN
  #define BTN_ENC                    EXP1_02_PIN

  #if ENABLED(CR10_STOCKDISPLAY)

    #define LCD_PINS_RS              EXP1_07_PIN

    #define BTN_EN1                  EXP1_03_PIN
    #define BTN_EN2                  EXP1_05_PIN

    #define LCD_PINS_EN              EXP1_08_PIN
    #define LCD_PINS_D4              EXP1_06_PIN

  #else

    #define LCD_PINS_RS              EXP1_04_PIN

    #define BTN_EN1                  EXP2_03_PIN
    #define BTN_EN2                  EXP2_05_PIN

    #define LCD_SDSS_PIN             EXP2_04_PIN

    #define LCD_PINS_EN              EXP1_03_PIN
    #define LCD_PINS_D4              EXP1_05_PIN

    #if ENABLED(FYSETC_MINI_12864)
      // See https://wiki.fysetc.com/Mini12864_Panel
      #define DOGLCD_CS              EXP1_03_PIN
      #define DOGLCD_A0              EXP1_04_PIN
      #if ENABLED(FYSETC_GENERIC_12864_1_1)
        #define LCD_BACKLIGHT_PIN    EXP1_07_PIN
      #endif
      #define LCD_RESET_PIN          EXP1_05_PIN  // Must be high or open for LCD to operate normally.
      #if ANY(FYSETC_MINI_12864_1_2, FYSETC_MINI_12864_2_0)
        #ifndef RGB_LED_R_PIN
          #define RGB_LED_R_PIN      EXP1_06_PIN
        #endif
        #ifndef RGB_LED_G_PIN
          #define RGB_LED_G_PIN      EXP1_07_PIN
        #endif
        #ifndef RGB_LED_B_PIN
          #define RGB_LED_B_PIN      EXP1_08_PIN
        #endif
      #elif ENABLED(FYSETC_MINI_12864_2_1)
        #define NEOPIXEL_PIN         EXP1_06_PIN
      #endif
    #endif

    #if IS_ULTIPANEL
      #define LCD_PINS_D5            EXP1_06_PIN
      #define LCD_PINS_D6            EXP1_07_PIN
      #define LCD_PINS_D7            EXP1_08_PIN
      #if ENABLED(REPRAP_DISCOUNT_FULL_GRAPHIC_SMART_CONTROLLER)
        #define BTN_ENC_EN           LCD_PINS_D7  // Detect the presence of the encoder
      #endif
    #endif

  #endif

#endif // HAS_WIRED_LCD

// Alter timing for graphical display
#if IS_U8GLIB_ST7920
  #define BOARD_ST7920_DELAY_1                96
  #define BOARD_ST7920_DELAY_2                48
  #define BOARD_ST7920_DELAY_3               640
#endif

//
// RGB LED strip / extra fans
// The pinout's "FAN3" (G), "FAN4" (R) and "FAN5" (B) are the three MOSFET channels
// (PWM_LED_G/R/B) that also drive the 4-pin VCC/B/R/G strip header J17.
// Use them for RGB_LED / RGBW_LED or as three extra fans, not both.
// The FYSETC Mini 12864 1.2/2.0 drives its own RGB LEDs from EXP1, which leaves these free as fans.
//
#if ANY(RGB_LED, RGBW_LED) && NONE(FYSETC_MINI_12864_1_2, FYSETC_MINI_12864_2_0)
  #ifndef RGB_LED_R_PIN
    #define RGB_LED_R_PIN                   PB6   // FAN4 (PWM_LED_R)
  #endif
  #ifndef RGB_LED_G_PIN
    #define RGB_LED_G_PIN                   PB5   // FAN3 (PWM_LED_G)
  #endif
  #ifndef RGB_LED_B_PIN
    #define RGB_LED_B_PIN                   PB7   // FAN5 (PWM_LED_B)
  #endif
#else
  #ifndef FAN3_PIN
    #define FAN3_PIN                        PB5   // FAN3 (PWM_LED_G)
  #endif
  #ifndef FAN4_PIN
    #define FAN4_PIN                        PB6   // FAN4 (PWM_LED_R)
  #endif
  #ifndef FAN5_PIN
    #define FAN5_PIN                        PB7   // FAN5 (PWM_LED_B)
  #endif
#endif

//
// NeoPixel LED
// 5V RGB header J19 "DATA(PD3)", buffered by 74HC1G125. PD3 also lights the blue debug LED.
// Some FYSETC displays use an EXP1 pin for NeoPixels, so the onboard header becomes the 2nd strip.
//
#if ANY(FYSETC_242_OLED_12864, FYSETC_MINI_12864_2_1)
  #ifndef NEOPIXEL2_PIN
    #define NEOPIXEL2_PIN                   PD3   // Neo-pixel
  #endif
#elif !defined(NEOPIXEL_PIN)
  #define NEOPIXEL_PIN                      PD3   // Neo-pixel
#endif

/**
 * Other board signals (no Marlin pin macro):
 *  - Raspberry Pi header J22: TX = PA9, RX = PA10 (USART1). Use SERIAL_PORT_2 1 for a host on this header.
 *  - CAN (J20, transceiver U14): CAN1 RX = PD0, TX = PD1, shared with EXP1-7 / EXP1-8.
 *  - USB-C: DM = PA11, DP = PA12.
 *  - SWD (J62): SWDIO = PA13, SWCLK = PA14, shared with FAN0 / FAN1.
 */
