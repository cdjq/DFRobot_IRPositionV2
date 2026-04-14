/*!
 * @file DFRobot_IRPositionV2_reg.h
 * @brief Register map and packed object layout for the CS32 ↔ PAJ7025R2 bridge.
 * @details Exposes the same address space as firmware `reg.h`: frame dump at 0x0000 and parameter mirror at 0x0100.
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @License     The MIT License (MIT)
 * @author [thdyyl](yuanlong.yu@dfrobot.com)
 * @version  V1.0
 * @date  2026-04-13
 * @url         https://github.com/DFRobot/DFRobot_IRPositionV2
 */

#ifndef __DFROBOT_IRPOSITIONV2_REG_H
#define __DFROBOT_IRPOSITIONV2_REG_H

#include <stdint.h>

/** 7-bit I2C slave address (firmware uses 0x58; Arduino 8-bit form 0xB0 >> 1). */
#define IRPOS_V2_I2C_ADDR 0x58

/** Frame region: 16 objects × 16 bytes, read-only from host. */
#define REG_FRAME_LEN    0x10
#define REG_FRAME_COUNT  16
#define REG_FRAMES_START 0x00
#define REG_FRAMES_END   0xFF
#define REG_FRAME_SIZE   (REG_FRAME_LEN * REG_FRAME_COUNT) /**< 256 bytes */

/** Parameter mirror base (read/write), length must match firmware. */
#define REG_PARAM_BASE 0x100
#define REG_PARAM_LEN  30
#define REG_TOTAL_SIZE (REG_FRAME_SIZE + REG_PARAM_LEN) /**< 286 bytes total mapped span */

/** Parameter offsets from `REG_PARAM_BASE` (must match firmware `reg.h`). */
#define REG_BRIGHTNESS_THRES       0x00 /**< Sensor reg 0x47, bank 0x0c */
#define REG_NOISE_THRES            0x01 /**< reg 0x0F, bank 0x00 */
#define REG_AREA_THRES_MAX_LSB     0x02 /**< reg 0x0b, bank 0x00 */
#define REG_AREA_THRES_MAX_MSB     0x03 /**< reg 0x0c, bank 0x00 */
#define REG_AREA_THRES_MIN         0x04 /**< reg 0x46, bank 0x0c */
#define REG_SCALE_RESOLUTION_X_LSB 0x05 /**< reg 0x60, bank 0x0c */
#define REG_SCALE_RESOLUTION_X_8_11 0x06 /**< reg 0x61, bank 0x0c */
#define REG_SCALE_RESOLUTION_Y_LSB 0x07 /**< reg 0x62, bank 0x0c */
#define REG_SCALE_RESOLUTION_Y_8_11 0x08 /**< reg 0x63, bank 0x0c */
#define REG_OBJECT_LABELLING       0x09 /**< reg 0x12, bank 0x00 */
#define REG_OBJECT_NUM             0x10 /**< reg 0x19, bank 0x00 */
#define REG_ORIRNTATION_RATIO      0x11 /**< reg 0x10, bank 0x00 */
#define REG_W_B_EXPO_LSB           0x12 /**< reg 0x0f, bank 0x0c */
#define REG_W_B_EXPO_MSB           0x13 /**< reg 0x10, bank 0x0c */
#define REG_R_B_EXPO_LSB           0x14 /**< reg 0x0e, bank 0x01 */
#define REG_R_B_EXPO_MSB           0x15 /**< reg 0x0f, bank 0x01 */
#define REG_W_B_EXPO_COMMIT        0x16 /**< Write 0x01 to commit exposure to bank1 */

/**
 * @brief One tracked object, 16 bytes (matches PAJ7025R2 `regbank05_t`).
 */
struct __attribute__((packed)) IRPosV2_ObjData {
  uint8_t area_07_00;   ///< Area[7:0]
  uint8_t area_13_08;   ///< Area[13:8], valid in lower 6 bits
  uint8_t cx_07_00;     ///< Cx[7:0]
  uint8_t cx_11_08;     ///< Cx[11:8], valid in lower 4 bits
  uint8_t cy_07_00;     ///< Cy[7:0]
  uint8_t cy_11_08;     ///< Cy[11:8], valid in lower 4 bits
  uint8_t avg_brightness;
  uint8_t max_brightness;
  uint8_t radius_range; ///< bits[3:0]=radius, bits[7:4]=range
  uint8_t boundary_left;   ///< bits[6:0]
  uint8_t boundary_right;
  uint8_t boundary_up;
  uint8_t boundary_down;
  uint8_t aspect_ratio;
  uint8_t vx;
  uint8_t vy;
};

/**
 * @fn irpos_v2_getCx
 * @brief Extract 12-bit center X from packed object fields.
 */
static inline uint16_t irpos_v2_getCx(const IRPosV2_ObjData *obj)
{
  return (uint16_t)obj->cx_07_00 | ((uint16_t)(obj->cx_11_08 & 0x0F) << 8);
}

/**
 * @fn irpos_v2_getCy
 * @brief Extract 12-bit center Y from packed object fields.
 */
static inline uint16_t irpos_v2_getCy(const IRPosV2_ObjData *obj)
{
  return (uint16_t)obj->cy_07_00 | ((uint16_t)(obj->cy_11_08 & 0x0F) << 8);
}

/**
 * @fn irpos_v2_getRadius
 * @brief Extract 4-bit radius from `radius_range`.
 */
static inline uint8_t irpos_v2_getRadius(const IRPosV2_ObjData *obj)
{
  return obj->radius_range & 0x0F;
}

#endif /* __DFROBOT_IRPOSITIONV2_REG_H */
