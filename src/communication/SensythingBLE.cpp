//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything Platform - BLE Communication Module
//    OPENVIEW protocol compatible BLE streaming
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "SensythingBLE.h"

SensythingBLE::SensythingBLE() {
    pServer = nullptr;
    pService = nullptr;
    pDataCharacteristic = nullptr;
    pCallbacks = nullptr;
    pHeartRateService = nullptr;
    pSpo2Service = nullptr;
    pHeartRateChar = nullptr;
    pSpo2Char = nullptr;
    vitalsEnabled = false;
    lastHeartRate = -1;
    lastSpo2 = -1;
    connected = false;
    initialized = false;
    deviceName = "Sensything";
}

SensythingBLE::~SensythingBLE() {
    if (pCallbacks) {
        delete pCallbacks;
    }
    if (initialized) {
        BLEDevice::deinit(true);
    }
}

bool SensythingBLE::init(String deviceName) {
    if (initialized) {
        Serial.println(String(EMOJI_WARNING) + " BLE already initialized");
        return true;
    }
    
    this->deviceName = deviceName;
    
    Serial.print(String(EMOJI_INFO) + " Initializing BLE as \"");
    Serial.print(deviceName);
    Serial.println("\"...");
    
    // Initialize BLE Device
    BLEDevice::init(deviceName.c_str());
    
    // Create BLE Server
    pServer = BLEDevice::createServer();
    if (!pServer) {
        Serial.println(String(EMOJI_ERROR) + " Failed to create BLE server");
        return false;
    }
    
    // Set connection callbacks
    pCallbacks = new BLEConnectionCallbacks(&connected);
    pServer->setCallbacks(pCallbacks);
    
    // Create BLE Service (OPENVIEW UUID)
    pService = pServer->createService(SENSYTHING_BLE_SERVICE_UUID);
    if (!pService) {
        Serial.println(String(EMOJI_ERROR) + " Failed to create BLE service");
        return false;
    }
    
    // Create Data Characteristic (NOTIFY property)
    pDataCharacteristic = pService->createCharacteristic(
        SENSYTHING_BLE_DATA_CHAR_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    
    if (!pDataCharacteristic) {
        Serial.println(String(EMOJI_ERROR) + " Failed to create BLE characteristic");
        return false;
    }
    
    // Add Client Characteristic Configuration Descriptor (required for notifications)
    pDataCharacteristic->addDescriptor(new BLE2902());
    
    // Start the service
    pService->start();

    // Create standard SIG health services for boards that report vitals (OX).
    // Heart Rate Service (0x180D) → Heart Rate Measurement (0x2A37)
    // Pulse Oximeter Service (0x1822) → PLX Spot-Check Measurement (0x2A5E)
    // These notify at ~1Hz (only on change), independent of the raw sample stream.
    if (vitalsEnabled) {
        pHeartRateService = pServer->createService(
            BLEUUID((uint16_t)SENSYTHING_BLE_HEARTRATE_SERVICE_UUID));
        pHeartRateChar = pHeartRateService->createCharacteristic(
            BLEUUID((uint16_t)SENSYTHING_BLE_HEARTRATE_CHAR_UUID),
            BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
        pHeartRateChar->addDescriptor(new BLE2902());
        pHeartRateService->start();

        pSpo2Service = pServer->createService(
            BLEUUID((uint16_t)SENSYTHING_BLE_SPO2_SERVICE_UUID));
        pSpo2Char = pSpo2Service->createCharacteristic(
            BLEUUID((uint16_t)SENSYTHING_BLE_SPO2_CHAR_UUID),
            BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
        pSpo2Char->addDescriptor(new BLE2902());
        pSpo2Service->start();

        Serial.println(String(EMOJI_SUCCESS) + " HR (0x180D) and SpO2 (0x1822) services started");
    }

    // Start advertising
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SENSYTHING_BLE_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // Functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println(String(EMOJI_SUCCESS) + " BLE initialized - ready for connections");
    Serial.print(String(EMOJI_INFO) + " Service UUID: ");
    Serial.println(SENSYTHING_BLE_SERVICE_UUID);
    
    initialized = true;
    return true;
}

bool SensythingBLE::init(const BoardConfig& config) {
    String name = String(config.channels[0].label);  // Use first channel label as base
    if (config.boardType == BOARD_TYPE_CAP) {
        name = "Sensything-Cap";
    } else if (config.boardType == BOARD_TYPE_OX) {
        name = "Sensything-OX";
        vitalsEnabled = true;  // expose standard HR + SpO2 services
    }
    return init(name);
}

void SensythingBLE::streamData(const MeasurementData& data, const BoardConfig& config) {
    if (!initialized) {
        return;
    }
    
    if (!connected) {
        return;  // No client connected, skip silently
    }
    
    // Format data as raw Int16 array (GATT notifications provide framing)
    uint8_t buffer[32];  // Max: 16 channels × 2 bytes = 32 bytes
    int bufferSize = formatAsInt16Array(buffer, data, config);
    
    // Send notification
    pDataCharacteristic->setValue(buffer, bufferSize);
    pDataCharacteristic->notify();

    // Push SpO2/HR on their dedicated SIG characteristics (only on change)
    if (vitalsEnabled) {
        updateVitalsCharacteristics(data, config);
    }
}

// IEEE-11073-20601 16-bit SFLOAT: 4-bit signed exponent (0 here) + 12-bit
// signed mantissa. For small integers this is just the value in the low 12 bits.
static uint16_t sfloatFromInt(int value) {
    if (value > 2047)  value = 2047;     // 12-bit signed range
    if (value < -2048) value = -2048;
    return (uint16_t)(value & 0x0FFF);
}

void SensythingBLE::updateVitalsCharacteristics(const MeasurementData& data, const BoardConfig& config) {
    if (!pHeartRateChar || !pSpo2Char) {
        return;
    }

    // OX layout: channel 2 = SpO2 (%), channel 3 = heart rate (bpm)
    int hr = (int)data.channels[3];
    int spo2 = (int)data.channels[2];
    bool signalOk = !(data.status_flags & SENSYTHING_STATUS_NO_SIGNAL);

    // Heart Rate Measurement (0x2A37): flags(uint8) + HR(uint8)
    // flags = 0x00 → HR value format is uint8, no extra fields present.
    if (signalOk && hr > 0 && hr <= 255 && hr != lastHeartRate) {
        uint8_t hrm[2];
        hrm[0] = 0x00;
        hrm[1] = (uint8_t)hr;
        pHeartRateChar->setValue(hrm, sizeof(hrm));
        pHeartRateChar->notify();
        lastHeartRate = hr;
    }

    // PLX Spot-Check Measurement (0x2A5E): flags(uint8) + SpO2(SFLOAT) + PR(SFLOAT)
    // flags = 0x00 → no optional fields. SpO2 and pulse rate as SFLOATs (LE).
    if (signalOk && spo2 > 0 && spo2 <= 100 && spo2 != lastSpo2) {
        uint16_t spo2Sfloat = sfloatFromInt(spo2);
        uint16_t prSfloat = sfloatFromInt(hr > 0 ? hr : 0);
        uint8_t plx[5];
        plx[0] = 0x00;
        plx[1] = spo2Sfloat & 0xFF;
        plx[2] = (spo2Sfloat >> 8) & 0xFF;
        plx[3] = prSfloat & 0xFF;
        plx[4] = (prSfloat >> 8) & 0xFF;
        pSpo2Char->setValue(plx, sizeof(plx));
        pSpo2Char->notify();
        lastSpo2 = spo2;
    }
}

int SensythingBLE::formatAsInt16Array(uint8_t* buffer, const MeasurementData& data, const BoardConfig& config) {
    // BLE GATT notifications: Send raw Int16List (no packet framing needed)
    // OpenView app expects: [Ch0_LSB, Ch0_MSB, Ch1_LSB, Ch1_MSB, ...]
    
    int index = 0;
    
    for (int i = 0; i < config.channelCount; i++) {
        int16_t value;
        
        if (data.status_flags & (1 << i)) {
            // Channel failed - send zero
            value = 0;
        } else {
            // Convert float to int16.
            // Channels whose full-scale range exceeds the int16 range (e.g. the
            // OX 19-bit raw PPG channels, 0..524288) are right-shifted down to
            // fit without wrapping; the shift is derived from the channel's
            // declared maxValue so it is self-describing. Small-range channels
            // (capacitance in pF, SpO2 %, heart rate bpm) pass through unchanged.
            float maxVal = config.channels[i].maxValue;
            if (maxVal > 32767.0f) {
                int shift = 0;
                float m = maxVal;
                while (m > 32767.0f) { m /= 2.0f; shift++; }
                value = (int16_t)((long)(data.channels[i]) >> shift);
            } else {
                value = (int16_t)(data.channels[i]);
            }
        }
        
        // Send as little-endian (LSB first)
        buffer[index++] = value & 0xFF;         // LSB
        buffer[index++] = (value >> 8) & 0xFF;  // MSB
    }
    
    return index;  // Return number of bytes (channelCount × 2)
}
