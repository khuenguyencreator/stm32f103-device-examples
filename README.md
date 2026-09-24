# STM32F103 Device Examples

Runnable example projects for the drivers in
[**stm32-device-drivers**](https://github.com/khuenguyencreator/stm32-device-drivers),
one project per device (Keil MDK-ARM, a few in STM32CubeIDE). The driver sources are **not** copied in —
they are pulled from the library repo as a git submodule at `lib/drivers/` and
compiled straight from there.

- MCU: **STM32F103xB** (Blue Pill / F103C8T6 class) on every example.
- Toolchain: **Keil MDK-ARM (µVision 5)**, ARM Compiler 5 or 6.
  `LCD_TFT_ST7789`, `LED7_6PIN` and `RFID_RC522_SIMPLE` are **STM32CubeIDE**
  projects instead (import via *File → Import → Existing Projects into Workspace*).
- Flash/debug: ST-Link.
- The `.ioc` file in each project is kept for pin reference only; the build is
  driven by Keil, not STM32CubeIDE.

## Getting the code

```bash
git clone --recurse-submodules https://github.com/khuenguyencreator/stm32f103-device-examples.git
# already cloned without --recurse-submodules:
git submodule update --init --recursive
```

## Repo layout

```
stm32f103-device-examples/
├── lib/drivers/        git submodule -> stm32-device-drivers
├── <EXAMPLE>/
│   ├── MDK-ARM/<name>.uvprojx   Keil project (open this)
│   ├── Src/  or  Core/Src/      main.c + HAL glue
│   ├── Inc/  or  Core/Inc/      main.h (pin defines)
│   └── Drivers/                 vendored CMSIS + STM32F1 HAL
└── ...
```

Two folder layouts exist: most projects use `Src/` + `Inc/`; `LED7SEG`,
`RFID_RC522` and the STM32CubeIDE projects use `Core/Src/` + `Core/Inc/`. That only changes the relative
include depth (`../../lib/...` vs `../../../lib/...`).

## Examples

| Project | Peripheral | Driver (`lib/drivers/…`) | Notes |
|---|---|---|---|
| `BUTTON` | GPIO in | `input/button/button.h` | debounced button read |
| `CLCD_4BIT` | GPIO | `display/char_lcd/char_lcd.h` | HD44780 character LCD, 4-bit bus |
| `CLCD_8BIT` | GPIO | `display/char_lcd/char_lcd.h` | HD44780, 8-bit bus |
| `CLCD_I2C` | I2C | `display/char_lcd_i2c/char_lcd_i2c.h` | HD44780 via PCF8574 backpack |
| `DFPLAYER` | UART | `audio/dfplayer/dfplayer.h` | DFPlayer Mini MP3 module |
| `DHT11` | GPIO + TIM4_CH1 (input capture) | `sensor/dht/dht.h` | **DATA wired to PB6** (see below) |
| `DS18B20` | GPIO (DWT µs delay) | `sensor/ds18b20/ds18b20.h` | 1-Wire temperature sensor |
| `DS3231` | I2C | `rtc/ds3231/ds3231.h` | RTC |
| `JOYSTICK` | ADC | `input/joystick/joystick.h` | 2-axis analog joystick + button |
| `KEYPAD` | GPIO | `input/keypad/keypad.h` | 3×4 matrix keypad |
| `LCD_OLED_SSD1306` | I2C | `display/ssd1306/ssd1306.h` | 128×64 OLED |
| `LCD_TFT_ST7735` | SPI | `display/st7735/st7735.h` | 160×128 TFT |
| `LCD_TFT_ST7789` | SPI | `display/st7789/st7789.h` | IPS TFT — STM32CubeIDE project |
| `LED7SEG` | GPIO | `display/led7seg/led7seg.h` | multiplexed 7-segment (`Core/Src` layout) |
| `LED7_6PIN` | GPIO | `display/led7_6pin/led7_6pin.h` | 7-segment, 6-pin module — STM32CubeIDE project |
| `RFID_RC522` | SPI | `rfid/rc522/rc522.h` (API `MFRC522_*`) | MIFARE reader (`Core/Src` layout) |
| `RFID_RC522_SIMPLE` | SPI | `rfid/rc522_simple/rc522_simple.h` | simplified RC522 read — STM32CubeIDE project |
| `SRF05` | GPIO + TIM1_CH2 (input capture) | `sensor/srf05/srf05.h` | ultrasonic range finder |
| `SERVO` | TIM PWM | *local (in `main.c`)* | no shared driver yet |
| `TFT_LCD_ILI9341` | SPI | *local (`lcd.h`)* | no shared driver yet |

## Build & flash one example

1. Open `<EXAMPLE>/MDK-ARM/<name>.uvprojx` in Keil µVision.
2. Project → Build (F7). Driver `.c` files are listed under the
   **Application/User** group and compiled from `..\..\lib\drivers\…`.
3. Flash → Download (F8) with an ST-Link connected.

If a driver header is not found, check the project's
*Options → C/C++ → Include Paths* contains `..\..\lib\drivers\<category>\<device>`.

## Updating the drivers

```bash
cd lib/drivers
git pull origin master
cd ../..
git add lib/drivers
git commit -m "chore: bump drivers"
```

## Notes

- **DHT11 wiring** — the DHT driver needs its DATA pin on a timer channel for
  input capture. PB14 (the original pin) has none on F103, so this example uses
  **PB6 = TIM4_CH1**; move the sensor's DATA wire to PB6.
- **DHT / SRF05** need a timer configured in Input Capture mode, 1 µs/tick
  (Prescaler = `TIMxCLK/1_000_000 − 1`, Period `0xFFFF`); `MX_TIMx_Init()` in
  those `main.c` files already does this.
- **DS18B20** uses the Cortex-M `DWT` cycle counter for its µs delays — no timer,
  but needs a correct `SystemCoreClock`.
- `SERVO` and `TFT_LCD_ILI9341` still carry their driver logic locally; they are
  not yet part of `stm32-device-drivers`.
- Build output (`*.o`, `*.axf`, `*.hex`, `*.map`, …) is git-ignored — a fresh
  clone has only sources until you build.

## Links

- 📖 Tutorials (Vietnamese): [khuenguyencreator.com](https://khuenguyencreator.com)
- 📚 More repos: [github.com/khuenguyencreator](https://github.com/khuenguyencreator)
