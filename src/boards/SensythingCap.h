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

#include "../core/SensythingCore.h"
#include "../sensors/FDC1004Sensor.h"

class SensythingCap : public SensythingCore {
public:
    SensythingCap();
    
private:
    FDC1004Sensor capSensor;
};

#endif // SENSYTHING_CAP_H
