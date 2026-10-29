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

#include "../core/SensythingCore.h"
#include "../sensors/AFE4400Sensor.h"

class SensythingOX : public SensythingCore {
public:
    SensythingOX();
    
private:
    AFE4400Sensor ppgSensor;
};

#endif // SENSYTHING_OX_H
