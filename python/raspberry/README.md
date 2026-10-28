DFRobot_IRPositionV2
===========================

- [中文版](./README_CN.md)

The IR positioning module (SKU: SEN0701) tracks up to 16 infrared spots in each frame and reports the center, boundary, brightness and radius of every spot. It connects over I2C at the default 7-bit address 0x58.

This Raspberry Pi library reads one frame with `update()`, then sets the detection threshold, coordinate scale, exposure, maximum number of targets, the indicator LED and the module I2C address. It uses `smbus2` because each frame is a 256-byte read from a 16-bit register address.

## Product Link (https://www.dfrobot.com)

    SKU: SEN0701

## Table of Contents

* [Summary](#summary)
* [Configuration parameter ranges](#configuration-parameter-ranges)
* [Installation](#installation)
* [Examples](#examples)
* [Methods](#methods)
* [Compatibility](#compatibility)
* [History](#history)
* [Credits](#credits)

## Summary

* Track up to **16** infrared spots per frame (index **0–15**).
* Communicates over **I2C** with 7-bit slave address **`0x58`**. The bridge uses a **16-bit register address** and a **256-byte** frame, so this library depends on **`smbus2`** (`i2c_rdwr` with repeated start).
* Outputs per object include **center (12-bit scaled coordinates)**, **boundary** on the native **98×98** grid, **average / max brightness**, and **radius** (low 4 bits of the packed radius field).
* Call **`update()`** once per loop to refresh the internal frame buffer before reading any object slot.

## Configuration parameter ranges

Use these ranges when tuning parameters in your script or product integration:

| Item | Range / note |
| ---- | -------------- |
| **I2C address** | 7-bit `0x58`; pass the bus number and a 7-bit address to `DFRobot_IRPositionV2(bus, addr)`. |
| **`set_i2c_address(addr7bit)`** | `addr7bit`: **0x08–0x77**. Saves to bridge flash and takes effect after reset or power cycle. Returns as soon as the save finishes, and returns `False` if the address is invalid, communication fails, flash save fails, or it is still pending after the internal limit (200 ms). |
| **`set_ir_brightness_threshold(brightness)`** | `brightness`: **0–255** (typical default in firmware/docs **0x97**). Higher → stricter brightness gate. |
| **`set_ir_detection_noise(noise)`** | `noise`: **0–255** (typical default **0x0A**). Adjusts noise rejection. |
| **`set_scale_resolution(x, y)`** | Each axis **0–4095** (values above 4095 are clamped). Maps native sensor space to scaled 12-bit outputs; common example **2490×2490** for ~30× scale from **98×98**. |
| **`set_max_tracking_number(num)`** | `num`: **1–16** (clamped by the library). |
| **`set_exposure(expo)`** | `expo`: **100–65535** (values below 100 are clamped by the library; effective maximum depends on frame period). Returns `True` when both writes succeed, then waits a fixed **100 ms**. The wait is skipped when a write fails. Use `get_exposure()` to read the current value. |
| **`get_ir_coordinate(ir_num)`** | `ir_num`: **0–15**. Returns `[x, y]`, **12-bit** each in **0–4095** after scaling (see `set_scale_resolution`). |
| **`get_ir_bound(ir_num)`** | `ir_num`: **0–15**. Returns `[up, down, left, right]` in **0–98** (native sensor cells). |
| **`get_average_brightness` / `get_max_brightness`** | Return **0–255**. |
| **`get_radius(ir_num)`** | Radius uses **4 bits** (typical interpretation **0–15** pixels). |

Exact factory defaults and optical behavior may vary; always validate on your hardware.

## Installation

1. Enable the I2C interface on the Raspberry Pi (`sudo raspi-config` → Interface Options → I2C). For best frame rate set the bus to 400 kHz by adding `dtparam=i2c_arm_baudrate=400000` to `/boot/config.txt` and rebooting.
2. Install the `smbus2` dependency:

```shell
sudo pip install smbus2
```

3. Download this library to Raspberry Pi, then open the `examples` folder. Type `python demox.py` on the command line to execute a routine `demox.py`. For example, to execute the `get_position.py` routine, you need to enter:

```python
python get_position.py
```

## Examples

* `get_position.py`: read IR object positions and metrics. `DISPLAY_MODE` at the top of the script selects the output:
  * `DISPLAY_CONSOLE`: print all 16 slots to the terminal every frame; works on headless / Raspberry Pi OS Lite.
  * `DISPLAY_GUI` (default): a tkinter window that draws the tracked spots on a 2D canvas (index, radius, boundary box), lists their coordinates and brightness, and offers sliders for brightness threshold / noise threshold / exposure plus an indicator LED switch. Needs the Raspberry Pi desktop or `ssh -X`; install tkinter with `sudo apt install python3-tk` if missing. If tkinter or a display is unavailable the script falls back to the console mode automatically.
* `set_i2c_address.py`: save a new 7-bit bridge address to flash; reset or power-cycle after success.

## Methods

```python
  '''!
    @brief DFRobot_IRPositionV2 constructor
    @param bus  I2C bus number of the Raspberry Pi, usually 1 (/dev/i2c-1)
    @param addr 7-bit I2C address of the bridge, default 0x58
  '''
  def __init__(self, bus=1, addr=0x58):

  '''!
    @brief Init the IR position V2 module and verify communication by reading the frame region
    @return Return init status
    @retval True  Succeed
    @retval False Failed, I2C error
  '''
  def begin(self):

  '''!
    @brief Read one full frame (16 objects x 16 bytes) into an internal buffer
    @details Call once per loop before get_ir_coordinate() / brightness accessors to minimize I2C traffic.
    @return Return update status
    @retval True  Succeed
    @retval False Failed, I2C error
  '''
  def update(self):

  '''!
    @brief Set the IR spot brightness threshold (factory default in firmware is typically 0x97)
    @param brightness Range 0-255, higher is a stricter brightness gate
    @return True on success, False on I2C error
  '''
  def set_ir_brightness_threshold(self, brightness):

  '''!
    @brief Set the IR detection noise threshold (default is often 0x0A)
    @param noise Range 0-255
    @return True on success, False on I2C error
  '''
  def set_ir_detection_noise(self, noise):

  '''!
    @brief Set X/Y output scaling (sensor native grid is 98x98; commonly scaled up, max 4095 per axis)
    @param x_axis Range 0-4095, values above 4095 are clamped
    @param y_axis Range 0-4095, values above 4095 are clamped
    @return True on success, False on I2C error
  '''
  def set_scale_resolution(self, x_axis, y_axis):

  '''!
    @brief Set the maximum number of IR spots to track (default often 0x10 = 16)
    @param num Range 1-16, clamped by the library
    @return True on success, False on I2C error
  '''
  def set_max_tracking_number(self, num):

  '''!
    @brief Write 16-bit exposure, commit it, and wait a fixed 100 ms
    @param expo Range 100-65535. Values below 100 are clamped to 100; effective maximum depends on frame period
    @return True if both writes succeed, False on I2C error. The wait is skipped when a write fails.
  '''
  def set_exposure(self, expo):

  '''!
    @brief Read current exposure from the sensor readback registers (16-bit)
    @return Return the exposure value 100-65535, -1 on I2C error
  '''
  def get_exposure(self):

  '''!
    @brief Save a new 7-bit I2C address to bridge flash; it takes effect after reset or power cycle
    @param addr7bit New 7-bit address, valid range 0x08-0x77
    @return True if the address is saved, False on invalid address, I2C error, flash failure, or timeout
  '''
  def set_i2c_address(self, addr7bit):

  '''!
    @brief Read the current active 7-bit I2C address from the bridge
    @return Return the active 7-bit address, -1 on I2C error
  '''
  def get_i2c_address(self):

  '''!
    @brief Turn the on-board indicator LED on or off
    @param status True to turn on, False to turn off
    @return True if the register write succeeds, False on I2C error
  '''
  def set_indicator(self, status):

  '''!
    @brief Get the center coordinates of the IR spot from the last update() frame
    @param ir_num Index 0-15 for the N-th spot
    @return Return a list [x, y], 12-bit each (0-4095 after scaling), [0, 0] if the index is invalid
  '''
  def get_ir_coordinate(self, ir_num):

  '''!
    @brief Get boundary box of the IR spot (native 0-98 range per side)
    @param ir_num Index 0-15
    @return Return a list [up, down, left, right], [0, 0, 0, 0] if the index is invalid
  '''
  def get_ir_bound(self, ir_num):

  '''!
    @brief Average brightness of the selected spot in the last frame
    @param ir_num Index 0-15
    @return Return brightness 0-255, 0 if the index is invalid
  '''
  def get_average_brightness(self, ir_num):

  '''!
    @brief Peak brightness of the selected spot in the last frame
    @param ir_num Index 0-15
    @return Return brightness 0-255, 0 if the index is invalid
  '''
  def get_max_brightness(self, ir_num):

  '''!
    @brief Radius (pixels) of the selected spot from the last frame
    @param ir_num Index 0-15
    @return Return radius (low 4 bits of the packed field), 0 if the index is invalid
  '''
  def get_radius(self, ir_num):
```

## Compatibility

* RaspberryPi Version

| Board         | Work Well | Work Wrong | Untested | Remarks |
| ------------- | :-------: | :--------: | :------: | ------- |
| Raspberry Pi2 |          |            |    √    |         |
| Raspberry Pi3 |          |            |    √    |         |
| Raspberry Pi4 |    √    |            |          |         |

* Python Version

| Python  | Work Well | Work Wrong | Untested | Remarks |
| ------- | :-------: | :--------: | :------: | ------- |
| Python2 |          |            |    √    |         |
| Python3 |    √    |            |          |         |

## History

- 2026-10-26 - Version 1.0.0 released.

## Credits

Written by thdyyl(yuanlong.yu@dfrobot.com), 2026. (Welcome to our [website](https://www.dfrobot.com/))
