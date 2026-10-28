/*!
 * @file DFRobot_IRPositionV2.h
 * @brief Define the basic structure of class DFRobot_IRPositionV2 and its public APIs.
 * @details Multi-object IR position tracking via a CS32 I2C bridge and PAJ7025R2 sensor; read up to 16 targets per frame.
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @License     The MIT License (MIT)
 * @author [thdyyl](yuanlong.yu@dfrobot.com)
 * @version  V1.0
 * @date  2026-04-13
 * @url         https://github.com/DFRobot/DFRobot_IRPositionV2
 */
#ifndef __DFROBOT_IRPOSITIONV2_H
#define __DFROBOT_IRPOSITIONV2_H

#include "Arduino.h"
#include "Wire.h"

/**
 * @class DFRobot_IRPositionV2
 * @brief Driver for IR position V2 (PAJ7025R2 + CS32 bridge over I2C).
 */
class DFRobot_IRPositionV2 {
public:
  /**
   * @fn DFRobot_IRPositionV2
   * @brief Default constructor (uses `Wire` and 7-bit address 0x58).
   */
  DFRobot_IRPositionV2();

  /**
   * @fn DFRobot_IRPositionV2
   * @brief Construct with a custom I2C bus and 7-bit address.
   * @param pWire Pointer to `TwoWire` instance (pass `NULL` to use `Wire`).
   * @param addr7bit 7-bit I2C address.
   */
  DFRobot_IRPositionV2(TwoWire *pWire, uint8_t addr7bit);

  /**
   * @fn ~DFRobot_IRPositionV2
   * @brief Destructor of DFRobot_IRPositionV2 class.
   */
  ~DFRobot_IRPositionV2();

  /**
   * @fn begin
   * @brief Initialize I2C (400 kHz) and verify communication by reading the frame region.
   * @return `true` if the probe read succeeds, `false` on I2C error.
   */
  bool begin();

  /**
   * @fn update
   * @brief Read one full frame (16 objects × 16 bytes) into an internal buffer.
   * @details Call once per loop before `getIRCoordinate()` / brightness accessors to minimize I2C traffic.
   * @return `true` on success, `false` on I2C error.
   */
  bool update();

  /**
   * @fn setIRBrightnessThreshold
   * @brief Set the IR spot brightness threshold (factory default in firmware is typically `0x97`).
   * @param brightness Range 0–255.
   * @return `true` on success, `false` on I2C error.
   */
  bool setIRBrightnessThreshold(uint8_t brightness);

  /**
   * @fn setIRDetectionNoise
   * @brief Set the IR detection noise threshold (default is often `0x0A`).
   * @param noise Range 0–255.
   * @return `true` on success, `false` on I2C error.
   */
  bool setIRDetectionNoise(uint8_t noise);

  /**
   * @fn setScaleResolution
   * @brief Set X/Y output scaling (sensor native grid is 98×98; commonly scaled up, max 4095 per axis).
   * @param x_axis Range 0–4095.
   * @param y_axis Range 0–4095.
   * @return `true` on success, `false` on I2C error.
   */
  bool setScaleResolution(uint16_t x_axis, uint16_t y_axis);

  /**
   * @fn setMaxTrackingNumber
   * @brief Set the maximum number of IR spots to track (default often `0x10` = 16).
   * @param num Range 1–16.
   * @return `true` on success, `false` on I2C error.
   */
  bool setMaxTrackingNumber(uint8_t num);

  /**
   * @fn setExposure
   * @brief Write 16-bit exposure, commit it, and wait a fixed 100 ms.
   * @param expo Range 100–65535. Values below 100 are clamped to 100; effective maximum depends on frame period.
   * @return `true` if both writes succeed, `false` on I2C error. The wait is skipped when a write fails.
   */
  bool setExposure(uint16_t expo);

  /**
   * @fn getExposure
   * @brief Read current exposure from the sensor readback registers (16-bit).
   * @param expo Output reference for the value.
   * @return `true` on success, `false` on I2C error.
   */
  bool getExposure(uint16_t &expo);

  /**
   * @fn setI2CAddress
   * @brief Save a new 7-bit I2C address to bridge flash; it takes effect after reset or power cycle.
   * @param addr7bit New 7-bit address, valid range 0x08–0x77.
   * @return `true` if the address is saved, `false` on invalid address, I2C error, flash failure, or timeout.
   */
  bool setI2CAddress(uint8_t addr7bit);

  /**
   * @fn getI2CAddress
   * @brief Read the current active 7-bit I2C address from the bridge.
   * @param addr7bit Output reference for the active 7-bit address.
   * @return `true` on success, `false` on I2C error.
   */
  bool getI2CAddress(uint8_t &addr7bit);

  /**
   * @fn setIndicator
   * @brief Set turn on/off the indicator of the sensor
   * @param status `true` to turn on, `false` to turn off.
   * @return: whether set indicator on/off success
   */
  bool setIndicator(bool status);

  /**
   * @fn getIRCoordinate
   * @brief Get the center coordinates of the IR spot from the last `update()` frame.
   * @param IRnum Index 0–15 for the N-th spot.
   * @return Pointer to a 4-byte buffer `[xL, xH(4 LSB), yL, yH(4 LSB)]` (12-bit X/Y each). Valid until the next call that uses `_coordBuf`.
   */
  uint8_t *getIRCoordinate(int IRnum);

  /**
   * @fn getIRBound
   * @brief Get boundary box of the IR spot (native 0–98 range per side).
   * @param IRnum Index 0–15.
   * @return Pointer to `[Up, Down, Left, Right]`. Valid until the next call that uses `_boundBuf`.
   */
  uint8_t *getIRBound(int IRnum);

  /**
   * @fn getAverageBrightness
   * @brief Average brightness of the selected spot in the last frame.
   * @param IRnum Index 0–15.
   * @return Brightness byte, or 0 if the index is invalid.
   */
  uint8_t getAverageBrightness(int IRnum);

  /**
   * @fn getMaxBrightness
   * @brief Peak brightness of the selected spot in the last frame.
   * @param IRnum Index 0–15.
   * @return Brightness byte, or 0 if the index is invalid.
   */
  uint8_t getMaxBrightness(int IRnum);

  /**
   * @fn getRadius
   * @brief Radius (pixels) of the selected spot from the last frame.
   * @param IRnum Index 0–15.
   * @return Radius, or 0 if the index is invalid.
   */
  uint8_t getRadius(int IRnum);

private:
  /** Register map, same address space as the bridge firmware. Kept inside the class so sketch code does not see these names. */
  static const uint8_t  I2C_ADDR             = 0x58;    ///< 7-bit I2C slave address
  static const uint16_t EXPOSURE_MIN         = 100;     ///< Minimum B_expo[15:0]; datasheet minimum is 20 us
  static const uint16_t EXPOSURE_APPLY_MS    = 100;     ///< Fixed wait after a successful exposure commit
  static const uint16_t I2C_ADDR_SAVE_MAX_MS = 200;     ///< Stop waiting once the save leaves pending, or after this long

  static const uint16_t REG_FRAME_LEN    = 0x10;
  static const uint8_t  REG_FRAME_COUNT  = 16;
  static const uint16_t REG_FRAMES_START = 0x00;
  static const uint16_t REG_FRAMES_END   = 0xFF;
  static const uint16_t REG_FRAME_SIZE   = REG_FRAME_LEN * REG_FRAME_COUNT;    ///< 256 bytes

  static const uint16_t REG_PARAM_BASE = 0x100;
  static const uint16_t REG_PARAM_LEN  = 30;
  static const uint16_t REG_TOTAL_SIZE = REG_FRAME_SIZE + REG_PARAM_LEN;    ///< 286 bytes total mapped span

  /** Parameter offsets from `REG_PARAM_BASE` (must match firmware `reg.h`). */
  static const uint16_t REG_BRIGHTNESS_THRES        = 0x00;    ///< Sensor reg 0x47, bank 0x0c
  static const uint16_t REG_NOISE_THRES             = 0x01;    ///< reg 0x0F, bank 0x00
  static const uint16_t REG_AREA_THRES_MAX_LSB      = 0x02;    ///< reg 0x0b, bank 0x00
  static const uint16_t REG_AREA_THRES_MAX_MSB      = 0x03;    ///< reg 0x0c, bank 0x00
  static const uint16_t REG_AREA_THRES_MIN          = 0x04;    ///< reg 0x46, bank 0x0c
  static const uint16_t REG_SCALE_RESOLUTION_X_LSB  = 0x05;    ///< reg 0x60, bank 0x0c
  static const uint16_t REG_SCALE_RESOLUTION_X_8_11 = 0x06;    ///< reg 0x61, bank 0x0c
  static const uint16_t REG_SCALE_RESOLUTION_Y_LSB  = 0x07;    ///< reg 0x62, bank 0x0c
  static const uint16_t REG_SCALE_RESOLUTION_Y_8_11 = 0x08;    ///< reg 0x63, bank 0x0c
  static const uint16_t REG_OBJECT_LABELLING        = 0x09;    ///< reg 0x12, bank 0x00
  static const uint16_t REG_OBJECT_NUM              = 0x10;    ///< reg 0x19, bank 0x00
  static const uint16_t REG_ORIRNTATION_RATIO       = 0x11;    ///< reg 0x10, bank 0x00
  static const uint16_t REG_W_B_EXPO_LSB            = 0x12;    ///< reg 0x0f, bank 0x0c
  static const uint16_t REG_W_B_EXPO_MSB            = 0x13;    ///< reg 0x10, bank 0x0c
  static const uint16_t REG_R_B_EXPO_LSB            = 0x14;    ///< reg 0x0e, bank 0x01
  static const uint16_t REG_R_B_EXPO_MSB            = 0x15;    ///< reg 0x0f, bank 0x01
  static const uint16_t REG_W_B_EXPO_COMMIT         = 0x16;    ///< Write 0x01 to commit exposure to bank1
  static const uint16_t REG_I2C_ADDR_PENDING        = 0x17;    ///< Pending 7-bit I2C address to save
  static const uint16_t REG_I2C_ADDR_COMMIT         = 0x18;    ///< Write 0xA5 to save pending address to flash
  static const uint16_t REG_I2C_ADDR_STATUS         = 0x19;    ///< Address save status
  static const uint16_t REG_I2C_ADDR_ACTIVE         = 0x1A;    ///< Current active 7-bit I2C address, read-only
  static const uint16_t REG_INDICATOR               = 0x1B;    ///< Indicator LED: write 0 off, non-zero on; reads back 0/1

  static const uint8_t REG_I2C_ADDR_COMMIT_KEY        = 0xA5;
  static const uint8_t REG_I2C_ADDR_STATUS_OK         = 0x00;
  static const uint8_t REG_I2C_ADDR_STATUS_INVALID    = 0x01;
  static const uint8_t REG_I2C_ADDR_STATUS_FLASH_FAIL = 0x02;
  static const uint8_t REG_I2C_ADDR_STATUS_PENDING    = 0x03;

  /**
   * @brief One tracked object, 16 bytes (matches PAJ7025R2 `regbank05_t`).
   */
  struct __attribute__((packed)) ObjData {
    uint8_t area_07_00;    ///< Area[7:0]
    uint8_t area_13_08;    ///< Area[13:8], valid in lower 6 bits
    uint8_t cx_07_00;      ///< Cx[7:0]
    uint8_t cx_11_08;      ///< Cx[11:8], valid in lower 4 bits
    uint8_t cy_07_00;      ///< Cy[7:0]
    uint8_t cy_11_08;      ///< Cy[11:8], valid in lower 4 bits
    uint8_t avg_brightness;
    uint8_t max_brightness;
    uint8_t radius_range;     ///< bits[3:0]=radius, bits[7:4]=range
    uint8_t boundary_left;    ///< bits[6:0]
    uint8_t boundary_right;
    uint8_t boundary_up;
    uint8_t boundary_down;
    uint8_t aspect_ratio;
    uint8_t vx;
    uint8_t vy;
  };

  /**
   * @fn objectCx
   * @brief Extract 12-bit center X from packed object fields.
   */
  static uint16_t objectCx(const ObjData &obj)
  {
    return (uint16_t)obj.cx_07_00 | ((uint16_t)(obj.cx_11_08 & 0x0F) << 8);
  }

  /**
   * @fn objectCy
   * @brief Extract 12-bit center Y from packed object fields.
   */
  static uint16_t objectCy(const ObjData &obj)
  {
    return (uint16_t)obj.cy_07_00 | ((uint16_t)(obj.cy_11_08 & 0x0F) << 8);
  }

  /**
   * @fn objectRadius
   * @brief Extract 4-bit radius from `radius_range`.
   */
  static uint8_t objectRadius(const ObjData &obj)
  {
    return obj.radius_range & 0x0F;
  }

  /**
   * @fn readReg
   * @brief Read a contiguous block from the bridge register space (16-bit address, little-endian on the wire).
   * @param reg Start register address.
   * @param data Destination buffer.
   * @param len Number of bytes to read.
   * @return `0` on success, `-1` on error.
   */
  int readReg(uint16_t reg, void *data, uint16_t len);

  /**
   * @fn writeReg
   * @brief Write a contiguous block to the bridge register space.
   * @param reg Start register address.
   * @param data Source buffer.
   * @param len Number of bytes to write.
   * @return `0` on success, `-1` on error.
   */
  int writeReg(uint16_t reg, const void *data, uint16_t len);

  /**
   * @fn updateFrame
   * @brief Fetch the full object frame into `_frame`.
   * @return `true` if `readReg` succeeds for `REG_FRAME_SIZE` bytes.
   */
  bool updateFrame();

  /**
   * @fn readObject
   * @brief Copy one object slot from `_frame` into `obj`.
   * @param index Object index 0–15.
   * @param obj Output structure.
   * @return `true` if `index` is in range.
   */
  bool readObject(int index, ObjData &obj);

  /**
   * @fn getI2CAddressStatus
   * @brief Read the last I2C address save status.
   * @param status Output status: `0x00` OK, `0x01` invalid address/commit, `0x02` flash failure, `0x03` pending.
   * @return `true` on success, `false` on I2C error.
   */
  bool getI2CAddressStatus(uint8_t &status);

  TwoWire *_pWire;       ///< I2C bus used for transfers
  uint8_t  _addr7bit;    ///< 7-bit bridge address

  uint8_t _frame[REG_FRAME_SIZE];    ///< Cached frame: 16 × 16 bytes
  uint8_t _coordBuf[4];              ///< Scratch for `getIRCoordinate()` return buffer
  uint8_t _boundBuf[4];              ///< Scratch for `getIRBound()` return buffer
};

#endif
