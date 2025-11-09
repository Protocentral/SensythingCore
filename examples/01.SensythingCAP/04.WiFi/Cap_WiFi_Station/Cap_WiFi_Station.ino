// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Ashwin Whitchurch, Protocentral Electronics <info@protocentral.com>
//
// Sensything Cap - WiFi Station Mode with mDNS
//
// Connects to your existing WiFi network and streams capacitance data
// to a web dashboard accessible via http://sensything.local
//
// Features:
// - Connects to existing WiFi network (Station mode)
// - mDNS enabled: Access via http://sensything.local
// - WebSocket streaming on port 81
// - Real-time web dashboard with canvas chart (no CDN dependencies)
// - Simultaneous USB Serial monitoring
//
// Setup:
// 1. Change WIFI_SSID and WIFI_PASSWORD below
// 2. Upload sketch
// 3. Open Serial Monitor (115200 baud) to see IP address
// 4. Open browser to http://sensything.local (or use IP address)
// 5. Watch real-time capacitance data!
//
// Troubleshooting:
// - If mDNS doesn't work, use the IP address shown in Serial Monitor
// - Windows users: Install Bonjour Print Services for mDNS support
// - macOS/iOS: mDNS works natively
// - Linux: Install avahi-daemon
//
// License: MIT
//
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT).

#include <SensythingCore.h>

const char* WIFI_SSID = "zeus";      // Your WiFi network name
const char* WIFI_PASSWORD = "open1234";     // Your WiFi password

// Measurement Configuration
const unsigned long SAMPLE_RATE_MS = 100;       // 100ms = 10Hz sampling rate

SensythingCap sensything;

void setup() {
    // Initialize platform (sensor + serial)
    sensything.initPlatform();
    
    // Set sample rate
    sensything.setSampleRate(SAMPLE_RATE_MS);
    
    // Connect to WiFi network (Station mode)
    Serial.println();
    Serial.println("🌐 Starting WiFi in Station mode...");
    Serial.println("ℹ️  This keeps you connected to your main WiFi");
    Serial.println();
    
    if (sensything.initWiFiStation(WIFI_SSID, WIFI_PASSWORD)) {
        Serial.println("✓ WiFi Station started successfully!");
        Serial.println();
        Serial.println("============================================================");
        Serial.println("🌐 Dashboard Access:");
        Serial.println("   • http://sensything.local  (preferred - uses mDNS)");
        Serial.println("   • Or use IP address shown above");
        Serial.println("============================================================");
        Serial.println();
    } else {
        Serial.println("✗ WiFi connection failed!");
        Serial.println("ℹ️  Check your SSID and password in the sketch");
        Serial.println("ℹ️  Continuing without WiFi...");
    }
    
    // Enable USB streaming for Serial Monitor
    sensything.enableUSB(false);  // Disable to reduce output clutter
    
    // Start measurements and WiFi streaming
    sensything.startMeasurements();
    sensything.enableWiFi(true);
    
    Serial.println("✓ Auto-started WiFi streaming");
    Serial.println("ℹ️  Type 'help' for available commands");
    Serial.println();
}

void loop() {
    // Update handles:
    // - Serial command processing
    // - Sensor reading at configured rate
    // - WiFi streaming to WebSocket clients
    // - mDNS responder updates
    sensything.update();
}
