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
    ppgSensor = nullptr;
    configureBoardConfig();
}

SensythingOX::~SensythingOX() {
    if (ppgSensor) {
        delete ppgSensor;
        ppgSensor = nullptr;
    }
}

void SensythingOX::configureBoardConfig() {
    boardConfig.boardType = BOARD_TYPE_OX;
    boardConfig.boardName = "Sensything OX";
    boardConfig.sensorType = "AFE4400 PPG/SpO2";
    boardConfig.channelCount = 4;
    
    // Configure channel information
    // Channel 0: IR PPG raw data
    snprintf(boardConfig.channels[0].label, SENSYTHING_MAX_LABEL_LEN, "ir_raw");
    snprintf(boardConfig.channels[0].unit, 16, "ADC");
    boardConfig.channels[0].minValue = 0.0;
    boardConfig.channels[0].maxValue = 524288.0;  // 19-bit ADC
    boardConfig.channels[0].active = true;
    
    // Channel 1: RED PPG raw data
    snprintf(boardConfig.channels[1].label, SENSYTHING_MAX_LABEL_LEN, "red_raw");
    snprintf(boardConfig.channels[1].unit, 16, "ADC");
    boardConfig.channels[1].minValue = 0.0;
    boardConfig.channels[1].maxValue = 524288.0;  // 19-bit ADC
    boardConfig.channels[1].active = true;
    
    // Channel 2: SpO2 percentage
    snprintf(boardConfig.channels[2].label, SENSYTHING_MAX_LABEL_LEN, "spo2");
    snprintf(boardConfig.channels[2].unit, 16, "%%");
    boardConfig.channels[2].minValue = 0.0;
    boardConfig.channels[2].maxValue = 100.0;
    boardConfig.channels[2].active = true;
    
    // Channel 3: Heart Rate
    snprintf(boardConfig.channels[3].label, SENSYTHING_MAX_LABEL_LEN, "heart_rate");
    snprintf(boardConfig.channels[3].unit, 16, "bpm");
    boardConfig.channels[3].minValue = 0.0;
    boardConfig.channels[3].maxValue = 250.0;
    boardConfig.channels[3].active = true;
    
    // Pin configuration (SPI-based)
    boardConfig.i2c_sda = -1;  // Not used
    boardConfig.i2c_scl = -1;  // Not used
    boardConfig.spi_cs = PIN_SPI_CS;
    boardConfig.spi_mosi = SENSYTHING_SPI_MOSI;
    boardConfig.spi_miso = SENSYTHING_SPI_MISO;
    boardConfig.spi_sck = SENSYTHING_SPI_SCK;
    
    // Sample rate limits (AFE4400 typically runs at ~125Hz)
    boardConfig.minSampleInterval = 8;     // 125Hz maximum (8ms period)
    boardConfig.maxSampleInterval = 10000;  // 0.1Hz minimum
}

bool SensythingOX::initSensor() {
    // Initialize SPI
    SPI.begin(boardConfig.spi_sck, boardConfig.spi_miso, boardConfig.spi_mosi, boardConfig.spi_cs);
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    SPI.setFrequency(4000000);  // 4 MHz
    
    // Create AFE44XX sensor instance (published library only takes CS and PWDN pins)
    ppgSensor = new AFE44XX(PIN_SPI_CS, PIN_AFE_PWDN);
    
    // Initialize sensor
    ppgSensor->afe44xx_init();
    
    DEBUG_PRINTLN("AFE4400 initialized successfully");
    return true;
}

bool SensythingOX::readMeasurement(MeasurementData& data) {
    if (!ppgSensor) {
        DEBUG_PRINTLN("Sensor not initialized");
        return false;
    }
    
    // Get data from AFE4400
    if (!ppgSensor->get_AFE44XX_Data(&rawData)) {
        DEBUG_PRINTLN("Failed to read AFE4400 data");
        return false;
    }
    
    // Initialize measurement data
    data.timestamp = millis();
    data.channel_count = 4;
    data.status_flags = 0;
    
    // Fill channel data
    // Channel 0: IR raw ADC value
    data.channels[0] = (float)rawData.IR_data;
    
    // Channel 1: RED raw ADC value
    data.channels[1] = (float)rawData.RED_data;
    
    // Channel 2: SpO2 percentage (0-100%)
    data.channels[2] = (float)rawData.spo2;
    
    // Channel 3: Heart rate (bpm)
    data.channels[3] = (float)rawData.heart_rate;
    
    // Status flags
    if (rawData.buffer_count_overflow) {
        data.status_flags |= SENSYTHING_STATUS_BUFFER_OVERFLOW;
    }
    
    // Check for invalid heart rate (typically 0 or >250 indicates no signal)
    if (rawData.heart_rate == 0 || rawData.heart_rate > 250) {
        data.status_flags |= SENSYTHING_STATUS_NO_SIGNAL;
    }
    
    // Check for invalid SpO2 (typically <70% or >100% indicates error)
    if (rawData.spo2 < 70.0 || rawData.spo2 > 100.0) {
        data.status_flags |= SENSYTHING_STATUS_INVALID_DATA;
    }
    
    // Metadata: Store quality indicators
    data.metadata[0] = (uint8_t)(rawData.spo2);  // SpO2 as byte
    data.metadata[1] = (uint8_t)(rawData.heart_rate);  // HR as byte
    data.metadata[2] = rawData.buffer_count_overflow ? 1 : 0;
    data.metadata[3] = 0;  // Reserved
    
    return true;
}

String SensythingOX::getBoardName() {
    return boardConfig.boardName;
}

String SensythingOX::getSensorType() {
    return boardConfig.sensorType;
}

BoardConfig SensythingOX::getBoardConfig() {
    return boardConfig;
}
