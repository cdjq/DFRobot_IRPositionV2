/*!
 * @file getPosition.ino
 * @brief Example: read up to 16 IR objects via the CS32 I2C bridge and print coordinates and metrics.
 * @details Demonstrates `begin()`, optional parameter tuning (with valid ranges), `update()` per frame,
 *          and per-object accessors. See README "Configuration parameter ranges" for limits.
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @License     The MIT License (MIT)
 * @author [thdyyl](yuanlong.yu@dfrobot.com)
 * @version  V1.0.0
 * @date  2026-04-13
 * @url         https://github.com/DFRobot/DFRobot_IRPositionV2
 */

#include <Wire.h>
#include "DFRobot_IRPositionV2.h"

/* >> Step 1: Create the driver (default: Wire + 8-bit address 0xB0 → 7-bit 0x58) */
DFRobot_IRPositionV2 irPosition;
// Optional: DFRobot_IRPositionV2 irPosition(&Wire, 0xB0);

/** Demo exposure: 16-bit (range 0–65535). Adjust for your lighting. */
static const uint16_t kDemoExposure = 0x0080;

static uint32_t s_lastExpoPrintMs = 0;

/**
 * @brief Initialize serial, sensor, and optional parameters.
 * @details Parameter comments list valid ranges; values below are examples only.
 */
void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    ; // USB-Leonardo/M0: wait for Serial
  }

  Serial.println(F("DFRobot IRPosition V2 - Get Position Example"));

  if (!irPosition.begin()) {
    Serial.println(F("IRPositionV2 init failed: check wiring, pull-ups, and bridge firmware."));
    while (1) {
      delay(1000);
    }
  }

  /**
   * Optional tuning (see README "Configuration parameter ranges"):
   * - setIRBrightnessThreshold: 0–255 (typ. default 0x97)
   * - setIRDetectionNoise:      0–255 (typ. default 0x0A)
   * - setScaleResolution:       each axis 0–4095 (maps native 98×98; e.g. 2490 ≈ 30× scale)
   * - setMaxTrackingNumber:     1–16
   */
  irPosition.setIRBrightnessThreshold(0xEE);   // 0–255
  irPosition.setIRDetectionNoise(0xFF);        // 0–255
  irPosition.setScaleResolution(2490, 2490);   // x,y each 0–4095
  irPosition.setMaxTrackingNumber(16);          // 1–16

  // setExposure(expo, settleMs): expo 0–65535; settleMs = ms to wait after commit (default 10).
  if (irPosition.setExposure(kDemoExposure, 10)) {
    Serial.print(F("setExposure OK, wrote 0x"));
    Serial.println(kDemoExposure, HEX);
  } else {
    Serial.println(F("setExposure failed (I2C)."));
  }
}

/**
 * @brief Read one frame, print all 16 slots, and periodically print exposure readback.
 */
void loop()
{
  if (!irPosition.update()) {
    Serial.println(F("update() failed — frame read error."));
    delay(500);
    return;
  }

  for (int i = 0; i < 16; i++) {
    // IR index i: 0–15
    uint8_t *coord = irPosition.getIRCoordinate(i);
    uint8_t *bound = irPosition.getIRBound(i);

    uint16_t x = coord[0] | ((uint16_t)(coord[1] & 0x0F) << 8);
    uint16_t y = coord[2] | ((uint16_t)(coord[3] & 0x0F) << 8);

    uint8_t avgB = irPosition.getAverageBrightness(i);
    uint8_t maxB = irPosition.getMaxBrightness(i);
    uint8_t rad  = irPosition.getRadius(i);

    Serial.print(F("Obj["));
    Serial.print(i);
    Serial.print(F("] Cxy=("));
    Serial.print(x);
    Serial.print(F(","));
    Serial.print(y);
    Serial.print(F(") AvgB="));
    Serial.print(avgB);
    Serial.print(F(" MaxB="));
    Serial.print(maxB);
    Serial.print(F(" R="));
    Serial.print(rad);
    Serial.print(F(" Bound(U,D,L,R)=("));
    Serial.print(bound[0]);
    Serial.print(F(","));
    Serial.print(bound[1]);
    Serial.print(F(","));
    Serial.print(bound[2]);
    Serial.print(F(","));
    Serial.print(bound[3]);
    Serial.println(F(")"));
  }

  if (millis() - s_lastExpoPrintMs >= 1000) {
    s_lastExpoPrintMs = millis();
    uint16_t expo = 0;
    if (irPosition.getExposure(expo)) {
      Serial.print(F("Exposure(readback)=0x"));
      Serial.println(expo, HEX);
    } else {
      Serial.println(F("getExposure() failed."));
    }
  }

  Serial.println(F("----------------------------------------------------------------"));
}
