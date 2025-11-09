// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Ashwin Whitchurch, Protocentral Electronics <info@protocentral.com>
//
// Sensything Cap - WiFi Configuration Portal (AP+Station Mode)
//==============================================================================
// Features:
// - Starts in AP mode (SSID: "sensything")
// - Web dashboard with configuration panel
// - WiFi network scanner and connector
// - Sample rate control
// - SD logging enable/disable
// - Automatic mDNS after WiFi connection
// - Simultaneous AP + Station mode
//
// How to Use:
// 1. Upload this sketch
// 2. Connect to "sensything" WiFi network (password: sensything)
// 3. Open browser to http://192.168.4.1
// 4. Click "Scan Networks" and select your WiFi
// 5. Enter password and click "Connect"
// 6. Device will connect while keeping AP active
// 7. Access via http://sensything.local on your main network
//
// Control Panel Features:
// - WiFi Configuration: Scan and connect to networks
// - Measurement Settings: Change sample rate, start/stop
// - Data Logging: Enable/disable SD card logging
// - System Info: View WiFi status, IP addresses
//
// License: MIT
//
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT).

#include <SensythingCore.h>

const char* AP_SSID = "sensything";             // Access Point name
const char* AP_PASSWORD = "sensything";         // AP password (min 8 chars)

// Optional: Pre-configure Station credentials (leave empty for portal-only)
const char* STA_SSID = "";                      // Your WiFi network (optional)
const char* STA_PASSWORD = "";                  // Your WiFi password (optional)

// Measurement Configuration
const unsigned long SAMPLE_RATE_MS = 100;       // 100ms = 10Hz sampling rate
SensythingCap sensything;

void setup() {
    // Initialize platform (sensor + serial)
    sensything.initPlatform();
    
    // Set sample rate
    sensything.setSampleRate(SAMPLE_RATE_MS);
    
    // Initialize WiFi in AP+Station mode
    Serial.println();
    Serial.println("🌐 Starting WiFi Configuration Portal...");
    Serial.println();
    
    if (sensything.initAPStation(AP_SSID, AP_PASSWORD, STA_SSID, STA_PASSWORD)) {
        Serial.println("✓ WiFi portal started successfully!");
        Serial.println();
        Serial.println("============================================================");
        Serial.println("📡 Configuration Portal:");
        Serial.println("   1. Connect to WiFi: sensything");
        Serial.println("   2. Password: sensything");
        Serial.println("   3. Open: http://192.168.4.1");
        Serial.println("   4. Configure your WiFi network");
        Serial.println();
        Serial.println("After connecting to your network:");
        Serial.println("   • Access via: http://sensything.local");
        Serial.println("   • AP remains active for configuration");
        Serial.println("============================================================");
        Serial.println();
    } else {
        Serial.println("✗ WiFi initialization failed!");
    }
    
    // Disable USB streaming to reduce clutter (optional)
    sensything.enableUSB(false);
    
    // Start measurements and WiFi streaming
    sensything.startMeasurements();
    sensything.enableWiFi(true);
    
    Serial.println("✓ System ready");
    Serial.println("ℹ️  Type 'help' for Serial commands");
    Serial.println();
}

void loop() {
    // Update handles:
    // - Serial command processing
    // - Sensor reading at configured rate  
    // - WiFi streaming to WebSocket clients
    // - mDNS responder updates
    // - Web server HTTP requests
    sensything.update();
}
