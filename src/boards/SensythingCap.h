//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything Cap - Board Implementation
//    FDC1004-based 4-channel capacitance measurement board
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#ifndef SENSYTHING_CAP_H
#define SENSYTHING_CAP_H

#include <Wire.h>
#include <Protocentral_FDC1004.h>
#include "../core/SensythingCore.h"

class SensythingCap : public SensythingCore {
public:
    SensythingCap();
    ~SensythingCap();
    
    // Implement pure virtual methods from SensythingCore
    bool initSensor() override;
    bool readMeasurement(MeasurementData& data) override;
    String getBoardName() override;
    String getSensorType() override;
    BoardConfig getBoardConfig() override;
    
private:
    FDC1004* capSensor;
    
    /**
     * Configure board-specific settings
     */
    void configureBoardConfig();
};

#endif // SENSYTHING_CAP_H
