// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Ashwin Whitchurch, Protocentral Electronics <info@protocentral.com>
//
// Sensything Cap - WiFi WebSocket Streaming with Web Dashboard
//==============================================================================
// This example demonstrates WiFi connectivity with real-time WebSocket streaming
// and an embedded web dashboard for the Sensything Cap board.
//
// Features:
// - Access Point (AP) mode: Creates standalone WiFi network
// - Station mode: Connects to existing WiFi network
// - WebSocket streaming on port 81 (real-time data)
// - Web dashboard on port 80 (browser-based monitoring)
// - Real-time plotting with Chart.js
// - Serial commands for configuration
//
// Hardware Setup:
// - Sensything Cap board with FDC1004 capacitance sensor
// - USB-C connection for Serial Monitor (command interface)
// - No external sensors required (4 capacitance channels available)
//
// WiFi Access:
// - AP Mode: Look for "Sensything-XXXX" network in WiFi settings
// - Default Password: "sensything" (can be changed)
// - Dashboard URL: http://192.168.4.1 (AP mode default IP)
// - Station Mode: Use IP address shown in Serial Monitor
//
// Serial Commands (115200 baud):
// - help              : Show all available commands
// - status            : Display current system state
// - start_all         : Enable WiFi streaming
// - stop_all          : Disable WiFi streaming
// - set_rate <ms>     : Change sample rate (e.g., "set_rate 100" for 10Hz)
//
// Web Dashboard Features:
// - Real-time line chart with 4 capacitance channels
// - Connection status indicator
// - Sample rate display
// - Measurement counter
// - Auto-scaling Y-axis
// - Responsive design (mobile-friendly)
//
// WebSocket Protocol (JSON):
// {
//   "ts": 123456,        // Timestamp (milliseconds)
//   "cnt": 42,           // Measurement count
//   "ch": [10.5, 12.3, -5.2, 8.7],  // Channel values (pF)
//   "flags": 0           // Status flags (0 = all channels OK)
// }
//
// Board Configuration (CRITICAL):
// - Board: ESP32S3 Dev Module
// - USB Mode: Hardware CDC and JTAG
// - USB CDC on Boot: Enabled
// - PSRAM: QSPI PSRAM (required for web dashboard)
// - Partition Scheme: Huge APP (3MB No OTA)
// - Upload Speed: 921600
//
// Compilation Notes:
// - Expected size: ~600-700KB (web dashboard embedded)
// - Requires WebSockets library (Links2004)
// - Uses ESP32 WiFi, WebServer, and SPIFFS
//
// Usage Flow:
// 1. Upload sketch to Sensything Cap board
// 2. Open Serial Monitor at 115200 baud
// 3. Wait for WiFi initialization message
// 4. Connect to WiFi network (AP or Station)
// 5. Open web browser to http://192.168.4.1 (AP) or shown IP (Station)
// 6. Type "start_all" in Serial Monitor to begin streaming
// 7. Watch real-time data in browser dashboard
//
// License: MIT (see LICENSE file)
//
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT).

#include <SensythingCore.h>

// WiFi Mode Selection (uncomment ONE of these)
#define WIFI_MODE_AP          // Create standalone Access Point
// #define WIFI_MODE_STATION  // Connect to existing WiFi network

// Station Mode Credentials (only used if WIFI_MODE_STATION is enabled)
const char* WIFI_SSID = "YourNetworkName";      // Change to your WiFi SSID
const char* WIFI_PASSWORD = "YourPassword";     // Change to your WiFi password

// AP Mode Configuration (only used if WIFI_MODE_AP is enabled)
const char* AP_PASSWORD = "sensything";         // AP password (min 8 characters, NULL for open)

// Measurement Configuration
const unsigned long SAMPLE_RATE_MS = 100;       // 100ms = 10Hz sampling rate
SensythingCap sensything;  // Sensything Cap board instance

void setup() {
    // Initialize platform (Serial, FDC1004 sensor, USB streaming)
    if (!sensything.initPlatform()) {
        Serial.println("❌ Platform initialization failed!");
        while (1) { delay(1000); }  // Halt on critical failure
    }
    
    Serial.println("✓ Platform initialized successfully");
    Serial.println("ℹ️  Board: " + sensything.getBoardName());
    Serial.println("ℹ️  Sensor: " + sensything.getSensorType());
    Serial.println();
    
    // Initialize WiFi
    bool wifiSuccess = false;
    
#ifdef WIFI_MODE_AP
    // Access Point Mode: Create standalone WiFi network
    Serial.println("🌐 Starting WiFi in Access Point mode...");
    wifiSuccess = sensything.initWiFi(AP_PASSWORD);  // Auto-generates SSID from board name
    
    if (wifiSuccess) {
        Serial.println("✓ WiFi AP started successfully!");
        Serial.println("📡 Network SSID: Sensything-XXXX (check Serial output above)");
        Serial.println("📡 Password: " + String(AP_PASSWORD ? AP_PASSWORD : "(none - open network)"));
        Serial.println("🌐 Dashboard URL: http://192.168.4.1");
    }
    
#elif defined(WIFI_MODE_STATION)
    // Station Mode: Connect to existing WiFi network
    Serial.println("🌐 Connecting to WiFi network: " + String(WIFI_SSID));
    wifiSuccess = sensything.initWiFiStation(WIFI_SSID, WIFI_PASSWORD);
    
    if (wifiSuccess) {
        Serial.println("✓ WiFi connected successfully!");
        Serial.print("🌐 Dashboard URL: http://");
        Serial.println(WiFi.localIP());
    }
    
#else
    #error "Must define either WIFI_MODE_AP or WIFI_MODE_STATION"
#endif
    
    if (!wifiSuccess) {
        Serial.println("❌ WiFi initialization failed!");
        Serial.println("ℹ️  Continuing with Serial-only operation...");
    }
    
    Serial.println();
    
    // Configure sample rate
    sensything.setSampleRate(SAMPLE_RATE_MS);
    Serial.println("✓ Sample rate set to " + String(SAMPLE_RATE_MS) + "ms (" + 
                   String(1000.0 / SAMPLE_RATE_MS, 1) + " Hz)");
    Serial.println();
    
    // Print instructions
    Serial.println("============================================================");
    Serial.println("WiFi Streaming Ready - Type 'help' for commands");
    Serial.println("============================================================");
    Serial.println();
    Serial.println("Quick Start:");
    Serial.println("  1. Connect to WiFi network (see SSID above)");
    Serial.println("  2. Open web browser to dashboard URL");
    Serial.println("  3. Type 'start_all' to begin streaming");
    Serial.println();
    Serial.println("Available Commands:");
    Serial.println("  help       - Show all commands");
    Serial.println("  status     - Display system state");
    Serial.println("  start_all  - Enable WiFi streaming");
    Serial.println("  stop_all   - Disable WiFi streaming");
    Serial.println("  set_rate <ms> - Change sample rate");
    Serial.println();
    
    // Auto-start streaming (optional - comment out if you prefer manual start)
    sensything.startMeasurements();
    // Disable USB streaming to avoid output collision with WiFi debug messages
    sensything.enableUSB(false);
    sensything.enableWiFi(true);
    Serial.println("✓ Auto-started WiFi streaming (USB streaming disabled for debug)");
    Serial.println("ℹ️  Use 'stop_wifi' command to pause streaming");
    Serial.println();
}

void loop() {
    // Update handles:
    // - Serial command processing
    // - Sensor reading at configured rate
    // - WiFi streaming to WebSocket clients
    // - WebServer HTTP request handling
    // - USB streaming to Serial Monitor
    sensything.update();
}