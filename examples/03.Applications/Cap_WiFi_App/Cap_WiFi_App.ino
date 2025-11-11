// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Ashwin Whitchurch, Protocentral Electronics <info@protocentral.com>
//
// SensythingCAP - Complete WiFi Application
//
// ⚠️  BUILD NOTE: This sketch includes the full embedded web dashboard and exceeds
//    the default ESP32-S3 flash partition size (1.3MB). 
//    
//    REQUIRED Arduino IDE Settings:
//       Board: ESP32S3 Dev Module
//       USB Mode: Hardware CDC and JTAG
//       USB CDC on Boot: Enabled
//       Upload Mode: UART0 / Hardware CDC
//       PSRAM: QSPI PSRAM
//       Partition Scheme: Huge APP (3MB No OTA/1MB SPIFFS)
//       Upload Speed: 921600
//    
//    For ARDUINO-CLI or CI/CD, use the --board-options flag:
//       arduino-cli compile -b esp32:esp32:esp32s3 \
//         --board-options "PartitionScheme=huge_app,PSRAM=enabled,CDCOnBoot=cdc,USBMode=hwcdc" \
//         examples/03.Applications/Cap_WiFi_App/
//    
//    GitHub Actions automatically uses this approach (see .github/workflows/compile-examples.yml)
//
// First Time Setup:
//   1. Upload this sketch to Sensything Cap
//   2. Device creates WiFi AP "sensything" (password: sensything)
//   3. Connect your laptop/phone to "sensything" network
//   4. Open browser to http://192.168.4.1
//   5. Click "Scan Networks" in dashboard
//   6. Select your WiFi network and enter password
//   7. Click "Connect"
//   8. Credentials saved automatically!
//
// After Configuration:
//   - Device auto-connects to your WiFi on every boot
//   - Access dashboard via http://sensything.local (or IP address)
//   - AP remains available at 192.168.4.1 for reconfiguration
//   - Use "Forget Network" button to clear saved credentials

#include <SensythingCore.h>

// Access Point Settings (used when no saved credentials or connection fails)
const char* AP_SSID = "sensything";              // AP network name
const char* AP_PASSWORD = "sensything";          // AP password (min 8 chars)

// Optional: Pre-configure WiFi (leave empty to use saved credentials)
const char* PRECONFIGURED_SSID = "";             // Your WiFi network (optional)
const char* PRECONFIGURED_PASSWORD = "";         // Your WiFi password (optional)

// Measurement Settings
const unsigned long DEFAULT_SAMPLE_RATE_MS = 100;  // 100ms = 10Hz (adjustable via dashboard)

// Feature Enables (can be changed via dashboard)
bool AUTO_START_MEASUREMENTS = true;              // Start measuring on boot
bool AUTO_ENABLE_WIFI = true;                     // Enable WiFi streaming on boot
bool AUTO_ENABLE_USB = false;                     // Enable USB Serial output (disable for cleaner logs)
bool AUTO_ENABLE_BLE = false;                     // Enable BLE on boot (optional)
bool AUTO_ENABLE_SD = false;                      // Enable SD logging on boot (optional)

SensythingCap sensything;  // Sensything Cap board instance

void setup() {
    Serial.println();
    Serial.println("=================================================================");
    Serial.println("   Sensything Cap - Complete WiFi Application");
    Serial.println("=================================================================");
    Serial.println();
    
    sensything.initPlatform();
    
    // -------------------------------------------------------------------------
    // 2. Configure Measurement Settings
    // -------------------------------------------------------------------------
    sensything.setSampleRate(DEFAULT_SAMPLE_RATE_MS);
    
    // -------------------------------------------------------------------------
    // 3. Initialize WiFi with AP+Station Mode
    // -------------------------------------------------------------------------
    Serial.println("🌐 Initializing WiFi...");
    Serial.println();
    
    // AP+Station mode will:
    // - Create AP for configuration portal
    // - Try to load saved credentials from NVS
    // - Auto-connect if credentials exist
    // - Fall back to AP-only if connection fails
    
    if (sensything.initAPStation(AP_SSID, AP_PASSWORD, PRECONFIGURED_SSID, PRECONFIGURED_PASSWORD)) {
        Serial.println("✓ WiFi initialized successfully!");
        Serial.println();
        Serial.println("=================================================================");
        Serial.println("📡 ACCESS POINTS:");
        Serial.println("=================================================================");
        Serial.println();
        Serial.println("Configuration Portal (Always Available):");
        Serial.println("   • WiFi Network: sensything");
        Serial.println("   • Password: sensything");
        Serial.println("   • Dashboard: http://192.168.4.1");
        Serial.println();
        Serial.println("Main Network (After Configuration):");
        Serial.println("   • Dashboard: http://sensything.local");
        Serial.println("   • Or use IP address shown above");
        Serial.println();
        Serial.println("=================================================================");
        Serial.println("🎛️  DASHBOARD FEATURES:");
        Serial.println("=================================================================");
        Serial.println();
        Serial.println("   • Real-time capacitance chart");
        Serial.println("   • WiFi network scanner & configurator");
        Serial.println("   • Sample rate control (1-100 Hz)");
        Serial.println("   • Measurement start/stop");
        Serial.println("   • SD logging enable/disable");
        Serial.println("   • System information display");
        Serial.println("   • Forget saved network");
        Serial.println();
        Serial.println("=================================================================");
        Serial.println();
    } else {
        Serial.println("✗ WiFi initialization failed!");
        Serial.println("ℹ️  Continuing without WiFi...");
    }
    
    // -------------------------------------------------------------------------
    // 4. Initialize Optional Interfaces
    // -------------------------------------------------------------------------
    
    // USB Serial Streaming
    if (AUTO_ENABLE_USB) {
        sensything.enableUSB(true);
        Serial.println("✓ USB streaming enabled");
    } else {
        sensything.enableUSB(false);
        Serial.println("ℹ️  USB streaming disabled (enable via 'start_usb' command)");
    }
    
    // BLE Streaming (optional)
    if (AUTO_ENABLE_BLE) {
        if (sensything.initBLE()) {
            sensything.enableBLE(true);
            Serial.println("✓ BLE streaming enabled");
        } else {
            Serial.println("⚠️  BLE initialization failed");
        }
    }
    
    // SD Card Logging (optional)
    if (AUTO_ENABLE_SD) {
        if (sensything.initSDCard()) {
            sensything.enableSDCard(true);
            Serial.println("✓ SD card logging enabled");
        } else {
            Serial.println("⚠️  SD card initialization failed");
        }
    }
    
    // -------------------------------------------------------------------------
    // 5. Start Measurements
    // -------------------------------------------------------------------------
    
    if (AUTO_START_MEASUREMENTS) {
        sensything.startMeasurements();
        Serial.println("✓ Measurements started");
    }
    
    if (AUTO_ENABLE_WIFI) {
        sensything.enableWiFi(true);
        Serial.println("✓ WiFi streaming enabled");
    }
    
    // -------------------------------------------------------------------------
    // 6. Ready!
    // -------------------------------------------------------------------------
    
    Serial.println();
    Serial.println("=================================================================");
    Serial.println("✓ System Ready");
    Serial.println("=================================================================");
    Serial.println();
    Serial.println("💡 Quick Commands:");
    Serial.println("   help       - Show all commands");
    Serial.println("   status     - Display system state");
    Serial.println("   start_all  - Enable all interfaces");
    Serial.println("   stop_all   - Disable all interfaces");
    Serial.println("   set_rate <ms> - Change sample rate");
    Serial.println();
    Serial.println("=================================================================");
    Serial.println();
}

void loop() {
    // The update() method handles everything:
    // - Serial command processing
    // - Sensor reading at configured rate
    // - WiFi streaming to WebSocket clients
    // - USB/BLE/SD streaming (if enabled)
    // - mDNS responder updates
    // - Web server HTTP requests
    // - Automatic WiFi reconnection
    
    sensything.update();
    
    // Optional: Add your custom code here
    // The update() method is non-blocking and returns quickly
}