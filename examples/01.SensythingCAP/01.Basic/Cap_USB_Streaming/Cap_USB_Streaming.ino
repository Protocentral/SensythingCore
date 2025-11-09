// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Ashwin Whitchurch, Protocentral Electronics <info@protocentral.com>
//
// Sensything Cap - Basic USB Streaming Example
//
// This example demonstrates the simplest usage of the Sensything Platform library
// with the Sensything Cap board. It streams 4-channel capacitance measurements
// to the USB Serial port in CSV format.
//
// Hardware Required:
// - Protocentral Sensything Cap board (ESP32-S3 with FDC1004)
// - USB cable for power and data
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
// - Protocentral_FDC1004 (via Arduino Library Manager)
//
//    Serial Monitor Settings:
//    - Baud Rate: 115200
//    - Line Ending: Newline
//
//    Available Commands (type in Serial Monitor):
//    - help          : Show available commands
//    - status        : Show system status
//    - set_rate <ms> : Set sample rate (e.g., "set_rate 100" for 10Hz)
//    - reset_count   : Reset measurement counter
//    - start_all     : Start all interfaces
//    - stop_all      : Stop all interfaces
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <SensythingCore.h>

// Create Sensything Cap instance
SensythingCap sensything;

void setup() {
    // Initialize the platform (sensor + USB communication)
    if (!sensything.initPlatform()) {
        // If initialization fails, halt the program
        while (1) {
            delay(1000);
        }
    }
    
    // Optional: Configure custom sample rate
    // sensything.setSampleRate(50);  // 20Hz (50ms interval)
    
    // USB streaming is enabled by default after initPlatform()
    // You can explicitly enable/disable it:
    // sensything.enableUSB(true);
}

void loop() {
    sensything.update();
    
    // The update() function automatically:
    // - Checks if it's time for a new measurement
    // - Reads data from the FDC1004 sensor
    // - Streams data to enabled interfaces (USB in this example)
    // - Processes any serial commands
}