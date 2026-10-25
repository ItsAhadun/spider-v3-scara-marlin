# Marlin for a SCARA printer on the FYSETC Spider V3

Two Marlin patches for a SCARA 3D printer built on the [HALO robotic arm](https://github.com/koaldesigns/HALO) and driven by a FYSETC Spider V3.0 board (STM32F446VE).

> **Status: untested on hardware.** The firmware compiles. We have not flashed it or moved the arm with it yet. Expect wrong motor directions and home angles, and read [Values to measure](#values-to-measure) before you power the motors.

## Patches

| Patch | Contents |
|---|---|
| [`0001-fysetc-spider-v3-board.patch`](patches/0001-fysetc-spider-v3-board.patch) | Adds `BOARD_FYSETC_SPIDER_V3` (ID 5264) and a pin file for the F446 Spider V3. Works for any printer on this board. |
| [`0002-halo-scara-config.patch`](patches/0002-halo-scara-config.patch) | Configuration.h, Configuration_adv.h and platformio.ini for the HALO arm: SCARA kinematics, 4× TMC2209 over UART, sensorless homing. Needs 0001. |

You can read the pin file without applying anything: [`Marlin/src/pins/stm32f4/pins_FYSETC_SPIDER_V3.h`](Marlin/src/pins/stm32f4/pins_FYSETC_SPIDER_V3.h).

## Why the Spider V3 needs its own pin file

The closest board Marlin ships is `BOARD_FYSETC_SPIDER_V2_2`. On a V3 that file sets `X_ENABLE_PIN` to PE12 instead of PE9 and puts every TMC2209 UART line on the wrong pin. With UART drivers, X traffic would toggle the X enable pin. [docs/pin-verification.md](docs/pin-verification.md) compares the V3 schematic, the FYSETC pinout sheet and Marlin's V2.2 file signal by signal.

Marlin added the Spider V3 **H7** (STM32H723) in October 2026. That board has a different MCU and pin map, so `BOARD_FYSETC_SPIDER_V3_H723` will not run on an F446 Spider V3.

The pin file here includes `env_validate.h` and nothing else. Each TMC2209 talks single-wire UART at 19200 baud, with TX and RX on its socket's CS pin.

## Build

You need git and VS Code with the PlatformIO IDE extension.

```sh
git clone https://github.com/MarlinFirmware/Marlin.git
git clone https://github.com/ItsAhadun/spider-v3-scara-marlin.git
cd Marlin
git checkout 2ef3ee62fd46bd0c5d4304732ef8e255e2bb9869
git apply ../spider-v3-scara-marlin/patches/0001-fysetc-spider-v3-board.patch
git apply ../spider-v3-scara-marlin/patches/0002-halo-scara-config.patch
```

Open the `Marlin` folder in VS Code and build env `STM32F446VE_fysetc_32k_bootloader`. PlatformIO writes `.pio/build/STM32F446VE_fysetc_32k_bootloader/firmware.bin`.

On Windows, clone into a short path such as `C:\SCARA\Marlin`. A deep folder path pushes object files past the 260-character limit and the link step fails.

For a different printer on this board, apply 0001 alone, then set `MOTHERBOARD BOARD_FYSETC_SPIDER_V3`, `SERIAL_PORT -1` and `FAN_SOFT_PWM` in Configuration.h. FAN0 (PA13) and FAN1 (PA14) have no hardware timer, and Marlin stops the build without `FAN_SOFT_PWM`.

## Flash

The Spider ships with a 32 KB bootloader, and this env links the firmware at 0x08008000 to sit after it.

1. Format a microSD card of 32 GB or less as FAT32.
2. Copy `firmware.bin` to the root of the card.
3. With the board off, insert the card. Power the board and wait about 30 seconds while the LED beside the slot blinks.
4. Put the card back in your PC. The bootloader renames the file to `old.bin` when the flash succeeds.

Connect over USB and send `M122`. All four TMC2209s should answer with their register dump.

## Board setup

- **UART:** one jumper per driver socket, on positions 3 and 11 of the 3×4 block under the driver. It connects the driver's PDN_UART pin to the MCU.
- **Sensorless homing:** DIAG jumpers on X, Y and Z route DIAG to PB14, PB13 and PA0. Leave E0 without one, because its DIAG lands on PA3, the Z-MAX/probe input.
- **5V jumper:** USB5V while you flash from USB power, DC5V once the 24 V supply is connected.

## HALO kinematics

This Marlin version has a single `SCARA` mode (`MP_SCARA` and `MORGAN_SCARA` now stop the build with an error). `SCARA_CROSSTALK_FACTOR` tells Marlin how shoulder motion shifts the elbow angle. HALO drives both joints from motors in the base, and its elbow belt runs from 169 teeth on the shoulder axis to 122 teeth at the elbow, giving a factor of 169/122. [docs/kinematics.md](docs/kinematics.md) has the derivation and the STL measurements.

| Setting | Value | Source |
|---|---|---|
| `SCARA_LINKAGE_1` | 255.0 mm | ProximalArm.stl, bore to bore |
| `SCARA_CROSSTALK_FACTOR` | 169.0/122.0 | DistalDriveGear and DistalConnectorGear tooth counts |
| A steps/° (shoulder) | 271.52 | 16T→128T belt, 11T→42T gear, 1.8° motor, 16 microsteps |
| B steps/° (elbow) | 196.01 | 271.52 × 122/169 |
| `MIDDLE_DEAD_ZONE_R` | 91 mm | L1 − L2, the inner reach limit |

## Values to measure

We guessed these. Measure them on your arm and edit Configuration.h.

| Setting | Placeholder | Measure |
|---|---|---|
| `SCARA_LINKAGE_2` | 164.1 mm | Elbow axis to nozzle. 164.1 mm is the distance to HALO's tool-mount holes. |
| Z steps/mm | 400 | Assumes a T8 screw with 8 mm lead. A 2 mm lead needs 1600. |
| `SCARA_OFFSET_X` / `_Y` | 100 / −120 mm | Shoulder axis position in bed coordinates. |
| `SCARA_HOME_THETA` / `_PSI` | −40° / 160° | Joint angles at the homing stops. Send `M211 S0`, then calibrate with M360 to M364. |
| E steps/mm | 500 | Your extruder. |
| Motor current | 800 mA | Your motors' rated current. |
| Stall sensitivity | 8 | Tune per axis with `M914`. |

Before the first `G28`:

- Set `X_HOME_DIR` and `Y_HOME_DIR` to the side where each joint meets its stop, and make the home angles and `SCARA_ELBOW_DIR` agree with them. The shipped `Y_HOME_DIR -1` and `SCARA_HOME_PSI 160` contradict each other.
- Marlin switches the elbow motor off while the shoulder homes, so the distal arm swings free. Make sure your wiring can take that swing.
- Sensorless Z homes down toward the bed, and StallGuard misses stalls at leadscrew speeds. A Z-max hard stop or a cheap switch is safer.

## Target hardware

| Part | Choice |
|---|---|
| Board | FYSETC Spider V3.0, STM32F446VE, 32 KB bootloader |
| Drivers | 4× TMC2209 in UART mode, sockets X, Y, Z, E0 |
| Motors | NEMA 17, 1.8° |
| Arm | HALO, both joints driven from the base at 30.545:1 |
| Power | 24 V |
| Display | none, Marlin talks to a USB host |

## Contributing

Open an issue with your `M122` output, a measured value, or a homing result. A pull request that replaces a placeholder should say how you measured the new value.

We plan to submit patch 0001 to Marlin as a new board once `M122` confirms the UART pins on real hardware.

## Credits and license

- HALO arm by [koaldesigns](https://github.com/koaldesigns/HALO), licensed CC BY-NC-SA 4.0. This repo contains no HALO files, only measurements taken from them.
- [Marlin](https://github.com/MarlinFirmware/Marlin) by the Marlin team.
- Patches by Abdul Ahad.

GPL-3.0, the same license as Marlin. See [LICENSE](LICENSE).
