// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Ashwin Whitchurch, Protocentral Electronics <info@protocentral.com>
//
// Sensything Platform - Cap BLE Streaming Example
//
// Demonstrates BLE (OPENVIEW protocol) streaming from Sensything Cap board
//
// Hardware: Sensything Cap (ESP32-S3 + FDC1004)
// Features: 4-channel capacitance streaming via BLE notifications
//
// To test:
// 1. Upload this sketch to Sensything Cap
// 2. Open Serial Monitor at 115200 baud
// 3. Connect with BLE app (nRF Connect or OpenView mobile app)
// 4. Service UUID: 0001A7D3-D8A4-4FEA-8174-1736E808C066
// 5. Subscribe to Data Characteristic notifications
// 6. Observe raw Int16 data stream (8 bytes = 4 channels × 16-bit)
//
// BLE Data Format:
// [Ch0_LSB][Ch0_MSB][Ch1_LSB][Ch1_MSB][Ch2_LSB][Ch2_MSB][Ch3_LSB][Ch3_MSB]
// - Each channel: 16-bit signed integer (capacitance in pF)
// - No packet framing - GATT notifications provide message boundaries
// - Compatible with Protocentral OpenView mobile app
//
// Commands (via Serial):
// - help: Show available commands
// - status: Display system status
// - start_all: Start BLE streaming
// - stop_all: Stop BLE streaming
// - set_rate <ms>: Change sample rate (e.g., "set_rate 100")
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <SensythingES3.h>

// Create Cap board instance
SensythingCap sensything;

void setup() {
    // Initialize platform (sensor + Serial + USB streaming by default)
    if (!sensything.initPlatform()) {
        Serial.println("❌ Platform initialization failed!");
        while (1) delay(100);
    }
    
    // Initialize BLE module
    if (!sensything.initBLE()) {
        Serial.println("❌ BLE initialization failed!");
        while (1) delay(100);
    }
    
    // Enable BLE streaming (disabled by default)
    sensything.enableBLE(true);
    
    // Set sample rate to 100ms (10Hz)
    sensything.setSampleRate(100);
    
    // Start measurements
    sensything.startMeasurements();
    
    Serial.println("✓ Setup complete!");
    Serial.println("ℹ️  Waiting for BLE connections...");
    Serial.println("ℹ️  Type 'help' for available commands");
    Serial.println();
}

void loop() {
    // Main update loop - handles timing, reading, streaming, commands
    sensything.update();
    
    // Optional: Print connection status changes
    static bool lastBLEState = false;
    bool currentBLEState = sensything.isBLEConnected();
    
    if (currentBLEState != lastBLEState) {
        if (currentBLEState) {
            Serial.println("📱 BLE client connected - streaming active");
        } else {
            Serial.println("📱 BLE client disconnected - waiting for connection");
        }
        lastBLEState = currentBLEState;
    }
}
