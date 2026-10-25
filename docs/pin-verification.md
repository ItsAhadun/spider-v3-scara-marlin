# Spider V3 (STM32F446) vs Marlin `BOARD_FYSETC_SPIDER_V2_2`: pin verification

**Verdict:** Marlin's Spider V2.2 pin map is **not usable as-is** on this board with TMC2209s in UART mode.
75 Marlin pin definitions match the V3 hardware exactly. **13 do not**: `X_ENABLE_PIN` and every TMC2209 UART
TX/RX pin for X, Y, Z, E0, E1 and E2. A custom pin file for the V3 is required.

## Sources (three independent extractions, none saw the others)
The FYSETC documents live in [FYSETC/FYSETC-SPIDER/hardware/V3.0](https://github.com/FYSETC/FYSETC-SPIDER/tree/main/hardware/V3.0).

| # | Source | What it is | Method |
|---|---|---|---|
| A | `Spider_V3.0_Pinout-01.pdf` | FYSETC official V3.0 pinout sheet (image-only), read at 400 dpi | Visual transcription. Inset and silkscreen read separately: 0 disagreements on driver pins |
| B | `spider_V3.0_sch.pdf` (pages 15–28, rev dated June 2023) | FYSETC official V3.0 schematic | Traced from the MCU pin through the 74HCT365 buffers to the socket/connector |
| C | Marlin `bugfix-2.1.x` @ `2ef3ee62fd46bd0c5d4304732ef8e255e2bb9869` | `boards.h`, `pins.h`, `pins_FYSETC_SPIDER_V2_2.h` → `pins_FYSETC_SPIDER.h` → `pins_FYSETC_S6.h` → `pins_FYSETC_S6_common.h` | Manual reading, then confirmed with `gcc -E -dM/-dD` and `clang -E -dM`: 0 differences |

**Hardware sources A and B agree with each other on all 97 comparable signals.**

## The mismatches
| Signal | V3 pinout (A) | V3 schematic (B) | Marlin effective value (C) | Why Marlin ends up wrong |
|---|---|---|---|---|
| X driver EN | **PE9** | **PE9** (net `PE9_X_EN`, MCU pin 40) | `X_ENABLE_PIN` = **PE12** | `pins_FYSETC_SPIDER.h:68` sets PE9, then `pins_FYSETC_S6.h:29` redefines it as PE12 with no `#ifndef`/`#undef`. GCC warns `"X_ENABLE_PIN" redefined`. On the V3, PE12 is SPI4_SCK. |
| X UART (PDN) | PE7 | PE7 | TX = PE9, RX = PE8 | SPIDER.h:77 sets TX PE7. S6.h:35-36 overrides TX and adds RX. PE9 is the X **enable** pin, so UART traffic would toggle the driver's enable. |
| Y UART | PE15 | PE15 | TX = PE14, RX = PE13 | SPIDER.h:79 → S6.h:37-38 (PE14/PE13 are the motor-SPI MOSI/MISO) |
| Z UART | PD10 | PD10 | TX = PD11, RX = PD12 | SPIDER.h:80 → S6.h:39-40 (PD11 = M7 CS, PD12 = M6 STEP) |
| E0 UART | PD7 | PD7 | TX = PD3, RX = PA15 | SPIDER.h:82 → S6.h:41-42 (PD3 = Neopixel, PA15 = M6 CS) |
| E1 UART | PC14 | PC14 | TX = PC4, RX = PC5 | SPIDER.h:83 → S6.h:43-44 |
| E2 UART | PC15 | PC15 | TX = PE1, RX = PE0 | SPIDER.h:84 → S6.h:45-46 |

The Spider's UART is **single-wire**: the schematic shows each socket's CS line → 100 Ω → jumper → module pin 4 (PDN_UART),
with no separate TX/RX bridge. So the correct configuration is TX = RX = the socket's CS/PDN pin.

**Root cause:** Marlin commit `6f71a141` (2024-12-17, "🐛 Fix S6 V2 TMC communication error", PR #27595) rewrote
`pins_FYSETC_S6.h` and removed the `#ifndef` guards. The Spider file includes `pins_FYSETC_S6.h` and relied on those guards.
The GCC output in `evidence/marlin/stderr_dD_a_uart.txt` shows 8 "redefined" warnings, at S6.h lines 24, 29, 35, 37, 39, 41, 43 and 45.

**Stable Marlin 2.1.2.9 is also wrong, in a different way:** there the S6 file still has the guards
(`pins_FYSETC_S6.h:86` `#ifndef X_ENABLE_PIN`; :120-156 guarded TX/RX). EN and TX therefore come out correct (PE9; PE7/PE15/PD10/PD7),
but the Spider file never defines RX pins, so the S6 RX values (PE8/PE13/PD12/PA15/PC5/PE0) still apply. Marlin would
read driver replies on the wrong pins.

## Required corrections for a Spider V3 pin file
```
X_ENABLE_PIN      PE9
X_SERIAL_TX_PIN   PE7    X_SERIAL_RX_PIN   PE7
Y_SERIAL_TX_PIN   PE15   Y_SERIAL_RX_PIN   PE15
Z_SERIAL_TX_PIN   PD10   Z_SERIAL_RX_PIN   PD10
E0_SERIAL_TX_PIN  PD7    E0_SERIAL_RX_PIN  PD7
E1_SERIAL_TX_PIN  PC14   E1_SERIAL_RX_PIN  PC14
E2_SERIAL_TX_PIN  PC15   E2_SERIAL_RX_PIN  PC15
```
Everything else in the table below can stay as Marlin defines it.

## Hardware facts this check surfaced (relevant to this build)
- **E0's DIAG jumper routes the extruder's stall output to PA3 = Z_MAX**, which is also the probe connector J40
  (via 100 Ω + BAT85). Sources: pinout E0-MOT inset `Z+:PA3`; schematic JP4 → Z_MAX. If a bed probe or Z_MAX
  is used, the E0 DIAG jumper must not be fitted. X → PB14 (X_MIN), Y → PB13 (Y_MIN), Z → PA0 (Z_MIN).
- **FAN0 = PA13 and FAN1 = PA14 are also the SWD debug pins.** The schematic symbol lists no timer channel on either,
  so hardware PWM is unlikely. Plan on Marlin `FAN_SOFT_PWM` (check at config time).
- **"FAN3/4/5" are the RGB strip channels:** PB5 = G, PB6 = R, PB7 = B (low-side MOSFETs). Marlin maps them as
  `RGB_LED_G/R/B_PIN`, which matches the hardware.
- **The EEPROM is an AT24C32 (4 KB)** on I2C1, PB8 (SCL) / PB9 (SDA), address 000. Marlin's `I2C_EEPROM` / `MARLIN_EEPROM_SIZE 0x1000` matches.
- **The schematic's jumper block** (per socket, 4 rows × 3 pins): the centre pin goes to the socket pin, one side to a 4.7 kΩ pull-up (VCCIO = 5 V),
  and the other side to the MCU. MS1 ↔ MOSI PE14, MS2 ↔ SCK PE12, MS3 ↔ that socket's CS, RST ↔ MISO PE13.

## Full signal-by-signal table
Evidence for each row: the pinout column comes from the driver insets and callouts of sheet A, the schematic column from the MCU sheet
(page 17) of B, traced to the net label. The Marlin citation is the winning `file:line`.

| Signal | Pinout (A) | Schematic (B) | Marlin macro = value (C) | Marlin file:line | Result |
|---|---|---|---|---|---|
| M0 (X) EN | PE9 | PE9 | `X_ENABLE_PIN` = PE12 | pins_FYSETC_S6.h:29 | ❌ **MISMATCH** |
| M0 (X) STEP | PE11 | PE11 | `X_STEP_PIN` = PE11 | pins_FYSETC_S6_common.h:81 | ✅ match |
| M0 (X) DIR | PE10 | PE10 | `X_DIR_PIN` = PE10 | pins_FYSETC_S6_common.h:82 | ✅ match |
| M0 (X) CS/PDN | PE7 | PE7 | `X_CS_PIN` = PE7 | pins_FYSETC_S6_common.h:84 | ✅ match |
| M0 (X) UART TX (=CS/PDN, single-wire) | PE7 | PE7 | `X_SERIAL_TX_PIN` = PE9 | pins_FYSETC_S6.h:35 | ❌ **MISMATCH** |
| M0 (X) UART RX (=CS/PDN, single-wire) | PE7 | PE7 | `X_SERIAL_RX_PIN` = PE8 | pins_FYSETC_S6.h:36 | ❌ **MISMATCH** |
| M0 (X) DIAG -> endstop | PB14 | PB14 | `—` = — | — | (hardware only) |
| M1 (Y) EN | PD9 | PD9 | `Y_ENABLE_PIN` = PD9 | pins_FYSETC_S6_common.h:88 | ✅ match |
| M1 (Y) STEP | PD8 | PD8 | `Y_STEP_PIN` = PD8 | pins_FYSETC_S6_common.h:86 | ✅ match |
| M1 (Y) DIR | PB12 | PB12 | `Y_DIR_PIN` = PB12 | pins_FYSETC_S6_common.h:87 | ✅ match |
| M1 (Y) CS/PDN | PE15 | PE15 | `Y_CS_PIN` = PE15 | pins_FYSETC_S6_common.h:89 | ✅ match |
| M1 (Y) UART TX (=CS/PDN, single-wire) | PE15 | PE15 | `Y_SERIAL_TX_PIN` = PE14 | pins_FYSETC_S6.h:37 | ❌ **MISMATCH** |
| M1 (Y) UART RX (=CS/PDN, single-wire) | PE15 | PE15 | `Y_SERIAL_RX_PIN` = PE13 | pins_FYSETC_S6.h:38 | ❌ **MISMATCH** |
| M1 (Y) DIAG -> endstop | PB13 | PB13 | `—` = — | — | (hardware only) |
| M2 (Z) EN | PD15 | PD15 | `Z_ENABLE_PIN` = PD15 | pins_FYSETC_S6_common.h:93 | ✅ match |
| M2 (Z) STEP | PD14 | PD14 | `Z_STEP_PIN` = PD14 | pins_FYSETC_S6_common.h:91 | ✅ match |
| M2 (Z) DIR | PD13 | PD13 | `Z_DIR_PIN` = PD13 | pins_FYSETC_S6_common.h:92 | ✅ match |
| M2 (Z) CS/PDN | PD10 | PD10 | `Z_CS_PIN` = PD10 | pins_FYSETC_S6_common.h:94 | ✅ match |
| M2 (Z) UART TX (=CS/PDN, single-wire) | PD10 | PD10 | `Z_SERIAL_TX_PIN` = PD11 | pins_FYSETC_S6.h:39 | ❌ **MISMATCH** |
| M2 (Z) UART RX (=CS/PDN, single-wire) | PD10 | PD10 | `Z_SERIAL_RX_PIN` = PD12 | pins_FYSETC_S6.h:40 | ❌ **MISMATCH** |
| M2 (Z) DIAG -> endstop | PA0 | PA0 | `—` = — | — | (hardware only) |
| M3 (E0) EN | PD4 | PD4 | `E0_ENABLE_PIN` = PD4 | pins_FYSETC_S6_common.h:98 | ✅ match |
| M3 (E0) STEP | PD5 | PD5 | `E0_STEP_PIN` = PD5 | pins_FYSETC_S6_common.h:96 | ✅ match |
| M3 (E0) DIR | PD6 | PD6 | `E0_DIR_PIN` = PD6 | pins_FYSETC_S6_common.h:97 | ✅ match |
| M3 (E0) CS/PDN | PD7 | PD7 | `E0_CS_PIN` = PD7 | pins_FYSETC_S6_common.h:99 | ✅ match |
| M3 (E0) UART TX (=CS/PDN, single-wire) | PD7 | PD7 | `E0_SERIAL_TX_PIN` = PD3 | pins_FYSETC_S6.h:41 | ❌ **MISMATCH** |
| M3 (E0) UART RX (=CS/PDN, single-wire) | PD7 | PD7 | `E0_SERIAL_RX_PIN` = PA15 | pins_FYSETC_S6.h:42 | ❌ **MISMATCH** |
| M3 (E0) DIAG -> endstop | PA3 | PA3 | `—` = — | — | (hardware only) |
| M4 (E1) EN | PE5 | PE5 | `E1_ENABLE_PIN` = PE5 | pins_FYSETC_S6_common.h:103 | ✅ match |
| M4 (E1) STEP | PE6 | PE6 | `E1_STEP_PIN` = PE6 | pins_FYSETC_S6_common.h:101 | ✅ match |
| M4 (E1) DIR | PC13 | PC13 | `E1_DIR_PIN` = PC13 | pins_FYSETC_S6_common.h:102 | ✅ match |
| M4 (E1) CS/PDN | PC14 | PC14 | `E1_CS_PIN` = PC14 | pins_FYSETC_S6_common.h:104 | ✅ match |
| M4 (E1) UART TX (=CS/PDN, single-wire) | PC14 | PC14 | `E1_SERIAL_TX_PIN` = PC4 | pins_FYSETC_S6.h:43 | ❌ **MISMATCH** |
| M4 (E1) UART RX (=CS/PDN, single-wire) | PC14 | PC14 | `E1_SERIAL_RX_PIN` = PC5 | pins_FYSETC_S6.h:44 | ❌ **MISMATCH** |
| M4 (E1) DIAG -> endstop | PA2 | PA2 | `—` = — | — | (hardware only) |
| M5 (E2) EN | PE3 | PE3 | `E2_ENABLE_PIN` = PE3 | pins_FYSETC_S6_common.h:108 | ✅ match |
| M5 (E2) STEP | PE2 | PE2 | `E2_STEP_PIN` = PE2 | pins_FYSETC_S6_common.h:106 | ✅ match |
| M5 (E2) DIR | PE4 | PE4 | `E2_DIR_PIN` = PE4 | pins_FYSETC_S6_common.h:107 | ✅ match |
| M5 (E2) CS/PDN | PC15 | PC15 | `E2_CS_PIN` = PC15 | pins_FYSETC_S6_common.h:109 | ✅ match |
| M5 (E2) UART TX (=CS/PDN, single-wire) | PC15 | PC15 | `E2_SERIAL_TX_PIN` = PE1 | pins_FYSETC_S6.h:45 | ❌ **MISMATCH** |
| M5 (E2) UART RX (=CS/PDN, single-wire) | PC15 | PC15 | `E2_SERIAL_RX_PIN` = PE0 | pins_FYSETC_S6.h:46 | ❌ **MISMATCH** |
| M5 (E2) DIAG -> endstop | PA1 | PA1 | `—` = — | — | (hardware only) |
| M6 (E3) EN | PE8 | PE8 | `X2_ENABLE_PIN` = PE8 | pins_FYSETC_SPIDER.h:49 | ✅ match |
| M6 (E3) STEP | PD12 | PD12 | `X2_STEP_PIN` = PD12 | pins_FYSETC_SPIDER.h:47 | ✅ match |
| M6 (E3) DIR | PC4 | PC4 | `X2_DIR_PIN` = PC4 | pins_FYSETC_SPIDER.h:48 | ✅ match |
| M6 (E3) CS/PDN | PA15 | PA15 | `X2_CS_PIN` = PA15 | pins_FYSETC_SPIDER.h:50 | ✅ match |
| M6 (E3) UART TX (=CS/PDN, single-wire) | PA15 | PA15 | `X2_SERIAL_TX_PIN` = PA15 | pins_FYSETC_SPIDER.h:78 | ✅ match |
| M6 (E3) DIAG -> endstop | NC | NC | `—` = — | — | (hardware only) |
| M7 (E4) EN | PC5 | PC5 | `Z2_ENABLE_PIN` = PC5 | pins_FYSETC_SPIDER.h:54 | ✅ match |
| M7 (E4) STEP | PE1 | PE1 | `Z2_STEP_PIN` = PE1 | pins_FYSETC_SPIDER.h:52 | ✅ match |
| M7 (E4) DIR | PE0 | PE0 | `Z2_DIR_PIN` = PE0 | pins_FYSETC_SPIDER.h:53 | ✅ match |
| M7 (E4) CS/PDN | PD11 | PD11 | `Z2_CS_PIN` = PD11 | pins_FYSETC_SPIDER.h:55 | ✅ match |
| M7 (E4) UART TX (=CS/PDN, single-wire) | PD11 | PD11 | `Z2_SERIAL_TX_PIN` = PD11 | pins_FYSETC_SPIDER.h:81 | ✅ match |
| M7 (E4) DIAG -> endstop | NC | NC | `—` = — | — | (hardware only) |
| X_MIN | PB14 | PB14 | `X_MIN_PIN` = PB14 | pins_FYSETC_S6_common.h:63 | ✅ match |
| Y_MIN | PB13 | PB13 | `Y_MIN_PIN` = PB13 | pins_FYSETC_S6_common.h:65 | ✅ match |
| Z_MIN | PA0 | PA0 | `Z_MIN_PIN` = PA0 | pins_FYSETC_S6_common.h:67 | ✅ match |
| X_MAX | PA1 | PA1 | `X_MAX_PIN` = PA1 | pins_FYSETC_S6_common.h:64 | ✅ match |
| Y_MAX | PA2 | PA2 | `Y_MAX_PIN` = PA2 | pins_FYSETC_S6_common.h:66 | ✅ match |
| Z_MAX | PA3 | PA3 | `Z_MAX_PIN` = PA3 | pins_FYSETC_S6_common.h:68 | ✅ match |
| HEAT0 | PB15 | PB15 | `HEATER_0_PIN` = PB15 | pins_FYSETC_SPIDER.h:60 | ✅ match |
| HEAT1 | PC8 | PC8 | `HEATER_1_PIN` = PC8 | pins_FYSETC_SPIDER.h:61 | ✅ match |
| HEAT2 | PB3 | PB3 | `HEATER_2_PIN` = PB3 | pins_FYSETC_SPIDER.h:62 | ✅ match |
| BED-OUT | PB4 | PB4 | `HEATER_BED_PIN` = PB4 | pins_FYSETC_SPIDER.h:63 | ✅ match |
| T0 | PC0 | PC0 | `TEMP_0_PIN` = PC0 | pins_FYSETC_S6_common.h:114 | ✅ match |
| T1 | PC1 | PC1 | `TEMP_1_PIN` = PC1 | pins_FYSETC_S6_common.h:115 | ✅ match |
| T2 | PC2 | PC2 | `TEMP_2_PIN` = PC2 | pins_FYSETC_S6_common.h:116 | ✅ match |
| T3 | PC3 | PC3 | `TEMP_3_PIN` = PC3 | pins_FYSETC_SPIDER_V2_2.h:27 | ✅ match |
| T4 | PB1 | PB1 | `TEMP_4_PIN` = PB1 | pins_FYSETC_SPIDER_V2_2.h:28 | ✅ match |
| TB | PB0 | PB0 | `TEMP_BED_PIN` = PB0 | pins_FYSETC_SPIDER_V2_2.h:29 | ✅ match |
| FAN0 | PA13 | PA13 | `FAN0_PIN` = PA13 | pins_FYSETC_SPIDER_V2_2.h:31 | ✅ match |
| FAN1 | PA14 | PA14 | `FAN1_PIN` = PA14 | pins_FYSETC_SPIDER_V2_2.h:32 | ✅ match |
| FAN2 | PB2 | PB2 | `FAN2_PIN` = PB2 | pins_FYSETC_S6_common.h:143 | ✅ match |
| Motor SPI MOSI | PE14 | PE14 | `TMC_SPI_MOSI` = PE14 | pins_FYSETC_SPIDER.h:92 | ✅ match |
| Motor SPI MISO | PE13 | PE13 | `TMC_SPI_MISO` = PE13 | pins_FYSETC_SPIDER.h:95 | ✅ match |
| Motor SPI SCK | PE12 | PE12 | `TMC_SPI_SCK` = PE12 | pins_FYSETC_SPIDER.h:98 | ✅ match |
| EXP1 BEEP | PC9 (listed) | PC9 | `EXP1_01_PIN` = PC9 | pins_FYSETC_S6_common.h:162 | ✅ match |
| EXP1 BTN_ENC | PA8 (listed) | PA8 | `EXP1_02_PIN` = PA8 | pins_FYSETC_S6_common.h:163 | ✅ match |
| EXP1 LCD_EN | PC11 (listed) | PC11 | `EXP1_03_PIN` = PC11 | pins_FYSETC_S6_common.h:164 | ✅ match |
| EXP1 LCD_RS | PD2 (listed) | PD2 | `EXP1_04_PIN` = PD2 | pins_FYSETC_S6_common.h:165 | ✅ match |
| EXP1 LCD_D4 | PC10 (listed) | PC10 | `EXP1_05_PIN` = PC10 | pins_FYSETC_S6_common.h:166 | ✅ match |
| EXP1 LCD_D5 | PC12 (listed) | PC12 | `EXP1_06_PIN` = PC12 | pins_FYSETC_S6_common.h:167 | ✅ match |
| EXP1 LCD_D6 | PD0 (listed) | PD0 | `EXP1_07_PIN` = PD0 | pins_FYSETC_S6_common.h:168 | ✅ match |
| EXP1 LCD_D7 | PD1 (listed) | PD1 | `EXP1_08_PIN` = PD1 | pins_FYSETC_S6_common.h:169 | ✅ match |
| EXP2 MISO | PA6 (listed) | PA6 | `EXP2_01_PIN` = PA6 | pins_FYSETC_S6_common.h:171 | ✅ match |
| EXP2 SCK | PA5 (listed) | PA5 | `EXP2_02_PIN` = PA5 | pins_FYSETC_S6_common.h:172 | ✅ match |
| EXP2 BTN_EN1 | PC6 (listed) | PC6 | `EXP2_03_PIN` = PC6 | pins_FYSETC_S6_common.h:173 | ✅ match |
| EXP2 SS | PA4 (listed) | PA4 | `EXP2_04_PIN` = PA4 | pins_FYSETC_S6_common.h:174 | ✅ match |
| EXP2 BTN_EN2 | PC7 (listed) | PC7 | `EXP2_05_PIN` = PC7 | pins_FYSETC_S6_common.h:175 | ✅ match |
| EXP2 MOSI | PA7 (listed) | PA7 | `EXP2_06_PIN` = PA7 | pins_FYSETC_S6_common.h:176 | ✅ match |
| EXP2 CD | PB10 (listed) | PB10 | `EXP2_07_PIN` = PB10 | pins_FYSETC_S6_common.h:177 | ✅ match |
| EXP2 RESET | — | RESET (MCU NRST via  | `EXP2_08_PIN` = -1 | pins_FYSETC_S6_common.h:178 | n/a (reset line) |
| SD SCK | PA5 | — | `SD_SCK_PIN` = PA5 | pins_FYSETC_S6_common.h:183 | ✅ match |
| SD MISO | PA6 | — | `SD_MISO_PIN` = PA6 | pins_FYSETC_S6_common.h:184 | ✅ match |
| SD MOSI | PA7 | — | `SD_MOSI_PIN` = PA7 | pins_FYSETC_S6_common.h:185 | ✅ match |
| SD CS | PA4 | — | `SD_SS_PIN` = PA4 | pins_FYSETC_S6_common.h:187 | ✅ match |
| SD detect | PB10 | — | `SD_DETECT_PIN` = PB10 | pins_FYSETC_S6_common.h:188 | ✅ match |
| Neopixel DATA | PD3 | PD3 | `—` = — | — | (hardware only) |
| EEPROM SCL | — | PB8 | `—` = — | — | (hardware only) |
| EEPROM SDA | — | PB9 | `—` = — | — | (hardware only) |

## Limits of this check
- Marlin's `pins_postprocess.h` was stubbed in the preprocessor run. The values are the state at the end of the board pin files.
- The schematic doesn't say which physical driver sheet (MD1–MD8) is which axis. The MCU pins per axis come from the MCU sheet and match the pinout.
- This is a desk check against FYSETC's documents. The first power-on test (M122 driver report) is the hardware confirmation.

## Evidence files
- `evidence/marlin/`: preprocessor test files, raw `-dM`/`-dD` output, the GCC redefinition warnings, and the resolved map with file:line
- `evidence/pinout_extract.json`, `evidence/schematic_extract.json`, `evidence/compare_rows.json`: the full machine-readable extractions and the comparison
