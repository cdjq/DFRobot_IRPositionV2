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
#include "DFRobot_IRPositionV2_reg.h"

/**
 * @class DFRobot_IRPositionV2
 * @brief Driver for IR position V2 (PAJ7025R2 + CS32 bridge over I2C).
 */
class DFRobot_IRPositionV2 {
public:
  /**
   * @fn DFRobot_IRPositionV2
   * @brief Default constructor (uses `Wire` and 7-bit address `IRPOS_V2_I2C_ADDR`).
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
   * @brief Write 16-bit exposure, commit, and optionally wait before readback.
   * @param expo Range 100–65535. Values below 100 are clamped to 100; effective maximum depends on frame period.
   * @param settleMs Delay in milliseconds after commit (default 10).
   * @return `true` if writes succeed, `false` on I2C error.
   */
  bool setExposure(uint16_t expo, uint16_t settleMs = 10);

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
   * @param settleMs Maximum milliseconds to wait for the save operation to finish (default 100).
   * @return `true` if the address is saved, `false` on invalid address, I2C error, flash failure, or timeout.
   */
  bool setI2CAddress(uint8_t addr7bit, uint16_t settleMs = 100);

  /**
   * @fn getI2CAddress
   * @brief Read the current active 7-bit I2C address from the bridge.
   * @param addr7bit Output reference for the active 7-bit address.
   * @return `true` on success, `false` on I2C error.
   */
  bool getI2CAddress(uint8_t &addr7bit);

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
  bool readObject(int index, IRPosV2_ObjData &obj);

  /**
   * @fn getI2CAddressStatus
   * @brief Read the last I2C address save status.
   * @param status Output status: `0x00` OK, `0x01` invalid address/commit, `0x02` flash failure, `0x03` pending.
   * @return `true` on success, `false` on I2C error.
   */
  bool getI2CAddressStatus(uint8_t &status);

  TwoWire *_pWire;       ///< I2C bus used for transfers
  uint8_t  _addr7bit;    ///< 7-bit bridge address

  uint8_t _frame[REG_FRAME_SIZE]; ///< Cached frame: 16 × 16 bytes
  uint8_t _coordBuf[4];           ///< Scratch for `getIRCoordinate()` return buffer
  uint8_t _boundBuf[4];           ///< Scratch for `getIRBound()` return buffer
};

#endif
