DFRobot_IRPositionV2
===========================

- [中文版](./README_CN.md)

This IR positioning module (SKU: SEN0701) uses a PixArt PAJ7025R2 infrared image sensor with an onboard CS32 I2C bridge to detect multiple IR spots in real time and report their positions. The module can track up to 16 infrared spots per frame and outputs the center coordinates, bounding box, average brightness, maximum brightness and approximate radius of each spot. The sensor native resolution is about 98×98; the bridge can scale the X and Y axes independently into a 12-bit output range (0–4095). The host talks to the module over I2C at the default 7-bit address 0x58, and a new address can be saved to Flash (valid range 0x08–0x77, applied after reset or power cycle). This is the Raspberry Pi Python driver: it reads spot coordinates and brightness, and sets the brightness detection threshold, noise threshold, exposure, maximum tracking count, indicator LED and I2C address. One frame is 256 bytes at a 16-bit register address, so it depends on `smbus2`. Example `get_position.py` shows spots in the terminal or a desktop window; `set_i2c_address.py` saves a new address.

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

This library reads the infrared spots on Raspberry Pi and sets the detection threshold, coordinate scale, exposure, maximum tracking count, indicator LED and I2C address. Call `update()` once before reading each spot. Example `get_position.py` shows spots in the terminal or a desktop window. Example `set_i2c_address.py` saves a new 7-bit address.

## Installation

1. Enable the I2C interface on the Raspberry Pi (`sudo raspi-config` → Interface Options → I2C). For a higher frame rate, add `dtparam=i2c_arm_baudrate=400000` to `/boot/config.txt` and reboot.
2. Install the `smbus2` dependency:

```shell
sudo pip install smbus2
```

3. Download this library to the Raspberry Pi, then open the `examples` folder. Type `python demox.py` to run a routine. For example, to run `get_position.py`:

```python
python get_position.py
```

The desktop window needs `python3-tk` (`sudo apt install python3-tk`). Without a desktop or tkinter, `get_position.py` falls back to the terminal.

## Methods

```python

def __init__(self, bus=1, addr=0x58):
  '''!
    @brief DFRobot_IRPositionV2 constructor
    @param bus  I2C bus number of the Raspberry Pi, usually 1 (/dev/i2c-1)
    @param addr 7-bit I2C address of the bridge, default 0x58
  '''

def begin(self):
  '''!
    @brief Init the IR position V2 module and verify communication by reading the frame region
    @return Return init status
    @retval True  Succeed
    @retval False Failed, I2C error
  '''

def update(self):
  '''!
    @brief Read one full frame (16 objects x 16 bytes) into an internal buffer
    @n Call once per loop before get_ir_coordinate / get_ir_bound / brightness / radius accessors
    @return True on success, False on I2C error
  '''

def set_ir_brightness_threshold(self, brightness):
  '''!
    @brief Set the IR spot brightness threshold
    @param brightness Threshold value
    @n Range 0-255; typical factory default 0x97
    @n A higher value is a stricter brightness gate, and dimmer spots are dropped
    @return True on success, False on I2C error
  '''

def set_ir_detection_noise(self, noise):
  '''!
    @brief Set the IR detection noise threshold
    @param noise Noise threshold
    @n Range 0-255; typical default 0x0A. Used to suppress noise triggers
    @return True on success, False on I2C error
  '''

def set_scale_resolution(self, x_axis, y_axis):
  '''!
    @brief Set X/Y output scaling
    @param x_axis X scale
    @param y_axis Y scale
    @n Each axis is 0-4095. Values above 4095 are clamped
    @n Maps the native 98x98 sensor grid to 12-bit outputs. Examples use 2490x2490
    @return True on success, False on I2C error
  '''

def set_max_tracking_number(self, num):
  '''!
    @brief Set the maximum number of IR spots to track
    @param num Maximum object count
    @n Range 1-16, clamped by the library. Typical default is 0x10 (16)
    @return True on success, False on I2C error
  '''

def set_exposure(self, expo):
  '''!
    @brief Write 16-bit exposure, commit it, and wait a fixed 100 ms
    @param expo Exposure value
    @n Range 100-65535. Values below 100 are clamped to 100
    @n The effective maximum depends on the frame period
    @n On success the call waits 100 ms. The wait is skipped when a write fails
    @return True if both writes succeed, False on I2C error
  '''

def get_exposure(self):
  '''!
    @brief Read current exposure from the sensor readback registers (16-bit)
    @return Return the exposure value 100-65535, -1 on I2C error
  '''

def set_i2c_address(self, addr7bit):
  '''!
    @brief Save a new 7-bit I2C address to bridge flash
    @param addr7bit New 7-bit address
    @n Valid range 0x08-0x77
    @n Takes effect only after reset or power cycle
    @n Returns False on an invalid address, I2C error, flash failure, or timeout (200 ms)
    @return True if the address is saved, False otherwise
  '''

def get_i2c_address(self):
  '''!
    @brief Read the current active 7-bit I2C address from the bridge
    @return Return the active 7-bit address, -1 on I2C error
  '''

def set_indicator(self, status):
  '''!
    @brief Turn the on-board indicator LED on or off
    @param status True to turn on, False to turn off
    @return True if the register write succeeds, False on I2C error
  '''

def get_ir_coordinate(self, ir_num):
  '''!
    @brief Get the center coordinates of the IR spot from the last update() frame
    @param ir_num Index 0-15 for the N-th spot
    @return Return a list [x, y], 12-bit each (0-4095 after scaling), [0, 0] if the index is invalid
  '''

def get_ir_bound(self, ir_num):
  '''!
    @brief Get boundary box of the IR spot (native 0-98 range per side)
    @param ir_num Index 0-15
    @return Return a list [up, down, left, right], [0, 0, 0, 0] if the index is invalid
  '''

def get_average_brightness(self, ir_num):
  '''!
    @brief Average brightness of the selected spot in the last update() frame
    @param ir_num Index 0-15
    @n Returns 0-255, or 0 if the index is invalid
    @return Average brightness
  '''

def get_max_brightness(self, ir_num):
  '''!
    @brief Peak brightness of the selected spot in the last update() frame
    @param ir_num Index 0-15
    @n Returns 0-255, or 0 if the index is invalid
    @return Max brightness
  '''

def get_radius(self, ir_num):
  '''!
    @brief Radius of the selected spot in the last update() frame
    @param ir_num Index 0-15
    @n Low 4 bits of the packed radius field, typically 0-15. Returns 0 if the index is invalid
    @return Radius value
  '''
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

- 2026-11-07 - Version 1.0.0 released.

## Credits

Written by thdyyl(yuanlong.yu@dfrobot.com), 2026-11-07. (Welcome to our [website](https://www.dfrobot.com/))
