//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    SensythingES3 - BLE Communication Module
//    OPENVIEW protocol compatible BLE streaming
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#ifndef SENSYTHING_BLE_H
#define SENSYTHING_BLE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "../core/SensythingTypes.h"
#include "../core/SensythingConfig.h"

// BLE connection callback class
class BLEConnectionCallbacks : public BLEServerCallbacks {
public:
    BLEConnectionCallbacks(bool* connected) : pConnected(connected) {}
    
    void onConnect(BLEServer* pServer) override {
        *pConnected = true;
        Serial.println(String(EMOJI_SUCCESS) + " BLE client connected");
    }
    
    void onDisconnect(BLEServer* pServer) override {
        *pConnected = false;
        Serial.println(String(EMOJI_INFO) + " BLE client disconnected");
        // Restart advertising
        BLEDevice::startAdvertising();
    }
    
private:
    bool* pConnected;
};

class SensythingBLE {
public:
    SensythingBLE();
    ~SensythingBLE();
    
    /**
     * Initialize BLE with device name and OPENVIEW service
     * @param deviceName Name to advertise (e.g., "Sensything-Cap")
     * @return true if initialization successful
     */
    bool init(String deviceName);
    
    /**
     * Initialize BLE with board-specific configuration
     * @param config Board configuration containing board name
     * @return true if initialization successful
     */
    bool init(const BoardConfig& config);
    
    /**
     * Stream measurement data via BLE notification
     * @param data Measurement data structure
     * @param config Board configuration for format adaptation
     */
    void streamData(const MeasurementData& data, const BoardConfig& config);
    
    /**
     * Check if a BLE client is connected
     * @return true if client connected
     */
    bool isConnected() const { return connected; }
    
    /**
     * Get device name
     * @return BLE device name
     */
    String getDeviceName() const { return deviceName; }
    
private:
    BLEServer* pServer;
    BLEService* pService;
    BLECharacteristic* pDataCharacteristic;
    BLEConnectionCallbacks* pCallbacks;
    
    String deviceName;
    bool connected;
    bool initialized;
    
    /**
     * Format measurement data as raw Int16 array for GATT notifications
     * @param buffer Output buffer (2 bytes per channel)
     * @param data Measurement data
     * @param config Board configuration
     * @return Buffer size in bytes (channelCount × 2)
     */
    int formatAsInt16Array(uint8_t* buffer, const MeasurementData& data, const BoardConfig& config);
    
    /**
     * Create BLE server and service
     * @return true if successful
     */
    bool setupBLEServer();
};

#endif // SENSYTHING_BLE_H
