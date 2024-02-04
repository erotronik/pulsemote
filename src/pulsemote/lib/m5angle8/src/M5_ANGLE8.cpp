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

#include "M5_ANGLE8.hpp"

/*! @brief Initialize the ANGLE8.
    @return True if the init was successful, otherwise false.. */
bool M5_ANGLE8::begin() {
    auto ad = readRegister8(ANGLE8_ADDRESS_REG);
    ESP_LOGD("m5angle8", "Read address %u", ad.value_or(0));
    if ( ad.has_value() && (ad.value_or(0) == _addr) )
        return true;
    return false;
}

/*! @brief Write a certain length of data to the specified register address.
    @return True if the write was successful, otherwise false.. */
bool M5_ANGLE8::writeRegister(uint8_t reg, uint8_t *buffer, uint8_t length) {
    bool res = true;
    res |= _i2c->start(_addr, false, _freq);
    res |= _i2c->write(reg);
    res |= _i2c->write(buffer, 4);
    res |= _i2c->stop();
    return res;
}

/*! @brief Read a certain length of data to the specified register address.
    @return True if the write was successful, otherwise false. */
bool M5_ANGLE8::readRegister(uint8_t reg, uint8_t *buffer, uint8_t length) {
    bool res = true;
    res |= _i2c->start(_addr, false, _freq);
    res |= _i2c->write(reg);
    res |= _i2c->stop();
    if (!res) {
        ESP_LOGD("m5angle8", "Write: register address write failed");
        return {};
    }
    res |= _i2c->start(_addr, true, _freq);
    res |= _i2c->read(buffer, length);
    res |= _i2c->stop();
    return res;
}

std::optional<uint8_t> M5_ANGLE8::readRegister8(uint8_t reg) {
    bool res = true;
    uint8_t r;
    res |= _i2c->start(_addr, false, _freq);
    res |= _i2c->write(reg);
    res |= _i2c->stop();
    if (!res) {
        ESP_LOGD("m5angle8", "Write: register address write failed");
        return {};
    }
    res |= _i2c->start(_addr, true, _freq);
    res |= _i2c->read(&r, 1);
    res |= _i2c->stop();
    if (res)
        return r;
    return {};
}

/*! @brief Set the addr of device.
    @return True if the set was successful, otherwise false.. */
/* bool M5_ANGLE8::setDeviceAddr(uint8_t addr) {
    if (writeRegister(ANGLE8_ADDRESS_REG, &addr, 1)) {
        _addr = addr;
        return true;
    } else {
        return false;
    }
} */

/*! @brief Get the Version of Firmware.
    @return Firmware version */
uint8_t M5_ANGLE8::getVersion() {
    return readRegister8(ANGLE8_FW_VERSION_REG).value_or(0);
}

/*! @brief Set the color of led lights.
    @return True if the set was successful, otherwise false.. */
bool M5_ANGLE8::setLEDColor(uint8_t ch, uint32_t color, uint8_t bright) {
    if (ch > ANGLE8_TOTAL_LED) return false;
    uint8_t data[4] = {0};
    data[0]         = (color >> 16) & 0xff;
    data[1]         = (color >> 8) & 0xff;
    data[2]         = color & 0xff;
    data[3]         = bright & 0xff;
    uint8_t reg     = ch * 4 + ANGLE8_RGB_24B_REG;
    return writeRegister(reg, data, 4);
}

/*! @brief Get digital singal input.
    @return True if the read was successful, otherwise false.. */
bool M5_ANGLE8::getDigitalInput() {
    uint8_t data;
    uint8_t reg = ANGLE8_DIGITAL_INPUT_REG;
    if (readRegister(reg, &data, 1)) {
        return data;
    }
    return 0;
}

/*! @brief Get analog singal input.
    @return True if the read was successful, otherwise false.. */
uint16_t M5_ANGLE8::getAnalogInput(uint8_t ch, angle8_analog_read_mode_t bit) {
    if (bit == _8bit) {
        uint8_t data;
        uint8_t reg = ch + ANGLE8_ANALOG_INPUT_8B_REG;
        if (readRegister(reg, &data, 1)) {
            return data;
        }
    } else {
        uint8_t data[2];
        uint8_t reg = ch * 2 + ANGLE8_ANALOG_INPUT_12B_REG;
        if (readRegister(reg, data, 2)) {
            return (data[1] << 8) | data[0];
        }
    }
    ESP_LOGW("m5angle8", "Reading analog input failed");
    return 0;
}

// Get percentage reader from dial
uint8_t M5_ANGLE8::getDialPercent(uint8_t ch, bool reverse) {
    auto signal = getAnalogInput(ch, _8bit); // 0..254
    auto percent = 1.0*signal / 254;
    // discard spurious readings
    if (percent<0 || percent>1)
        return 0;

    if (reverse)
        percent = 1-percent;
    
    return percent*100;
}
