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
    capSensor = nullptr;
    configureBoardConfig();
}

SensythingCap::~SensythingCap() {
    if (capSensor) {
        delete capSensor;
    }
}

void SensythingCap::configureBoardConfig() {
    boardConfig.boardType = BOARD_TYPE_CAP;
    boardConfig.boardName = "Sensything Cap";
    boardConfig.sensorType = "FDC1004 Capacitance";
    boardConfig.channelCount = 4;
    
    // Configure channel information
    for (int i = 0; i < 4; i++) {
        snprintf(boardConfig.channels[i].label, SENSYTHING_MAX_LABEL_LEN, 
                "ch%d_pf", i);
        snprintf(boardConfig.channels[i].unit, 16, "pF");
        boardConfig.channels[i].minValue = -100.0;
        boardConfig.channels[i].maxValue = 100.0;
        boardConfig.channels[i].active = true;
    }
    
    // Pin configuration
    boardConfig.i2c_sda = SENSYTHING_I2C_SDA;
    boardConfig.i2c_scl = SENSYTHING_I2C_SCL;
    boardConfig.spi_cs = -1;  // Not used
    boardConfig.spi_mosi = -1;
    boardConfig.spi_miso = -1;
    boardConfig.spi_sck = -1;
    
    // Sample rate limits
    boardConfig.minSampleInterval = SENSYTHING_MIN_SAMPLE_INTERVAL_MS;
    boardConfig.maxSampleInterval = SENSYTHING_MAX_SAMPLE_INTERVAL_MS;
}

bool SensythingCap::initSensor() {
    // Initialize I2C
    Wire.begin(boardConfig.i2c_sda, boardConfig.i2c_scl);
    Wire.setClock(SENSYTHING_I2C_FREQ);
    
    // Create FDC1004 sensor instance
    capSensor = new FDC1004(FDC1004_RATE_100HZ);
    
    // Initialize sensor
    if (!capSensor->begin()) {
        DEBUG_PRINTLN("FDC1004 begin() failed");
        return false;
    }
    
    // Verify sensor is responding
    if (!capSensor->isConnected()) {
        DEBUG_PRINTLN("FDC1004 not responding on I2C bus");
        return false;
    }
    
    DEBUG_PRINTLN("FDC1004 initialized successfully");
    return true;
}

bool SensythingCap::readMeasurement(MeasurementData& data) {
    if (!capSensor) {
        DEBUG_PRINTLN("Sensor not initialized");
        return false;
    }
    
    // Initialize measurement data
    data.timestamp = millis();
    data.channel_count = 4;
    data.status_flags = 0;
    
    // Read all 4 channels
    for (int i = 0; i < 4; i++) {
        fdc1004_capacitance_t measurement = capSensor->getCapacitanceMeasurement(
            static_cast<fdc1004_channel_t>(FDC1004_CHANNEL_0 + i)
        );
        
        // Check if measurement is valid
        if (!isnan(measurement.capacitance_pf)) {
            data.channels[i] = measurement.capacitance_pf;
            data.metadata[i] = measurement.capdac_used;
            
            // Check if CAPDAC is adjusting (out of range)
            if (measurement.capdac_out_of_range) {
                data.status_flags |= SENSYTHING_STATUS_CAPDAC_ADJ;
            }
        } else {
            // Measurement failed for this channel
            data.channels[i] = 0.0;
            data.metadata[i] = 0;
            data.status_flags |= (1 << i);  // Set channel fail bit
            
            DEBUG_PRINTF("Channel %d measurement failed\n", i);
        }
    }
    
    return true;
}

String SensythingCap::getBoardName() {
    return boardConfig.boardName;
}

String SensythingCap::getSensorType() {
    return boardConfig.sensorType;
}

BoardConfig SensythingCap::getBoardConfig() {
    return boardConfig;
}
