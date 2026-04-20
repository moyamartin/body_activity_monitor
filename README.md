# Body Activity Monitor

Firmware for an STM32F446RE-based wearable activity monitor. The device reads
motion data from a Bosch BMA400 3-axis accelerometer, classifies the user's
activity (still / walking / running), counts steps, and detects free-fall
events that can escalate into an emergency state. Status is shown on a
16x2 character LCD and mirrored over the on-board ST-Link USB virtual COM
port.

## How it works

The application is organized as a finite state machine driven from
`main.c`'s super-loop. Two modules cooperate:

- `activity_monitor` (`Core/Src/activity_monitor.c`) owns the FSM and
  orchestrates the UART, display and IMU.
- `debounce` (`Drivers/API/Src/API_debounce.c`) provides a software-debounced
  reading of the user button via a 40 ms non-blocking delay.

### State machine

| State            | Entry condition                                                             | Exit condition                                                                                 |
| ---------------- | --------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------- |
| `STILL`          | Start-up or BMA400 reports no activity                                      | BMA400 reports walking/running; user button clears the step counter                            |
| `WALKING`        | BMA400 step-counter interrupt classifies activity as walking                | BMA400 reclassifies activity, or Gen1 interrupt fires (free fall)                              |
| `RUNNING`        | BMA400 step-counter interrupt classifies activity as running                | BMA400 reclassifies activity, or Gen1 interrupt fires (free fall)                              |
| `FREE_FALL`      | Gen1 interrupt: abs(X), abs(Y), abs(Z) < 504 mg for 200 ms (acc_filt2 @ 100 Hz)      | User presses the button (cancel), or 30 s elapse without a press (escalate to `EMERGENCY`)     |
| `EMERGENCY`      | `FREE_FALL` timeout reached                                                 | User presses the button at least 5 times                                                       |
| `ACTIVITY_ERROR` | UART/IMU failure during normal operation                                    | Sticky state; requires reset                                                                   |

The free-fall threshold (504 mg for ~200 ms of near-zero acceleration) is
chosen to filter out normal walking/running bounces while still catching a
fall from roughly 1.3 m; the full derivation is in the comment at the top of
`activity_monitor_init()`.

### User outputs

- **LCD1602** (16x2): row 0 shows the current state name, row 1 shows
  `Steps: <n>`.
- **UART** (115200 8N1, no flow control) on the Nucleo's ST-Link virtual COM:
  prints startup info, IMU diagnostics, and a one-line status update on every
  state/step change.

### User inputs

- **B1 (blue user button, PC13)**:
  - in `STILL`: resets the step counter to 0;
  - in `FREE_FALL`: cancels the fall alert and returns to `STILL`;
  - in `EMERGENCY`: must be pressed at least 5 times to recover.

## Hardware

Target board: **NUCLEO-F446RE** (STM32F446RET6, Cortex-M4F @ 180 MHz).

| Peripheral        | Pins                  | Purpose                                             |
| ----------------- | --------------------- | --------------------------------------------------- |
| USART2            | PA2 (TX), PA3 (RX)    | Serial console, routed through the ST-Link VCP      |
| I2C1              | PB8 (SCL), PB9 (SDA)  | Shared bus for BMA400 and LCD1602                   |
| GPIO (input)      | PC13                  | Onboard user button B1, EXTI13 falling edge         |
| TIM1              | (internal)            | Microsecond time base used by the BMA400 driver     |

External components (wired to the Nucleo's Arduino/Morpho headers):

- **Bosch BMA400** accelerometer breakout, I2C address `0x15`
  (SDO pulled low). SDA -> PB9, SCL -> PB8, VDD -> 3V3, GND -> GND. No
  INT line is required — the driver polls the status register from the
  main loop.
- **LCD1602** character LCD with a PCF8574 I2C backpack, default address
  `0x27`. SDA -> PB9, SCL -> PB8, VCC -> 5V, GND -> GND.
- Both devices share the same I2C1 bus.

The CubeMX configuration lives in `body_activity_monitor.ioc`; regenerating
from there will keep the pin map in sync.

## Building

The project uses CMake with the `arm-none-eabi-gcc` toolchain and Ninja.

### Prerequisites

- `arm-none-eabi-gcc` 10.3 or newer (tested with the Arm GNU Toolchain)
- `cmake` >= 3.22
- `ninja`
- `stlink-tools` or `STM32CubeProgrammer` for flashing (see below)

Make sure `arm-none-eabi-gcc` is on your `PATH`:

```sh
arm-none-eabi-gcc --version
```

### Configure and build

Two CMake presets are defined in `CMakePresets.json`:

```sh
# Debug build (default; -O0 -g3)
cmake --preset Debug
cd build/Debug && ninja

# Release build (-Os)
cmake --preset Release
cd build/Release && ninja
```

The resulting ELF is produced at
`build/<preset>/body_activity_monitor.elf`. The linker also emits a map file
and prints the flash/RAM usage summary at the end of the build.

## Flashing the NUCLEO-F446RE

The Nucleo board exposes its STM32 via an on-board ST-LINK/V2-1 over USB.
Any of the following works; pick the one that matches your toolchain.

### Option 1 - stlink-tools (`st-util` and `gdb-multiarch`)

in one terminal:
```sh
$ st-util
st-util 1.8.0
2026-04-20T19:41:06 INFO common.c: STM32F446: 128 KiB SRAM, 512 KiB flash in at least 128 KiB pages.
2026-04-20T19:41:06 INFO gdb-server.c: Listening at *:4242...
```

on another terminal:

```sh
$ gdb-multiarch -f <path_to_elf_file>
(gdb) target extended localhost:4242
(gdb) load

...

(gdb) run
```

### Option 2 - STM32CubeProgrammer CLI

```sh
STM32_Programmer_CLI -c port=SWD -w build/Debug/body_activity_monitor.elf -rst
```
## Observing the output

With the Nucleo connected, open the ST-Link virtual COM port at
**115200 8N1**:

```sh
# On Linux the VCP typically enumerates as /dev/ttyACM0
minicom -b 115200 -D /dev/ttyACM0
```

You should see the UART banner from `uart_init()`, the activity monitor's
initialization message, and live updates as the device transitions between
states.

## Project layout

```
Core/          Application code (FSM, CubeMX-generated main, IRQs)
Drivers/API/   HAL-agnostic helpers: UART wrapper, delay, debounce, I2C port
Drivers/BMA400 Bosch BMA400 accelerometer driver
Drivers/LCD1602 I2C LCD1602 driver
Drivers/Utils  Small utility macros (bit ops, ARRAY_SIZE, ...)
cmake/         Toolchain file and CubeMX-generated CMake subproject
```