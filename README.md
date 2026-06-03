# DFRobot_IRPositionV2

* [中文版本](./README_CN.md)

**DFRobot_IRPositionV2** is an Arduino host library for the IR positioning module (**SKU: SEN0701**) based on **PAJ7025R2**, accessed through a **CS32 I2C bridge** firmware. The host reads a **16 × 16-byte** object frame per update and can configure thresholds, scaling, exposure, and max tracked targets.

## Table of Contents

* [Description](#description)
* [Configuration parameter ranges](#configuration-parameter-ranges)
* [Installation](#installation)
* [Examples](#examples)
* [Methods](#methods)
* [Compatibility](#compatibility)
* [History](#history)
* [Credits](#credits)

## Description

* Track up to **16** infrared spots per frame (index **0–15**).
* Communicates over **I2C** with 7-bit slave address **`0x58`**. The library initializes **`Wire`** at **400 kHz** in `begin()`.
* Outputs per object include **center (12-bit scaled coordinates)**, **boundary** on the native **98×98** grid, **average / max brightness**, and **radius** (low 4 bits of the packed radius field).
* Call **`update()`** once per loop to refresh the internal frame buffer before reading any object slot.

## Configuration parameter ranges

Use these ranges when tuning parameters in your sketch or product integration:

| Item | Range / note |
| ---- | -------------- |
| **I2C address** | 7-bit `0x58`; pass a 7-bit address to `DFRobot_IRPositionV2(TwoWire*, uint8_t)`. |
| **`setI2CAddress(addr7bit, settleMs)`** | `addr7bit`: **0x08–0x77**. Saves to bridge flash and takes effect after reset or power cycle; returns `false` if the address is invalid, communication fails, flash save fails, or the operation times out. |
| **`setIRBrightnessThreshold(brightness)`** | `brightness`: **0–255** (typical default in firmware/docs **0x97**). Higher → stricter brightness gate. |
| **`setIRDetectionNoise(noise)`** | `noise`: **0–255** (typical default **0x0A**). Adjusts noise rejection. |
| **`setScaleResolution(x, y)`** | Each axis **0–4095** (values above 4095 are clamped). Maps native sensor space to scaled 12-bit outputs; common example **2490×2490** for ~30× scale from **98×98**. |
| **`setMaxTrackingNumber(num)`** | `num`: **1–16** (clamped by the library). |
| **`setExposure(expo, settleMs)`** | `expo`: **100–65535** (values below 100 are clamped by the library; effective maximum depends on frame period); `settleMs`: post-commit delay in **ms** (default **10**) before relying on readback. |
| **`getIRCoordinate(IRnum)`** | `IRnum`: **0–15**. Returns packed **12-bit X/Y** in **0–4095** after scaling (see `setScaleResolution`). |
| **`getIRBound(IRnum)`** | `IRnum`: **0–15**. Returns **Up, Down, Left, Right** in **0–98** (native sensor cells). |
| **`getAverageBrightness` / `getMaxBrightness`** | Return **0–255**. |
| **`getRadius(IRnum)`** | Radius uses **4 bits** (typical interpretation **0–15** pixels). |

Exact factory defaults and optical behavior may vary; always validate on your hardware.

## Installation

1. Download the library into your `\Arduino\libraries` folder (or install from the Library Manager when published), then open the `examples` folder and run the sketches.
2. This library depends only on **Arduino** core APIs and **`Wire`**.

## Examples

* `getPosition`: read IR object positions and metrics.
* `setI2CAddress`: save a new 7-bit bridge address to flash; reset or power-cycle after success.

## Methods

```c++
  /**
   * @fn DFRobot_IRPositionV2
   * @brief Default constructor (uses `Wire` and 7-bit address `IRPOS_V2_I2C_ADDR`).
   */
  DFRobot_IRPositionV2();

  /**
   * @fn DFRobot_IRPositionV2
   * @brief Construct with a custom I2C bus and 7-bit address.
   * @param pWire Pointer to `TwoWire` (NULL uses `Wire`).
   * @param addr7bit 7-bit I2C address.
   */
  DFRobot_IRPositionV2(TwoWire *pWire, uint8_t addr7bit);

  /**
   * @fn begin
   * @brief Initialize I2C (400 kHz) and verify communication.
   * @return `true` if probe read succeeds, `false` on I2C error.
   */
  bool begin();

  /**
   * @fn update
   * @brief Read one full frame (16 objects × 16 bytes) into an internal buffer.
   * @return `true` on success, `false` on I2C error.
   */
  bool update();

  /**
   * @fn setIRBrightnessThreshold
   * @brief Set IR spot brightness threshold (typical default 0x97).
   * @param brightness 0–255.
   * @return `true` on success, `false` on I2C error.
   */
  bool setIRBrightnessThreshold(uint8_t brightness);

  /**
   * @fn setIRDetectionNoise
   * @brief Set IR detection noise threshold (typical default 0x0A).
   * @param noise 0–255.
   * @return `true` on success, `false` on I2C error.
   */
  bool setIRDetectionNoise(uint8_t noise);

  /**
   * @fn setScaleResolution
   * @brief Set X/Y scaling (native grid 98×98; each axis clamped to 4095).
   * @param x_axis 0–4095.
   * @param y_axis 0–4095.
   * @return `true` on success, `false` on I2C error.
   */
  bool setScaleResolution(uint16_t x_axis, uint16_t y_axis);

  /**
   * @fn setMaxTrackingNumber
   * @brief Set maximum number of IR spots to track (default often 0x10 = 16).
   * @param num 1–16.
   * @return `true` on success, `false` on I2C error.
   */
  bool setMaxTrackingNumber(uint8_t num);

  /**
   * @fn setExposure
   * @brief Write 16-bit exposure, commit, and optional delay before readback.
   * @param expo 100–65535. Values below 100 are clamped by the library; effective maximum depends on frame period.
   * @param settleMs Delay in ms after commit (default 10).
   * @return `true` on success, `false` on I2C error.
   */
  bool setExposure(uint16_t expo, uint16_t settleMs = 10);

  /**
   * @fn getExposure
   * @brief Read current exposure from readback registers (16-bit).
   * @param expo Output reference.
   * @return `true` on success, `false` on I2C error.
   */
  bool getExposure(uint16_t &expo);

  /**
   * @fn setI2CAddress
   * @brief Save a new 7-bit I2C address to bridge flash; it takes effect after reset or power cycle.
   * @param addr7bit New 7-bit address, valid range 0x08–0x77.
   * @param settleMs Maximum wait in ms for the save operation to finish (default 100).
   * @return `true` if the address is saved, `false` on invalid address, I2C error, flash failure, or timeout.
   */
  bool setI2CAddress(uint8_t addr7bit, uint16_t settleMs = 100);

  /**
   * @fn getI2CAddress
   * @brief Read current active 7-bit I2C address.
   * @param addr7bit Output reference.
   * @return `true` on success, `false` on I2C error.
   */
  bool getI2CAddress(uint8_t &addr7bit);

  /**
   * @fn getIRCoordinate
   * @brief Center of the IR spot from the last `update()` frame.
   * @param IRnum 0–15.
   * @return Pointer to `[xL, xH(4 LSB), yL, yH(4 LSB)]` (12-bit coordinates).
   */
  uint8_t *getIRCoordinate(int IRnum);

  /**
   * @fn getIRBound
   * @brief Boundary box on the native 0–98 grid.
   * @param IRnum 0–15.
   * @return Pointer to `[Up, Down, Left, Right]`.
   */
  uint8_t *getIRBound(int IRnum);

  /**
   * @fn getAverageBrightness
   * @param IRnum 0–15.
   * @return Average brightness (0–255), or 0 if index invalid.
   */
  uint8_t getAverageBrightness(int IRnum);

  /**
   * @fn getMaxBrightness
   * @param IRnum 0–15.
   * @return Max brightness (0–255), or 0 if index invalid.
   */
  uint8_t getMaxBrightness(int IRnum);

  /**
   * @fn getRadius
   * @param IRnum 0–15.
   * @return Radius (low 4 bits of packed field), or 0 if index invalid.
   */
  uint8_t getRadius(int IRnum);
```

## Compatibility

| MCU                | Work Well | Work Wrong | Untested | Remarks |
| ------------------ | :-------: | :--------: | :------: | ------- |
| Arduino Uno        |     √     |            |          |         |
| Arduino Leonardo   |     √     |            |          |         |
| Arduino MEGA2560   |     √     |            |          |         |
| FireBeetle-ESP32   |     √     |            |          |         |
| ESP8266            |     √     |            |          |         |
| FireBeetle-M0      |     √     |            |          |         |
| Micro:bit          |     √     |            |          |         |
| Raspberry Pi       |     √     |            |          |         |

## History

* Date 2026-04-13
* Version V1.0.0

## Credits

Written by thdyyl(yuanlong.yu@dfrobot.com), 2026-04-13 (Welcome to our [website](https://www.dfrobot.com/))
