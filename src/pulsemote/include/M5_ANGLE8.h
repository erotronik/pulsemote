/*
Original source: https://github.com/m5stack/M5Unit-8Angle

Copyright notice for this file:

MIT License

Copyright (c) 2022 M5Stack

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Modifications licensed under project Apache2 license.
*/

#ifndef _M5_ANGLE8_H_
#define _M5_ANGLE8_H_

#include <Arduino.h>
#include <Wire.h>
#include "pins_arduino.h"

#define ANGLE8_I2C_ADDR             0x43
#define ANGLE8_ANALOG_INPUT_12B_REG 0x00
#define ANGLE8_ANALOG_INPUT_8B_REG  0x10
#define ANGLE8_DIGITAL_INPUT_REG    0x20
#define ANGLE8_RGB_24B_REG          0x30

#define ANGLE8_FW_VERSION_REG 0xFE
#define ANGLE8_ADDRESS_REG    0xFF

#define ANGLE8_TOTAL_LED 9
#define ANGLE8_TOTAL_ADC 8

typedef enum { _8bit = 0, _12bit } angle8_analog_read_mode_t;

class M5_ANGLE8 {
   private:
    uint8_t _addr;
    TwoWire *_wire;
    uint8_t _sda;
    uint8_t _scl;
    bool writeBytes(uint8_t addr, uint8_t reg, uint8_t *buffer, uint8_t length);
    bool readBytes(uint8_t addr, uint8_t reg, uint8_t *buffer, uint8_t length);

   public:
    bool begin(uint8_t addr = ANGLE8_I2C_ADDR);
    bool setDeviceAddr(uint8_t addr);
    bool setLEDColor(uint8_t ch, uint32_t color, uint8_t bright);
    bool getDigitalInput();
    uint16_t getAnalogInput(uint8_t ch, angle8_analog_read_mode_t bit = _8bit);
    uint8_t getDialPercent(uint8_t channel, bool reverse);
    uint8_t getVersion();
};

#endif
