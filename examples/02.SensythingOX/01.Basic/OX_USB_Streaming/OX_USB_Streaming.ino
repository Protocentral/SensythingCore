//////////////////////////////////////////////////////////////////////////////////////////
// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Ashwin Whitchurch, Protocentral Electronics <info@protocentral.com>
//
// Sensything OX - Basic USB Streaming Example
// ==================================================================================
// This example demonstrates the simplest usage of the Sensything Platform library
// with the Sensything OX board. It streams 4-channel PPG/SpO2/Heart Rate measurements
// to the USB Serial port in CSV format.
//
// Hardware Required:
// - Protocentral Sensything OX board (ESP32-S3 with AFE4400)
// - USB cable for power and data
// - Optional: Finger/wrist contact with PPG sensor
//
// Arduino IDE Setup:
// - Board: "ESP32S3 Dev Module"
// - USB Mode: "Hardware CDC and JTAG"
// - USB CDC on Boot: "Enabled"
// - Upload Mode: "UART0 / Hardware CDC"
// - PSRAM: "QSPI PSRAM"
// - Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"
// - Upload Speed: 921600
//
// Required Libraries:
// - Sensything Platform (this library)
//    - Protocentral_AFE44xx (via Arduino Library Manager)
//
//    Serial Monitor Settings:
//    - Baud Rate: 115200
//    - Line Ending: Newline
//
//    Available Commands (type in Serial Monitor):
//    - help          : Show available commands
//    - status        : Show system status
//    - set_rate <ms> : Set sample rate (e.g., "set_rate 8" for 125Hz)
//    - reset_count   : Reset measurement counter
//    - start_all     : Start all interfaces
//    - stop_all      : Stop all interfaces
//
//    Measurement Notes:
//    - AFE4400 runs at approximately 125Hz (8ms sample period)
//    - IR/RED raw values are 19-bit ADC readings (0-524288)
//    - SpO2 values are percentages (70-100% normal range)
//    - Heart rate values are in beats per minute (40-200 bpm typical)
//    - Place finger/wrist on sensor for best results
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <SensythingES3.h>

// Create Sensything OX instance
SensythingOX sensything;

void setup() {
    // Initialize the platform (sensor + USB communication)
    // This will:
    // 1. Initialize Serial at 115200 baud
    // 2. Initialize the AFE4400 sensor on SPI
    // 3. Enable USB streaming by default
    // 4. Start measurements at 100ms (10Hz) default rate
    if (!sensything.initPlatform()) {
        // If initialization fails, halt the program
        while (1) {
            delay(1000);
        }
    }
    
    // Optional: Configure custom sample rate
    // For PPG measurements, faster rates are recommended:
    // sensything.setSampleRate(8);   // 125Hz (8ms interval) - maximum for AFE4400
    // sensything.setSampleRate(20);  // 50Hz (20ms interval)
    // sensything.setSampleRate(40);  // 25Hz (40ms interval)
    
    // USB streaming is enabled by default after initPlatform()
    // You can explicitly enable/disable it:
    // sensything.enableUSB(true);
}

void loop() {
    // Main update function - handles timing, measurements, and streaming
    // This must be called repeatedly in the loop()
    sensything.update();
    
    // The update() function automatically:
    // - Checks if it's time for a new measurement
    // - Reads PPG data from the AFE4400 sensor
    // - Calculates SpO2 and heart rate
    // - Streams data to enabled interfaces (USB in this example)
    // - Processes any serial commands
    
    // No additional code needed for basic streaming!
}