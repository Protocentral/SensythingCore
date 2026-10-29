//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything OX - Board Implementation
//    AFE4400-based PPG/SpO2/Heart Rate measurement board
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "SensythingOX.h"

SensythingOX::SensythingOX() : SensythingCore() {
    // Pin configuration (SPI-based)
    BoardPins pins;
    pins.spiSck = SENSYTHING_SPI_SCK;
    pins.spiMiso = SENSYTHING_SPI_MISO;
    pins.spiMosi = SENSYTHING_SPI_MOSI;
    pins.spiCs = 10;
    pins.drdy = 14;    // AFE ADC_RDY
    pins.reset = 7;    // AFE ~ADC_RESET
    pins.pwdn = 21;    // AFE ~PDN
    pins.diag = 2;     // AFE DIAG_END
    
    attachSensor(&ppgSensor, pins, BOARD_TYPE_OX, "Sensything OX");
}
