//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything OX - Board Implementation
//    AFE4400-based PPG/SpO2/Heart Rate measurement board
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#ifndef SENSYTHING_OX_H
#define SENSYTHING_OX_H

#include <SPI.h>
#include <protocentral_afe44xx.h>
#include "../core/SensythingCore.h"

class SensythingOX : public SensythingCore {
public:
    SensythingOX();
    ~SensythingOX();
    
    // Implement pure virtual methods from SensythingCore
    bool initSensor() override;
    bool readMeasurement(MeasurementData& data) override;
    String getBoardName() override;
    String getSensorType() override;
    BoardConfig getBoardConfig() override;
    
private:
    AFE44XX* ppgSensor;
    afe44xx_data rawData;
    
    // Pin definitions (from proposal)
    static const uint8_t PIN_SPI_CS   = 10;
    static const uint8_t PIN_AFE_DRDY = 14;
    static const uint8_t PIN_AFE_PWDN = 21;
    
    /**
     * Configure board-specific settings
     */
    void configureBoardConfig();
};

#endif // SENSYTHING_OX_H
