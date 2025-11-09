//////////////////////////////////////////////////////////////////////////////////////////
// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Ashwin Whitchurch, Protocentral Electronics <info@protocentral.com>
//
// Sensything OX - BLE Streaming Example
//
// Demonstrates BLE (OPENVIEW protocol) streaming from Sensything OX board
//
// Hardware: Sensything OX (ESP32-S3 + AFE4400 PPG sensor)
// Features: PPG/SpO2/HR streaming via BLE notifications
//
// To test:
// 1. Upload this sketch to Sensything OX
// 2. Open Serial Monitor at 115200 baud
// 3. Connect with BLE app (nRF Connect or OpenView mobile app)
// 4. Service UUID: 0001A7D3-D8A4-4FEA-8174-1736E808C066
// 5. Subscribe to Data Characteristic notifications
// 6. Observe data stream (format: IR_raw, RED_raw, SpO2%, HR)
//
// BLE Data Format (OX):
// Binary packet: [IR_LSB][IR_MSB][RED_LSB][RED_MSB][SpO2_LSB][SpO2_MSB][HR_LSB][HR_MSB]
// - IR/RED: 19-bit raw ADC values (0-524287, typically 200,000-400,000)
// - SpO2: Oxygen saturation percentage (70-100% normal)
// - HR: Heart rate in beats per minute (40-200 typical)
//
// Commands (via Serial):
// - help: Show available commands
// - status: Display system status
// - start_all: Start BLE streaming
// - stop_all: Stop BLE streaming
// - set_rate <ms>: Change sample rate (OX fixed at ~8ms, but can enable subsampling)
//
// Board Configuration (CRITICAL):
// - Board: ESP32S3 Dev Module
// - USB Mode: Hardware CDC and JTAG
// - USB CDC on Boot: Enabled
// - Upload Speed: 921600
//
// Required Libraries:
// - Sensything Platform (this library)
// - Protocentral_AFE44xx (via Arduino Library Manager)
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <SensythingES3.h>

// Create OX board instance
SensythingOX sensything;

void setup() {
    // Initialize platform (sensor + Serial + USB streaming by default)
    if (!sensything.initPlatform()) {
        Serial.println("❌ Platform initialization failed!");
        while (1) delay(100);
    }
    
    Serial.println("✓ Platform initialized successfully");
    Serial.println("ℹ️  Board: " + sensything.getBoardName());
    Serial.println("ℹ️  Sensor: " + sensything.getSensorType());
    Serial.println();
    
    // Initialize BLE module
    if (!sensything.initBLE()) {
        Serial.println("❌ BLE initialization failed!");
        while (1) delay(100);
    }
    
    Serial.println("✓ BLE initialized successfully");
    
    // Enable BLE streaming (disabled by default)
    sensything.enableBLE(true);
    Serial.println("📱 BLE streaming enabled - waiting for connections");
    
    // OX board runs at ~125Hz (8ms intervals) natively
    // You can optionally set a slower rate if needed
    sensything.setSampleRate(8);  // 125Hz native rate
    
    // Start measurements
    sensything.startMeasurements();
    
    Serial.println("✓ Setup complete!");
    Serial.println();
    Serial.println("📋 BLE Connection Instructions:");
    Serial.println("   1. Use nRF Connect (iOS/Android) or similar BLE app");
    Serial.println("   2. Look for device: 'Sensything-OX-XXXX'");
    Serial.println("   3. Connect to service: 0001A7D3-D8A4-4FEA-8174-1736E808C066");
    Serial.println("   4. Enable notifications on characteristic: 0002A7D3-D8A4-4FEA-8174-1736E808C066");
    Serial.println("   5. Data will stream as binary packets (8 bytes per measurement)");
    Serial.println();
    Serial.println("🫀 Measurement Guide:");
    Serial.println("   - Place finger/wrist on PPG sensor for best results");
    Serial.println("   - IR: Infrared raw value (200k-400k typical)");
    Serial.println("   - RED: Red LED raw value (200k-400k typical)");
    Serial.println("   - SpO2: Blood oxygen saturation (95-99% healthy)");
    Serial.println("   - HR: Heart rate (50-100 bpm resting)");
    Serial.println();
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
            Serial.println("📱 ✓ BLE client connected - streaming active");
        } else {
            Serial.println("📱 ✗ BLE client disconnected - waiting for connection");
        }
        lastBLEState = currentBLEState;
    }
}
