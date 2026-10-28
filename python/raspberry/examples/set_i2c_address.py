# -*- coding: utf-8 -*
'''!
@file  set_i2c_address.py
@brief This example saves a new bridge I2C address to flash.
@n     The new address takes effect after reset or power cycle.
@copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
@license     The MIT License (MIT)
@author      [thdyyl](yuanlong.yu@dfrobot.com)
@version     V1.0.0
@date        2026-10-26
@url         https://github.com/DFRobot/DFRobot_IRPositionV2
'''

from __future__ import print_function
import sys
import os

sys.path.append("../")
import time

from DFRobot_IRPositionV2 import *

'''
  CURRENT_ADDR -> address the module responds to now (factory default 0x58)
  NEW_ADDR     -> address to save, valid range 0x08-0x77
'''
I2C_1 = 0x01
CURRENT_ADDR = 0x58
NEW_ADDR = 0x59

ir_position = DFRobot_IRPositionV2(I2C_1, CURRENT_ADDR)


def setup():
  while ir_position.begin() == False:
    print("IRPositionV2 init failed. Use the current address and check wiring.")
    time.sleep(1)
  print("IRPositionV2 init OK.")

  active_addr = ir_position.get_i2c_address()
  if active_addr >= 0:
    print("Current active 7-bit address: 0x%02X" % active_addr)

  print("Saving new 7-bit address: 0x%02X" % NEW_ADDR)
  if ir_position.set_i2c_address(NEW_ADDR):
    print("Address saved. Reset or power-cycle the module.")
    print("After reset, use 7-bit address: 0x%02X" % NEW_ADDR)
  else:
    print("Address save failed. Check the address range, wiring, and bridge firmware.")


if __name__ == "__main__":
  setup()
