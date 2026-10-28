# -*- coding: utf-8 -*
'''!
@file        DFRobot_IRPositionV2.py
@brief       DFRobot_IRPositionV2 library is developed for the DFRobot IR positioning module (SKU: SEN0701).
@details     Multi-object IR position tracking via a CS32 I2C bridge and PAJ7025R2 sensor; read up to 16 targets per frame.
@n           The host reads a 16 x 16-byte object frame per update() and can configure thresholds, scaling,
@n           exposure, max tracked targets, indicator LED and the bridge I2C address.
@copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
@license     The MIT License (MIT)
@author      [thdyyl](yuanlong.yu@dfrobot.com)
@version     V1.0.0
@date        2026-10-26
@url         https://github.com/DFRobot/DFRobot_IRPositionV2
'''

import time

# smbus2 is required: the bridge uses a 16-bit register address with a repeated-start read,
# and one frame is 256 bytes, which the classic smbus API (8-bit command, 32-byte limit) cannot handle.
#   sudo pip install smbus2
from smbus2 import SMBus, i2c_msg


class DFRobot_IRPositionV2:
  '''!
  @brief Define DFRobot_IRPositionV2 basic class
  @details For driving the IR position V2 module (PAJ7025R2 + CS32 bridge over I2C)
  '''

  ## Register map, same address space as the bridge firmware. Kept on the class so importing the module does not export these names.
  _I2C_ADDR = 0x58  # 7-bit I2C slave address, factory default
  _EXPOSURE_MIN = 100  # Minimum B_expo[15:0]; datasheet minimum is 20 us
  _EXPOSURE_APPLY_S = 0.1  # Fixed wait after a successful exposure commit
  _I2C_ADDR_MIN = 0x08
  _I2C_ADDR_MAX = 0x77
  _I2C_ADDR_SAVE_MAX_S = 0.2  # Stop waiting once the save leaves pending, or after this long

  ## Frame region: 16 objects x 16 bytes, read-only from host.
  _REG_FRAME_LEN = 0x10
  _REG_FRAME_COUNT = 16
  _REG_FRAMES_START = 0x00
  _REG_FRAMES_END = 0xFF
  _REG_FRAME_SIZE = _REG_FRAME_LEN * _REG_FRAME_COUNT  # 256 bytes

  ## Parameter mirror base (read/write), length must match firmware.
  _REG_PARAM_BASE = 0x100
  _REG_PARAM_LEN = 30
  _REG_TOTAL_SIZE = _REG_FRAME_SIZE + _REG_PARAM_LEN  # 286 bytes total mapped span

  ## Parameter offsets from _REG_PARAM_BASE (must match firmware reg.h).
  _REG_BRIGHTNESS_THRES = 0x00  # Sensor reg 0x47, bank 0x0c
  _REG_NOISE_THRES = 0x01  # reg 0x0F, bank 0x00
  _REG_AREA_THRES_MAX_LSB = 0x02  # reg 0x0b, bank 0x00
  _REG_AREA_THRES_MAX_MSB = 0x03  # reg 0x0c, bank 0x00
  _REG_AREA_THRES_MIN = 0x04  # reg 0x46, bank 0x0c
  _REG_SCALE_RESOLUTION_X_LSB = 0x05  # reg 0x60, bank 0x0c
  _REG_SCALE_RESOLUTION_X_8_11 = 0x06  # reg 0x61, bank 0x0c
  _REG_SCALE_RESOLUTION_Y_LSB = 0x07  # reg 0x62, bank 0x0c
  _REG_SCALE_RESOLUTION_Y_8_11 = 0x08  # reg 0x63, bank 0x0c
  _REG_OBJECT_LABELLING = 0x09  # reg 0x12, bank 0x00
  _REG_OBJECT_NUM = 0x10  # reg 0x19, bank 0x00
  _REG_ORIRNTATION_RATIO = 0x11  # reg 0x10, bank 0x00
  _REG_W_B_EXPO_LSB = 0x12  # reg 0x0f, bank 0x0c
  _REG_W_B_EXPO_MSB = 0x13  # reg 0x10, bank 0x0c
  _REG_R_B_EXPO_LSB = 0x14  # reg 0x0e, bank 0x01
  _REG_R_B_EXPO_MSB = 0x15  # reg 0x0f, bank 0x01
  _REG_W_B_EXPO_COMMIT = 0x16  # Write 0x01 to commit exposure to bank1
  _REG_I2C_ADDR_PENDING = 0x17  # Pending 7-bit I2C address to save
  _REG_I2C_ADDR_COMMIT = 0x18  # Write 0xA5 to save pending address to flash
  _REG_I2C_ADDR_STATUS = 0x19  # Address save status
  _REG_I2C_ADDR_ACTIVE = 0x1A  # Current active 7-bit I2C address, read-only
  _REG_INDICATOR = 0x1B  # Indicator LED: write 0 off, non-zero on; reads back 0/1

  _REG_I2C_ADDR_COMMIT_KEY = 0xA5
  _REG_I2C_ADDR_STATUS_OK = 0x00
  _REG_I2C_ADDR_STATUS_INVALID = 0x01
  _REG_I2C_ADDR_STATUS_FLASH_FAIL = 0x02
  _REG_I2C_ADDR_STATUS_PENDING = 0x03

  ## Byte offsets inside one 16-byte object slot (matches PAJ7025R2 regbank05_t).
  _OBJ_AREA_07_00 = 0  # Area[7:0]
  _OBJ_AREA_13_08 = 1  # Area[13:8], valid in lower 6 bits
  _OBJ_CX_07_00 = 2  # Cx[7:0]
  _OBJ_CX_11_08 = 3  # Cx[11:8], valid in lower 4 bits
  _OBJ_CY_07_00 = 4  # Cy[7:0]
  _OBJ_CY_11_08 = 5  # Cy[11:8], valid in lower 4 bits
  _OBJ_AVG_BRIGHTNESS = 6
  _OBJ_MAX_BRIGHTNESS = 7
  _OBJ_RADIUS_RANGE = 8  # bits[3:0]=radius, bits[7:4]=range
  _OBJ_BOUNDARY_LEFT = 9  # bits[6:0]
  _OBJ_BOUNDARY_RIGHT = 10
  _OBJ_BOUNDARY_UP = 11
  _OBJ_BOUNDARY_DOWN = 12
  _OBJ_ASPECT_RATIO = 13
  _OBJ_VX = 14
  _OBJ_VY = 15

  ## Maximum payload per I2C transaction (same as the Arduino driver on non-AVR boards).
  _I2C_READ_CHUNK = 128
  _I2C_WRITE_CHUNK = 128 - 2

  def __init__(self, bus=1, addr=_I2C_ADDR):
    '''!
    @brief DFRobot_IRPositionV2 constructor
    @param bus  I2C bus number of the Raspberry Pi, usually 1 (/dev/i2c-1)
    @param addr 7-bit I2C address of the bridge, default 0x58
    '''
    self._bus = bus
    self._addr = addr
    self._i2cbus = SMBus(bus)
    self._frame = [0] * self._REG_FRAME_SIZE

  def begin(self):
    '''!
    @brief Init the IR position V2 module and verify communication by reading the frame region
    @return Return init status
    @retval True  Succeed
    @retval False Failed, I2C error
    '''
    return self._read_reg(self._REG_FRAMES_START, 1) is not None

  def update(self):
    '''!
    @brief Read one full frame (16 objects x 16 bytes) into an internal buffer
    @details Call once per loop before get_ir_coordinate() / brightness accessors to minimize I2C traffic.
    @return Return update status
    @retval True  Succeed
    @retval False Failed, I2C error
    '''
    frame = self._read_reg(self._REG_FRAMES_START, self._REG_FRAME_SIZE)
    if frame is None:
      return False
    self._frame = frame
    return True

  def set_ir_brightness_threshold(self, brightness):
    '''!
    @brief Set the IR spot brightness threshold (factory default in firmware is typically 0x97)
    @param brightness Range 0-255, higher is a stricter brightness gate
    @return True on success, False on I2C error
    '''
    brightness = self._clamp(brightness, 0, 255)
    return self._write_reg(self._REG_PARAM_BASE + self._REG_BRIGHTNESS_THRES, [brightness])

  def set_ir_detection_noise(self, noise):
    '''!
    @brief Set the IR detection noise threshold (default is often 0x0A)
    @param noise Range 0-255
    @return True on success, False on I2C error
    '''
    noise = self._clamp(noise, 0, 255)
    return self._write_reg(self._REG_PARAM_BASE + self._REG_NOISE_THRES, [noise])

  def set_scale_resolution(self, x_axis, y_axis):
    '''!
    @brief Set X/Y output scaling (sensor native grid is 98x98; commonly scaled up, max 4095 per axis)
    @param x_axis Range 0-4095, values above 4095 are clamped
    @param y_axis Range 0-4095, values above 4095 are clamped
    @return True on success, False on I2C error
    '''
    x_axis = self._clamp(x_axis, 0, 4095)
    y_axis = self._clamp(y_axis, 0, 4095)
    buf = [x_axis & 0xFF, (x_axis >> 8) & 0x0F, y_axis & 0xFF, (y_axis >> 8) & 0x0F]
    return self._write_reg(self._REG_PARAM_BASE + self._REG_SCALE_RESOLUTION_X_LSB, buf)

  def set_max_tracking_number(self, num):
    '''!
    @brief Set the maximum number of IR spots to track (default often 0x10 = 16)
    @param num Range 1-16, clamped by the library
    @return True on success, False on I2C error
    '''
    num = self._clamp(num, 1, 16)
    return self._write_reg(self._REG_PARAM_BASE + self._REG_OBJECT_NUM, [num])

  def set_exposure(self, expo):
    '''!
    @brief Write 16-bit exposure, commit it, and wait a fixed 100 ms
    @param expo Range 100-65535. Values below 100 are clamped to 100; effective maximum depends on frame period
    @return True if both writes succeed, False on I2C error. The wait is skipped when a write fails.
    '''
    expo = self._clamp(expo, self._EXPOSURE_MIN, 0xFFFF)
    if not self._write_reg(self._REG_PARAM_BASE + self._REG_W_B_EXPO_LSB, [expo & 0xFF, (expo >> 8) & 0xFF]):
      return False
    if not self._write_reg(self._REG_PARAM_BASE + self._REG_W_B_EXPO_COMMIT, [0x01]):
      return False
    time.sleep(self._EXPOSURE_APPLY_S)
    return True

  def get_exposure(self):
    '''!
    @brief Read current exposure from the sensor readback registers (16-bit)
    @return Return the exposure value 100-65535, -1 on I2C error
    '''
    buf = self._read_reg(self._REG_PARAM_BASE + self._REG_R_B_EXPO_LSB, 2)
    if buf is None:
      return -1
    return buf[0] | (buf[1] << 8)

  def set_i2c_address(self, addr7bit):
    '''!
    @brief Save a new 7-bit I2C address to bridge flash; it takes effect after reset or power cycle
    @param addr7bit New 7-bit address, valid range 0x08-0x77
    @return True if the address is saved, False on invalid address, I2C error, flash failure, or timeout
    '''
    if addr7bit < self._I2C_ADDR_MIN or addr7bit > self._I2C_ADDR_MAX:
      return False
    if not self._write_reg(self._REG_PARAM_BASE + self._REG_I2C_ADDR_PENDING, [addr7bit, self._REG_I2C_ADDR_COMMIT_KEY]):
      return False

    status = self._REG_I2C_ADDR_STATUS_FLASH_FAIL
    start = time.time()
    while True:
      status = self._get_i2c_address_status()
      if status < 0:
        return False
      if status != self._REG_I2C_ADDR_STATUS_PENDING:
        break
      if (time.time() - start) >= self._I2C_ADDR_SAVE_MAX_S:
        break
      time.sleep(0.005)
    return status == self._REG_I2C_ADDR_STATUS_OK

  def get_i2c_address(self):
    '''!
    @brief Read the current active 7-bit I2C address from the bridge
    @return Return the active 7-bit address, -1 on I2C error
    '''
    buf = self._read_reg(self._REG_PARAM_BASE + self._REG_I2C_ADDR_ACTIVE, 1)
    if buf is None:
      return -1
    return buf[0]

  def set_indicator(self, status):
    '''!
    @brief Turn the on-board indicator LED on or off
    @param status True to turn on, False to turn off
    @return True if the register write succeeds, False on I2C error
    '''
    val = 0x01 if status else 0x00
    return self._write_reg(self._REG_PARAM_BASE + self._REG_INDICATOR, [val])

  def get_ir_coordinate(self, ir_num):
    '''!
    @brief Get the center coordinates of the IR spot from the last update() frame
    @param ir_num Index 0-15 for the N-th spot
    @return Return a list [x, y], 12-bit each (0-4095 after scaling), [0, 0] if the index is invalid
    '''
    obj = self._read_object(ir_num)
    if obj is None:
      return [0, 0]
    cx = obj[self._OBJ_CX_07_00] | ((obj[self._OBJ_CX_11_08] & 0x0F) << 8)
    cy = obj[self._OBJ_CY_07_00] | ((obj[self._OBJ_CY_11_08] & 0x0F) << 8)
    return [cx, cy]

  def get_ir_bound(self, ir_num):
    '''!
    @brief Get boundary box of the IR spot (native 0-98 range per side)
    @param ir_num Index 0-15
    @return Return a list [up, down, left, right], [0, 0, 0, 0] if the index is invalid
    '''
    obj = self._read_object(ir_num)
    if obj is None:
      return [0, 0, 0, 0]
    return [obj[self._OBJ_BOUNDARY_UP] & 0x7F, obj[self._OBJ_BOUNDARY_DOWN] & 0x7F, obj[self._OBJ_BOUNDARY_LEFT] & 0x7F, obj[self._OBJ_BOUNDARY_RIGHT] & 0x7F]

  def get_average_brightness(self, ir_num):
    '''!
    @brief Average brightness of the selected spot in the last frame
    @param ir_num Index 0-15
    @return Return brightness 0-255, 0 if the index is invalid
    '''
    obj = self._read_object(ir_num)
    if obj is None:
      return 0
    return obj[self._OBJ_AVG_BRIGHTNESS]

  def get_max_brightness(self, ir_num):
    '''!
    @brief Peak brightness of the selected spot in the last frame
    @param ir_num Index 0-15
    @return Return brightness 0-255, 0 if the index is invalid
    '''
    obj = self._read_object(ir_num)
    if obj is None:
      return 0
    return obj[self._OBJ_MAX_BRIGHTNESS]

  def get_radius(self, ir_num):
    '''!
    @brief Radius (pixels) of the selected spot from the last frame
    @param ir_num Index 0-15
    @return Return radius (low 4 bits of the packed field), 0 if the index is invalid
    '''
    obj = self._read_object(ir_num)
    if obj is None:
      return 0
    return obj[self._OBJ_RADIUS_RANGE] & 0x0F

  def _get_i2c_address_status(self):
    '''!
    @brief Read the last I2C address save status
    @return Return status: 0x00 OK, 0x01 invalid address/commit, 0x02 flash failure, 0x03 pending, -1 on I2C error
    '''
    buf = self._read_reg(self._REG_PARAM_BASE + self._REG_I2C_ADDR_STATUS, 1)
    if buf is None:
      return -1
    return buf[0]

  def _read_object(self, index):
    '''!
    @brief Get one 16-byte object slot from the cached frame
    @param index Object index 0-15
    @return Return a list of 16 bytes, None if the index is out of range
    '''
    if index < 0 or index >= self._REG_FRAME_COUNT:
      return None
    start = index * self._REG_FRAME_LEN
    return self._frame[start : start + self._REG_FRAME_LEN]

  def _clamp(self, value, low, high):
    '''!
    @brief Clamp an integer value into [low, high]
    '''
    value = int(value)
    if value < low:
      return low
    if value > high:
      return high
    return value

  def _read_reg(self, reg, length):
    '''!
    @brief Read a contiguous block from the bridge register space
    @details The 16-bit register address is sent LSB first, followed by a repeated-start read.
    @n       Long reads are split into 128-byte chunks; the address is re-sent for each chunk.
    @param reg Start register address (16-bit)
    @param length Number of bytes to read
    @return Return a list of bytes on success, None on I2C error
    '''
    if length <= 0:
      return None
    data = []
    offset = 0
    try:
      while offset < length:
        chunk = min(self._I2C_READ_CHUNK, length - offset)
        addr = reg + offset
        write = i2c_msg.write(self._addr, [addr & 0xFF, (addr >> 8) & 0xFF])
        read = i2c_msg.read(self._addr, chunk)
        self._i2cbus.i2c_rdwr(write, read)
        data.extend(list(read))
        offset += chunk
    except Exception:
      return None
    if len(data) != length:
      return None
    return data

  def _write_reg(self, reg, data):
    '''!
    @brief Write a contiguous block to the bridge register space
    @details Splits into 126-byte chunks; the register address is advanced for each chunk.
    @param reg Start register address (16-bit)
    @param data List of bytes to write
    @return True on success, False on I2C error
    '''
    data = list(data)
    if len(data) == 0:
      return False
    offset = 0
    try:
      while offset < len(data):
        chunk = min(self._I2C_WRITE_CHUNK, len(data) - offset)
        addr = reg + offset
        buf = [addr & 0xFF, (addr >> 8) & 0xFF] + [b & 0xFF for b in data[offset : offset + chunk]]
        self._i2cbus.i2c_rdwr(i2c_msg.write(self._addr, buf))
        offset += chunk
    except Exception:
      return False
    return True
