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
            // Convert float to int16
            // For capacitance (pF), direct conversion (range ~-100 to 100 pF)
            // For PPG, may need scaling in OX board implementation
            value = (int16_t)(data.channels[i]);
        }
        
        // Send as little-endian (LSB first)
        buffer[index++] = value & 0xFF;         // LSB
        buffer[index++] = (value >> 8) & 0xFF;  // MSB
    }
    
    return index;  // Return number of bytes (channelCount × 2)
}
