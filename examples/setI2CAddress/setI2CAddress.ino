/*!
 * @file setI2CAddress.ino
 * @brief Example: save a new bridge I2C address to flash.
 * @n Wiring (I2C, default Wire, 7-bit address 0x58):
 * @n Red → VCC (3.3V or 5V, per the module rating)
 * @n Black → GND
 * @n Green → SCL
 * @n Yellow → SDA
 * @n Serial: 115200
 * @n The new address takes effect after reset or power cycle.
 * @details kCurrentAddr7bit is the address in use now. kNewAddr7bit is written to flash.
 *          Both must be in 0x08–0x77.
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @License     The MIT License (MIT)
 * @author [thdyyl](yuanlong.yu@dfrobot.com)
 * @version  V1.0.0
 * @date  2026-11-07
 * @url         https://github.com/DFRobot/DFRobot_IRPositionV2
 */

#include <Wire.h>

#include "DFRobot_IRPositionV2.h"

/** Current 7-bit address. Default 0x58. Valid range 0x08–0x77. */
static const uint8_t kCurrentAddr7bit = 0x58;
/** New 7-bit address. Default 0x59. Valid range 0x08–0x77. Applied after reset or power cycle. */
static const uint8_t kNewAddr7bit = 0x59;

/* Wiring: Red → VCC, Black → GND, Green → SCL, Yellow → SDA. */
DFRobot_IRPositionV2 irPosition(&Wire, kCurrentAddr7bit);

void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    ;    // USB-Leonardo/M0: wait for Serial
  }

  Serial.println(F("DFRobot IRPosition V2 - Set I2C Address Example"));

  while (!irPosition.begin()) {
    Serial.println(F("IRPositionV2 init failed. Use the current address and check wiring."));
    delay(1000);
  }
  Serial.println(F("IRPositionV2 init OK."));

  uint8_t activeAddr = 0;
  if (irPosition.getI2CAddress(activeAddr)) {
    Serial.print(F("Current active 7-bit address: 0x"));
    Serial.println(activeAddr, HEX);
  }

  Serial.print(F("Saving new 7-bit address: 0x"));
  Serial.println(kNewAddr7bit, HEX);

  if (irPosition.setI2CAddress(kNewAddr7bit)) {
    Serial.println(F("Address saved. Reset or power-cycle the module."));
    Serial.print(F("After reset, use 7-bit address: 0x"));
    Serial.println(kNewAddr7bit, HEX);
  } else {
    Serial.println(F("Address save failed. Check the address range, wiring, and bridge firmware."));
  }
}

void loop() {}
