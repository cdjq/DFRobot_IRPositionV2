/*!
 * @file getPosition.ino
 * @brief Example: read up to 16 IR objects and print coordinates and metrics.
 * @n Wiring (I2C, default Wire, 7-bit address 0x58):
 * @n Red → VCC (3.3V or 5V, per the module rating)
 * @n Black → GND
 * @n Green → SCL
 * @n Yellow → SDA
 * @n Serial: 115200
 * @details Demonstrates `begin()`, optional parameter tuning, `update()` per frame, and per-object accessors.
 *          Parameter ranges are in the method comments of `DFRobot_IRPositionV2.h`.
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @License     The MIT License (MIT)
 * @author [thdyyl](yuanlong.yu@dfrobot.com)
 * @version  V1.0.0
 * @date  2026-11-07
 * @url         https://github.com/DFRobot/DFRobot_IRPositionV2
 */

#include "DFRobot_IRPositionV2.h"

/* Wiring: Red → VCC, Black → GND, Green → SCL, Yellow → SDA. Default 7-bit address 0x58 (valid 0x08–0x77). */
DFRobot_IRPositionV2 irPosition(&Wire, 0x58);

/** Demo exposure: B_expo[15:0] (range 100–65535). Adjust for your lighting. */
static const uint16_t kDemoExposure = 0x0080;

/**
 * @brief Initialize serial, sensor, and optional parameters.
 * @details Parameter comments list valid ranges; values below are examples only.
 */
void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    ;    // USB-Leonardo/M0: wait for Serial
  }

  Serial.println(F("DFRobot IRPosition V2 - Get Position Example"));

  while (!irPosition.begin()) {
    Serial.println(F("IRPositionV2 init failed: check wiring, pull-ups, and bridge firmware."));
    delay(1000);
  }
  Serial.println(F("IRPositionV2 init OK."));

  irPosition.setIndicator(false);
  /**
   * Optional tuning (ranges are in the method comments):
   * - setIRBrightnessThreshold: 0–255 (typ. default 0x97). Higher drops dimmer spots.
   * - setIRDetectionNoise:      0–255 (typ. default 0x0A)
   * - setScaleResolution:       each axis 0–4095 (maps native 98×98; this example uses 2490, 2490)
   * - setMaxTrackingNumber:     1–16
   */
  irPosition.setIRBrightnessThreshold(0xEE);    // 0–255
  irPosition.setIRDetectionNoise(0xFF);         // 0–255
  irPosition.setScaleResolution(2490, 2490);    // x, y each 0–4095
  irPosition.setMaxTrackingNumber(16);          // 1–16

  // setExposure(expo): expo 100–65535. Waits a fixed 100 ms after a successful write.
  if (irPosition.setExposure(kDemoExposure)) {
    Serial.print(F("setExposure OK, wrote 0x"));
    Serial.println(kDemoExposure, HEX);
  } else {
    Serial.println(F("setExposure failed (I2C)."));
  }
  uint16_t expo = 0;
  if (irPosition.getExposure(expo)) {
    Serial.print(F("Exposure(readback)=0x"));
    Serial.println(expo, HEX);
  } else {
    Serial.println(F("getExposure() failed."));
  }
}

/**
 * @brief Read one frame and print only slots that hold a detected spot.
 */
void loop()
{
  if (!irPosition.update()) {
    Serial.println(F("update() failed — frame read error."));
    delay(500);
    return;
  }

  int spotCount = 0;
  for (int i = 0; i < 16; i++) {
    // IR index i: 0–15
    uint8_t *coord = irPosition.getIRCoordinate(i);
    uint8_t *bound = irPosition.getIRBound(i);

    uint16_t x = coord[0] | ((uint16_t)(coord[1] & 0x0F) << 8);
    uint16_t y = coord[2] | ((uint16_t)(coord[3] & 0x0F) << 8);

    uint8_t avgB = irPosition.getAverageBrightness(i);
    uint8_t maxB = irPosition.getMaxBrightness(i);
    uint8_t rad  = irPosition.getRadius(i);

    // Empty slots report zero brightness. Each axis is checked against the scale passed above.
    if (avgB == 0 || x > 2490 || y > 2490) {
      continue;
    }
    spotCount++;

    Serial.print(F("Obj["));
    Serial.print(i);
    Serial.print(F("] Cxy(scaled)=("));
    Serial.print(x);
    Serial.print(F(","));
    Serial.print(y);
    Serial.print(F(") AvgB(0-255)="));
    Serial.print(avgB);
    Serial.print(F(" MaxB(0-255)="));
    Serial.print(maxB);
    Serial.print(F(" R(px approx)="));
    Serial.print(rad);
    Serial.print(F(" Bound(U,D,L,R native 0-98)=("));
    Serial.print(bound[0]);
    Serial.print(F(","));
    Serial.print(bound[1]);
    Serial.print(F(","));
    Serial.print(bound[2]);
    Serial.print(F(","));
    Serial.print(bound[3]);
    Serial.println(F(")"));
  }

  if (spotCount == 0) {
    Serial.println(F("No IR spot."));
  }
  Serial.println(F("----------------------------------------------------------------"));
  delay(200);
}
