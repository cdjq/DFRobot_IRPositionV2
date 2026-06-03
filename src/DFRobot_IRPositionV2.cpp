/*!
 * @file DFRobot_IRPositionV2.cpp
 * @brief Implementation of class DFRobot_IRPositionV2 (I2C register access and object parsing).
 * @details Implements chunked read/write for AVR and faster MCUs, frame caching, and accessors for coordinates and metrics.
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @License     The MIT License (MIT)
 * @author [thdyyl](yuanlong.yu@dfrobot.com)
 * @version  V1.0
 * @date  2026-04-13
 * @url         https://github.com/DFRobot/DFRobot_IRPositionV2
 */

#include "DFRobot_IRPositionV2.h"

// ============ Class: DFRobot_IRPositionV2 ============

static bool irpos_v2_isValidI2CAddress(uint8_t addr7bit)
{
  return (addr7bit >= 0x08 && addr7bit <= 0x77);
}

/**
 * @fn DFRobot_IRPositionV2::DFRobot_IRPositionV2
 * @brief Default constructor of DFRobot_IRPositionV2 class
 * @details Uses `Wire` and default 7-bit address `IRPOS_V2_I2C_ADDR`.
 */
DFRobot_IRPositionV2::DFRobot_IRPositionV2()
: _pWire(&Wire),
  _addr7bit(IRPOS_V2_I2C_ADDR)
{
}

/**
 * @fn DFRobot_IRPositionV2::DFRobot_IRPositionV2
 * @brief Overloaded constructor of DFRobot_IRPositionV2 class
 * @details Stores the 7-bit I2C address used by `Wire`.
 * @param pWire Pointer to `TwoWire` (NULL uses `Wire`).
 * @param addr7bit 7-bit I2C address.
 */
DFRobot_IRPositionV2::DFRobot_IRPositionV2(TwoWire *pWire, uint8_t addr7bit)
: _pWire(pWire ? pWire : &Wire),
  _addr7bit(addr7bit)
{
}

/**
 * @fn DFRobot_IRPositionV2::~DFRobot_IRPositionV2
 * @brief Destructor of DFRobot_IRPositionV2 class
 */
DFRobot_IRPositionV2::~DFRobot_IRPositionV2()
{
}

/**
 * @fn DFRobot_IRPositionV2::begin
 * @brief Initialize I2C and verify the bridge responds
 * @return `true` if a read from the frame start succeeds, otherwise `false`
 */
bool DFRobot_IRPositionV2::begin()
{
  _pWire->begin();
  _pWire->setClock(400000);

  uint8_t dummy = 0;
  return (readReg(REG_FRAMES_START, &dummy, 1) == 0);
}

/**
 * @fn DFRobot_IRPositionV2::update
 * @brief Read the full object frame into the internal buffer
 * @return Result of `updateFrame()`
 */
bool DFRobot_IRPositionV2::update()
{
  return updateFrame();
}

/**
 * @fn DFRobot_IRPositionV2::readReg
 * @brief Read from the bridge with repeated-start and chunked `requestFrom`
 * @param reg 16-bit register address (LSB first on the wire).
 * @param data Output buffer.
 * @param len Number of bytes.
 * @return `0` on success, `-1` on invalid argument or I2C failure.
 */
int DFRobot_IRPositionV2::readReg(uint16_t reg, void *data, uint16_t len)
{
  if (data == NULL || len == 0) {
    return -1;
  }

  _pWire->beginTransmission(_addr7bit);
  _pWire->write((uint8_t)(reg & 0xFF));
  _pWire->write((uint8_t)((reg >> 8) & 0xFF));
  uint8_t err = _pWire->endTransmission(false); // repeated start
  if (err != 0) {
    return -1;
  }

  uint8_t *p = (uint8_t *)data;
  uint16_t remaining = len;

  while (remaining > 0) {
#if defined(ARDUINO_ARCH_AVR)
    uint8_t chunk = remaining > 32 ? 32 : (uint8_t)remaining;
#else
    uint8_t chunk = remaining > 128 ? 128 : (uint8_t)remaining;
#endif
    uint8_t got = _pWire->requestFrom(_addr7bit, chunk, (uint8_t)true);
    if (got == 0) {
      return -1;
    }
    for (uint8_t i = 0; i < got && remaining > 0; i++) {
      *p++ = _pWire->read();
      remaining--;
    }
    if (got < chunk && remaining > 0) {
      return -1;
    }
  }

  return 0;
}

/**
 * @fn DFRobot_IRPositionV2::writeReg
 * @brief Write to the bridge, splitting into chunks that fit the Wire buffer
 * @param reg Start address; advanced by each chunk for multi-byte writes.
 * @param data Source bytes.
 * @param len Total length.
 * @return `0` on success, `-1` on invalid argument or I2C failure.
 */
int DFRobot_IRPositionV2::writeReg(uint16_t reg, const void *data, uint16_t len)
{
  if (data == NULL || len == 0) {
    return -1;
  }

  const uint8_t *p = (const uint8_t *)data;
  uint16_t remaining = len;

  while (remaining > 0) {
    _pWire->beginTransmission(_addr7bit);
    _pWire->write((uint8_t)(reg & 0xFF));
    _pWire->write((uint8_t)((reg >> 8) & 0xFF));

#if defined(ARDUINO_ARCH_AVR)
    uint8_t space = 32 - 2;
#else
    uint8_t space = 128 - 2;
#endif
    uint8_t chunk = remaining > space ? space : (uint8_t)remaining;

    for (uint8_t i = 0; i < chunk; i++) {
      _pWire->write(*p++);
    }

    uint8_t err = _pWire->endTransmission(true);
    if (err != 0) {
      return -1;
    }

    remaining -= chunk;
    reg += chunk;
  }

  return 0;
}

/**
 * @fn DFRobot_IRPositionV2::setIRBrightnessThreshold
 * @brief Write brightness threshold parameter
 * @param brightness Value 0–255.
 * @return `true` if `writeReg` succeeds.
 */
bool DFRobot_IRPositionV2::setIRBrightnessThreshold(uint8_t brightness)
{
  uint16_t reg = REG_PARAM_BASE + REG_BRIGHTNESS_THRES;
  return (writeReg(reg, &brightness, 1) == 0);
}

/**
 * @fn DFRobot_IRPositionV2::setIRDetectionNoise
 * @brief Write noise threshold parameter
 * @param noise Value 0–255.
 * @return `true` if `writeReg` succeeds.
 */
bool DFRobot_IRPositionV2::setIRDetectionNoise(uint8_t noise)
{
  uint16_t reg = REG_PARAM_BASE + REG_NOISE_THRES;
  return (writeReg(reg, &noise, 1) == 0);
}

/**
 * @fn DFRobot_IRPositionV2::setScaleResolution
 * @brief Write X/Y scale (clamped to 4095 per axis)
 * @param x_axis X scale 0–4095.
 * @param y_axis Y scale 0–4095.
 * @return `true` if `writeReg` succeeds.
 */
bool DFRobot_IRPositionV2::setScaleResolution(uint16_t x_axis, uint16_t y_axis)
{
  if (x_axis > 4095)
    x_axis = 4095;
  if (y_axis > 4095)
    y_axis = 4095;

  uint8_t buf[4];
  buf[0] = (uint8_t)(x_axis & 0xFF);
  buf[1] = (uint8_t)((x_axis >> 8) & 0x0F);
  buf[2] = (uint8_t)(y_axis & 0xFF);
  buf[3] = (uint8_t)((y_axis >> 8) & 0x0F);

  uint16_t reg = REG_PARAM_BASE + REG_SCALE_RESOLUTION_X_LSB;
  return (writeReg(reg, buf, 4) == 0);
}

/**
 * @fn DFRobot_IRPositionV2::setMaxTrackingNumber
 * @brief Clamp and write maximum tracked object count
 * @param num Desired count, clamped to 1–16.
 * @return `true` if `writeReg` succeeds.
 */
bool DFRobot_IRPositionV2::setMaxTrackingNumber(uint8_t num)
{
  if (num < 1)
    num = 1;
  if (num > 16)
    num = 16;

  uint16_t reg = REG_PARAM_BASE + REG_OBJECT_NUM;
  return (writeReg(reg, &num, 1) == 0);
}

/**
 * @fn DFRobot_IRPositionV2::setExposure
 * @brief Write exposure, commit, and optional delay for readback
 * @param expo 16-bit exposure value, clamped to the datasheet minimum of 100.
 * @param settleMs Milliseconds to wait after commit (0 skips `delay`).
 * @return `true` if both writes succeed.
 */
bool DFRobot_IRPositionV2::setExposure(uint16_t expo, uint16_t settleMs)
{
  if (expo < IRPOS_V2_EXPOSURE_MIN) {
    expo = IRPOS_V2_EXPOSURE_MIN;
  }

  uint8_t expoBuf[2];
  expoBuf[0] = (uint8_t)(expo & 0xFF);         // LSB
  expoBuf[1] = (uint8_t)((expo >> 8) & 0xFF); // MSB

  uint16_t regExpo = REG_PARAM_BASE + REG_W_B_EXPO_LSB;
  if (writeReg(regExpo, expoBuf, 2) != 0) {
    return false;
  }

  uint8_t commit = 0x01;
  uint16_t regCommit = REG_PARAM_BASE + REG_W_B_EXPO_COMMIT;
  if (writeReg(regCommit, &commit, 1) != 0) {
    return false;
  }

  /* Sensor readback registers may update with a short delay after commit. */
  if (settleMs > 0) {
    delay(settleMs);
  }
  return true;
}

/**
 * @fn DFRobot_IRPositionV2::getExposure
 * @brief Read 16-bit exposure from readback registers
 * @param expo Output reference for the value.
 * @return `true` if two bytes are read successfully.
 */
bool DFRobot_IRPositionV2::getExposure(uint16_t &expo)
{
  uint8_t expoBuf[2] = { 0 };
  uint16_t regExpo = REG_PARAM_BASE + REG_R_B_EXPO_LSB;
  if (readReg(regExpo, expoBuf, 2) != 0) {
    return false;
  }

  expo = (uint16_t)expoBuf[0] | ((uint16_t)expoBuf[1] << 8);
  return true;
}

/**
 * @fn DFRobot_IRPositionV2::setI2CAddress
 * @brief Save a new 7-bit bridge address to flash; active after reset or power cycle
 * @param addr7bit New 7-bit address, valid range 0x08–0x77.
 * @param settleMs Maximum milliseconds to wait for the save status to leave pending.
 * @return `true` if the bridge reports save status OK.
 */
bool DFRobot_IRPositionV2::setI2CAddress(uint8_t addr7bit, uint16_t settleMs)
{
  if (!irpos_v2_isValidI2CAddress(addr7bit)) {
    return false;
  }

  uint8_t buf[2];
  buf[0] = addr7bit;
  buf[1] = REG_I2C_ADDR_COMMIT_KEY;

  uint16_t reg = REG_PARAM_BASE + REG_I2C_ADDR_PENDING;
  if (writeReg(reg, buf, 2) != 0) {
    return false;
  }

  uint8_t status = REG_I2C_ADDR_STATUS_FLASH_FAIL;
  uint32_t startMs = millis();
  do {
    if (!getI2CAddressStatus(status)) {
      return false;
    }
    if (status != REG_I2C_ADDR_STATUS_PENDING) {
      break;
    }
    if (settleMs == 0) {
      break;
    }
    delay(5);
  } while ((uint32_t)(millis() - startMs) < settleMs);

  return (status == REG_I2C_ADDR_STATUS_OK);
}

/**
 * @fn DFRobot_IRPositionV2::getI2CAddress
 * @brief Read current active 7-bit bridge address
 * @param addr7bit Output reference.
 * @return `true` if one byte is read successfully.
 */
bool DFRobot_IRPositionV2::getI2CAddress(uint8_t &addr7bit)
{
  uint16_t reg = REG_PARAM_BASE + REG_I2C_ADDR_ACTIVE;
  return (readReg(reg, &addr7bit, 1) == 0);
}

/**
 * @fn DFRobot_IRPositionV2::getI2CAddressStatus
 * @brief Read last address-save status
 * @param status Output status byte.
 * @return `true` if one byte is read successfully.
 */
bool DFRobot_IRPositionV2::getI2CAddressStatus(uint8_t &status)
{
  uint16_t reg = REG_PARAM_BASE + REG_I2C_ADDR_STATUS;
  return (readReg(reg, &status, 1) == 0);
}

/**
 * @fn DFRobot_IRPositionV2::updateFrame
 * @brief Load `REG_FRAME_SIZE` bytes starting at `REG_FRAMES_START`
 * @return `true` if `readReg` returns 0.
 */
bool DFRobot_IRPositionV2::updateFrame()
{
  return (readReg(REG_FRAMES_START, _frame, REG_FRAME_SIZE) == 0);
}

/**
 * @fn DFRobot_IRPositionV2::readObject
 * @brief Copy one 16-byte slot from `_frame` into `obj`
 * @param index Object index 0–15.
 * @param obj Output structure.
 * @return `true` if `index` is in range.
 */
bool DFRobot_IRPositionV2::readObject(int index, IRPosV2_ObjData &obj)
{
  if (index < 0 || index >= REG_FRAME_COUNT) {
    return false;
  }

  const uint8_t *p = &_frame[index * REG_FRAME_LEN];
  memcpy(&obj, p, sizeof(IRPosV2_ObjData));
  return true;
}

/**
 * @fn DFRobot_IRPositionV2::getIRCoordinate
 * @brief Pack 12-bit center X/Y into `_coordBuf` for the given slot
 * @param IRnum Object index 0–15.
 * @return Pointer to `_coordBuf`; cleared on invalid index.
 */
uint8_t *DFRobot_IRPositionV2::getIRCoordinate(int IRnum)
{
  IRPosV2_ObjData obj;
  if (!readObject(IRnum, obj)) {
    _coordBuf[0] = _coordBuf[1] = _coordBuf[2] = _coordBuf[3] = 0;
    return _coordBuf;
  }

  uint16_t cx = irpos_v2_getCx(&obj);
  uint16_t cy = irpos_v2_getCy(&obj);

  _coordBuf[0] = (uint8_t)(cx & 0xFF);
  _coordBuf[1] = (uint8_t)((cx >> 8) & 0x0F);
  _coordBuf[2] = (uint8_t)(cy & 0xFF);
  _coordBuf[3] = (uint8_t)((cy >> 8) & 0x0F);

  return _coordBuf;
}

/**
 * @fn DFRobot_IRPositionV2::getIRBound
 * @brief Return Up/Down/Left/Right boundaries (7 bits each) in `_boundBuf`
 * @param IRnum Object index 0–15.
 * @return Pointer to `_boundBuf`; cleared on invalid index.
 */
uint8_t *DFRobot_IRPositionV2::getIRBound(int IRnum)
{
  IRPosV2_ObjData obj;
  if (!readObject(IRnum, obj)) {
    _boundBuf[0] = _boundBuf[1] = _boundBuf[2] = _boundBuf[3] = 0;
    return _boundBuf;
  }

  uint8_t bL = obj.boundary_left & 0x7F;
  uint8_t bR = obj.boundary_right & 0x7F;
  uint8_t bU = obj.boundary_up & 0x7F;
  uint8_t bD = obj.boundary_down & 0x7F;

  _boundBuf[0] = bU;
  _boundBuf[1] = bD;
  _boundBuf[2] = bL;
  _boundBuf[3] = bR;

  return _boundBuf;
}

/**
 * @fn DFRobot_IRPositionV2::getAverageBrightness
 * @brief Return `avg_brightness` field for the slot
 * @param IRnum Object index 0–15.
 * @return Byte value, or 0 if invalid.
 */
uint8_t DFRobot_IRPositionV2::getAverageBrightness(int IRnum)
{
  IRPosV2_ObjData obj;
  if (!readObject(IRnum, obj)) {
    return 0;
  }
  return obj.avg_brightness;
}

/**
 * @fn DFRobot_IRPositionV2::getMaxBrightness
 * @brief Return `max_brightness` field for the slot
 * @param IRnum Object index 0–15.
 * @return Byte value, or 0 if invalid.
 */
uint8_t DFRobot_IRPositionV2::getMaxBrightness(int IRnum)
{
  IRPosV2_ObjData obj;
  if (!readObject(IRnum, obj)) {
    return 0;
  }
  return obj.max_brightness;
}

/**
 * @fn DFRobot_IRPositionV2::getRadius
 * @brief Return 4-bit radius from `radius_range`
 * @param IRnum Object index 0–15.
 * @return Radius, or 0 if invalid.
 */
uint8_t DFRobot_IRPositionV2::getRadius(int IRnum)
{
  IRPosV2_ObjData obj;
  if (!readObject(IRnum, obj)) {
    return 0;
  }
  return irpos_v2_getRadius(&obj);
}
