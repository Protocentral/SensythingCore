// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Ashwin Whitchurch, Protocentral Electronics <info@protocentral.com>
//
// Sensything Platform - Cap SD Card Logger Example
//
// Demonstrates SD Card logging from Sensything Cap board
//
// Hardware: Sensything Cap (ESP32-S3 + FDC1004 + SD Card via SDIO)
// Features: Buffered CSV logging to SD card with hourly file rotation
//
// Connections (SDIO):
// - CMD:  GPIO 41
// - CLK:  GPIO 38
// - D0:   GPIO 39
//
// To test:
// 1. Insert micro SD card (FAT32 formatted recommended)
// 2. Upload this sketch to Sensything Cap
// 3. Open Serial Monitor at 115200 baud
// 4. Wait for measurements to start
// 5. Files will be created as: sensything_1.csv, sensything_2.csv, etc.
// 6. Data is buffered (10 lines) before writing to reduce SD wear
//
// Commands (via Serial):
// - help: Show available commands
// - status: Display system status (includes SD info)
//    - toggle_sd: Enable/disable SD logging
//    - rotate_file: Force new file creation
//    - set_rate <ms>: Change sample rate (e.g., "set_rate 100")
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <SensythingCore.h>

// Create Cap board instance
SensythingCap sensything;

void setup() {
    // Initialize platform (sensor + Serial + USB streaming by default)
    if (!sensything.initPlatform()) {
        Serial.println("❌ Platform initialization failed!");
        while (1) delay(100);
    }
    
    // Initialize SD Card module
    if (!sensything.initSDCard()) {
        Serial.println("❌ SD Card initialization failed!");
        Serial.println("ℹ️  Check SD card insertion and SDIO connections");
        Serial.println("ℹ️  Continuing without SD logging...");
        // Don't halt - continue without SD logging
    } else {
        // Enable SD Card logging
        sensything.enableSDCard(true);
    }
    
    // Set sample rate to 100ms (10Hz)
    sensything.setSampleRate(100);
    
    // Start measurements
    sensything.startMeasurements();
    
    Serial.println("✓ Setup complete!");
    Serial.println("ℹ️  Data is being logged to SD card (buffered writes)");
    Serial.println("ℹ️  Type 'help' for available commands");
    Serial.println();
}

void loop() {
    // Main update loop - handles timing, reading, streaming, SD logging
    sensything.update();
    
    // Optional: Periodic SD status update (every 30 seconds)
    static unsigned long lastSDStatus = 0;
    unsigned long now = millis();
    
    if (now - lastSDStatus >= 30000) {
        lastSDStatus = now;
        
        if (sensything.isSDReady()) {
            Serial.println("💾 SD Card Status:");
            Serial.printf("   Measurements: %u\n", sensything.getMeasurementCount());
            Serial.println("   Buffer flushed every 10 lines");
            Serial.println("   File rotation: hourly");
        }
    }
}
