//////////////////////////////////////////////////////////////////////////////////////////
// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Ashwin Whitchurch, Protocentral Electronics <info@protocentral.com>
//
// Sensything OX - SD Card Logger Example
//
// Demonstrates SD Card logging from Sensything OX board
//
// Hardware: Sensything OX (ESP32-S3 + AFE4400 + SD Card via SDIO)
// Features: PPG/SpO2/HR data buffered CSV logging to SD card with hourly file rotation
//
// Connections (SDIO):
// - CMD:  GPIO 41
// - CLK:  GPIO 38
// - D0:   GPIO 39
//
// To test:
// 1. Insert micro SD card (FAT32 formatted recommended)
// 2. Upload this sketch to Sensything OX
// 3. Open Serial Monitor at 115200 baud
// 4. Wait for measurements to start
// 5. Files will be created as: sensything_1.csv, sensything_2.csv, etc.
// 6. Data is buffered (10 lines) before writing to reduce SD wear
// 7. Place finger/wrist on PPG sensor for continuous data
//
// CSV Format (OX):
// timestamp,count,ir_raw,ir_capdac,red_raw,red_capdac,spo2_pct,spo2_capdac,hr_bpm,hr_capdac,status_flags
// 12345,42,234567,0,289456,0,97,0,68,0,0x00
//
// Commands (via Serial):
// - help: Show available commands
// - status: Display system status (includes SD info)
// - toggle_sd: Enable/disable SD logging
// - rotate_file: Force new file creation
// - set_rate <ms>: Change sample rate (e.g., "set_rate 8" for OX 125Hz)
//
// Board Configuration (CRITICAL):
// - Board: ESP32S3 Dev Module
// - USB Mode: Hardware CDC and JTAG
// - USB CDC on Boot: Enabled
// - PSRAM: QSPI PSRAM
// - Partition Scheme: Huge APP (3MB No OTA)
// - Upload Speed: 921600
//
// Required Libraries:
// - Sensything Platform (this library)
// - Protocentral_AFE44xx (via Arduino Library Manager)
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <SensythingCore.h>

// Notes:
// - OX sensor produces 125Hz data, which gets buffered efficiently
// - 10-line buffer = ~80ms of data at 125Hz before write
// - Files rotate every hour to keep individual files manageable
// - Watch Serial Monitor for SD write warnings
//
//////////////////////////////////////////////////////////////////////////////////////////

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
    
    // Initialize SD Card module
    if (!sensything.initSDCard()) {
        Serial.println("⚠️  SD Card initialization failed!");
        Serial.println("ℹ️  Check SD card insertion and SDIO connections");
        Serial.println("ℹ️  Continuing without SD logging...");
        Serial.println();
        // Don't halt - continue without SD logging
    } else {
        Serial.println("✓ SD Card initialized successfully");
        
        // Enable SD Card logging
        sensything.enableSDCard(true);
        Serial.println("💾 SD logging enabled");
        Serial.println();
    }
    
    // OX board runs at ~125Hz (8ms intervals) natively
    sensything.setSampleRate(8);
    
    // Start measurements
    sensything.startMeasurements();
    
    Serial.println("✓ Setup complete!");
    Serial.println();
    Serial.println("📋 SD Card Information:");
    Serial.println("   - Data logged to: sensything_N.csv files");
    Serial.println("   - Buffering: 10 measurements per write (efficient SD access)");
    Serial.println("   - File rotation: hourly");
    Serial.println("   - Sample rate: 125Hz (~8ms intervals)");
    Serial.println();
    Serial.println("🫀 Measurement Guide:");
    Serial.println("   - Place finger/wrist on PPG sensor for continuous recording");
    Serial.println("   - IR/RED: Raw PPG values (higher = better signal)");
    Serial.println("   - SpO2: Blood oxygen saturation percentage");
    Serial.println("   - HR: Heart rate in beats per minute");
    Serial.println();
    Serial.println("ℹ️  Type 'help' for available commands");
    Serial.println("ℹ️  Use 'status' to check SD card status");
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
            Serial.println();
            Serial.println("💾 SD Card Status (30-second update):");
            Serial.printf("   Measurements logged: %lu\n", sensything.getMeasurementCount());
            Serial.println("   Buffering strategy: 10 measurements per write");
            Serial.println("   File rotation: hourly");
            Serial.println();
        }
    }
}
