# DFRobot_IRPositionV2

* [中文版本](./README_CN.md)

This IR positioning module (SKU: SEN0701) uses a PixArt PAJ7025R2 infrared image sensor with an onboard CS32 I2C bridge to detect multiple IR spots in real time and report their positions. The module can track up to 16 infrared spots per frame and outputs the center coordinates, bounding box, average brightness, maximum brightness and approximate radius of each spot. The sensor native resolution is about 98×98; the bridge can scale the X and Y axes independently into a 12-bit output range (0–4095). The host talks to the module over I2C at the default 7-bit address 0x58, and a new address can be saved to Flash (valid range 0x08–0x77, applied after reset or power cycle). This library drives the module from Arduino and similar hosts: it reads spot coordinates and brightness, and sets the brightness detection threshold, noise threshold, exposure, maximum tracking count, indicator LED and I2C address. Example `getPosition` prints spot data over serial; `setI2CAddress` saves a new address. The Raspberry Pi Python version also includes terminal and desktop window display examples.

Wiring (I2C):

- Red → VCC (3.3V or 5V, per the module rating)
- Black → GND
- Green → SCL
- Yellow → SDA

## Product Link (https://www.dfrobot.com)

SKU: SEN0701

## Table of Contents

* [Summary](#summary)
* [Installation](#installation)
* [Methods](#methods)
* [Compatibility](#compatibility)
* [History](#history)
* [Credits](#credits)

## Summary

This library reads the infrared spots and sets the detection threshold, coordinate scale, exposure, maximum tracking count, indicator LED and I2C address. Call `begin()`, then `update()` once per loop before reading each spot. Example `getPosition` prints coordinates and brightness. Example `setI2CAddress` saves a new 7-bit address.

## Installation

There are two methods:

1. Search for DFRobot_IRPositionV2 in the Arduino Library Manager and install it.
2. Download the library, extract it into the Arduino `libraries` directory, then open the `examples` folder and run a sketch.

This library depends only on the Arduino core and `Wire`.

A Raspberry Pi / Python version is in [`python/raspberry`](./python/raspberry/README.md). It depends on `smbus2` and exposes the same API in snake_case (`get_position.py`, `set_i2c_address.py`).

## Methods

```c++
  /**
   * @fn DFRobot_IRPositionV2
   * @brief Default constructor (uses `Wire` and 7-bit address 0x58).
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
   * @return `true` if the probe read succeeds, `false` on I2C error.
   */
  bool begin();

  /**
   * @fn update
   * @brief Read one full frame (16 objects × 16 bytes) into an internal buffer.
   * @n Call once per loop before getIRCoordinate / getIRBound / brightness / radius accessors.
   * @return `true` on success, `false` on I2C error.
   */
  bool update();

  /**
   * @fn setIRBrightnessThreshold
   * @brief Set the IR spot brightness threshold.
   * @param brightness Threshold value.
   * @n Range 0–255; typical factory default 0x97.
   * @n A higher value is a stricter brightness gate, and dimmer spots are dropped.
   * @return `true` on success, `false` on I2C error.
   */
  bool setIRBrightnessThreshold(uint8_t brightness);

  /**
   * @fn setIRDetectionNoise
   * @brief Set the IR detection noise threshold.
   * @param noise Noise threshold.
   * @n Range 0–255; typical default 0x0A. Used to suppress noise triggers.
   * @return `true` on success, `false` on I2C error.
   */
  bool setIRDetectionNoise(uint8_t noise);

  /**
   * @fn setScaleResolution
   * @brief Set X/Y output scaling.
   * @param x_axis X scale.
   * @param y_axis Y scale.
   * @n Each axis is 0–4095. Values above 4095 are clamped.
   * @n Maps the native 98×98 sensor grid to 12-bit outputs. Examples use 2490×2490.
   * @return `true` on success, `false` on I2C error.
   */
  bool setScaleResolution(uint16_t x_axis, uint16_t y_axis);

  /**
   * @fn setMaxTrackingNumber
   * @brief Set the maximum number of IR spots to track.
   * @param num Maximum object count.
   * @n Range 1–16, clamped by the library. Typical default is 0x10 (16).
   * @return `true` on success, `false` on I2C error.
   */
  bool setMaxTrackingNumber(uint8_t num);

  /**
   * @fn setExposure
   * @brief Write 16-bit exposure, commit it, and wait a fixed 100 ms.
   * @param expo Exposure value.
   * @n Range 100–65535. Values below 100 are clamped to 100.
   * @n The effective maximum depends on the frame period.
   * @n On success the call waits 100 ms. The wait is skipped when a write fails.
   * @return `true` if both writes succeed, `false` on I2C error.
   */
  bool setExposure(uint16_t expo);

  /**
   * @fn getExposure
   * @brief Read current exposure from the readback registers (16-bit).
   * @param expo Output reference.
   * @return `true` on success, `false` on I2C error.
   */
  bool getExposure(uint16_t &expo);

  /**
   * @fn setI2CAddress
   * @brief Save a new 7-bit I2C address to bridge flash.
   * @param addr7bit New 7-bit address.
   * @n Valid range 0x08–0x77.
   * @n Takes effect only after reset or power cycle.
   * @n Returns false on an invalid address, I2C error, flash failure, or timeout (200 ms).
   * @return `true` if the address is saved, `false` otherwise.
   */
  bool setI2CAddress(uint8_t addr7bit);

  /**
   * @fn getI2CAddress
   * @brief Read the current active 7-bit I2C address.
   * @param addr7bit Output reference.
   * @return `true` on success, `false` on I2C error.
   */
  bool getI2CAddress(uint8_t &addr7bit);

  /**
   * @fn setIndicator
   * @brief Turn the on-board indicator LED on or off.
   * @param status `true` to turn on, `false` to turn off.
   * @return `true` on success, `false` on I2C error.
   */
  bool setIndicator(bool status);

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
   * @brief Average brightness of the selected spot in the last `update()` frame.
   * @param IRnum Index 0–15.
   * @n Returns 0–255, or 0 if the index is invalid.
   * @return Average brightness byte.
   */
  uint8_t getAverageBrightness(int IRnum);

  /**
   * @fn getMaxBrightness
   * @brief Peak brightness of the selected spot in the last `update()` frame.
   * @param IRnum Index 0–15.
   * @n Returns 0–255, or 0 if the index is invalid.
   * @return Max brightness byte.
   */
  uint8_t getMaxBrightness(int IRnum);

  /**
   * @fn getRadius
   * @brief Radius of the selected spot in the last `update()` frame.
   * @param IRnum Index 0–15.
   * @n Low 4 bits of the packed radius field, typically 0–15. Returns 0 if the index is invalid.
   * @return Radius value.
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

* Date 2026-11-07
* Version V1.0.0

## Credits

Written by thdyyl(yuanlong.yu@dfrobot.com), 2026-11-07. (Welcome to our [website](https://www.dfrobot.com/))
