//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything Cap - Board Implementation
//    FDC1004-based 4-channel capacitance measurement board
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "SensythingCap.h"

SensythingCap::SensythingCap() : SensythingCore() {
    // Pin configuration (I2C-based)
    BoardPins pins;
    pins.i2cSda = SENSYTHING_I2C_SDA;
    pins.i2cScl = SENSYTHING_I2C_SCL;
    pins.i2cFreq = SENSYTHING_I2C_FREQ;
    
    attachSensor(&capSensor, pins, BOARD_TYPE_CAP, "Sensything Cap");
}
