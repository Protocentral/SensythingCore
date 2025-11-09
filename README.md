# SensythingES3 - Arduino Library

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Arduino Library](https://img.shields.io/badge/Arduino-Library-blue.svg)](https://www.arduino.cc/reference/en/libraries/)

A unified platform library for the SensythingES3 family of ESP32-S3 sensor boards from Protocentral Electronics.

## Overview

The SensythingES3 library provides a consistent, modular API for all SensythingES3 boards, including:

- **SensythingES3 Cap**: 4-channel capacitance measurement using FDC1004 (Phase 1 ✅ Complete)
- **SensythingES3 OX**: PPG/SpO2/HR measurement using AFE4400 (Phase 4 ✅ Complete Infrastructure)

All boards share common ESP32-S3 hardware and communication infrastructure while supporting board-specific sensors through inheritance-based polymorphism.

## Features

- 📡 **USB Serial Streaming**: CSV format with emoji prefixes for easy debugging
- 📱 **BLE Streaming**: OPENVIEW protocol compatible (Phase 2)
- 🌐 **WiFi Streaming**: WebSocket real-time data + web dashboard (Phase 3)
- 💾 **SD Card Logging**: High-speed SDIO logging with CSV format (Phase 2)
- ⚙️ **Unified API**: Same code structure across all Sensything boards
- 🎛️ **Configurable**: Sample rate, interfaces, and output formats
- 🔧 **Command Interface**: Interactive commands via Serial/BLE/WiFi

## Installation

### Via Arduino Library Manager (Recommended)

1. Open Arduino IDE
2. Go to **Sketch > Include Library > Manage Libraries**
3. Search for "SensythingES3"
4. Click **Install**

### Manual Installation

1. Download the latest release from [GitHub](https://github.com/Protocentral/sensything_cap)
2. Extract to your Arduino libraries folder:
   - Windows: `Documents\Arduino\libraries\`
   - Mac: `~/Documents/Arduino/libraries/`
   - Linux: `~/Arduino/libraries/`
3. Restart Arduino IDE

## Quick Start - Sensything Cap

```cpp
#include <Sensything.h>

// Create Sensything Cap instance
SensythingCap sensything;

void setup() {
    // Initialize platform (sensor + USB streaming)
    sensything.initPlatform();
    
    // Optional: Set custom sample rate (default 10Hz)
    sensything.setSampleRate(50);  // 20Hz
}

void loop() {
    // Handle measurements and streaming
    sensything.update();
}
```

That's it! The board will now stream 4-channel capacitance data via USB Serial.

## Quick Start - Sensything OX

```cpp
#include <Sensything.h>

// Create Sensything OX instance
SensythingOX sensything;

void setup() {
    // Initialize platform (sensor + USB streaming)
    sensything.initPlatform();
    
    // Optional: Set fast sample rate for PPG (default 10Hz)
    sensything.setSampleRate(8);  // 125Hz (8ms interval)
}

void loop() {
    // Handle measurements and streaming
    sensything.update();
}
```

**Note**: OX board requires `Protocentral_AFE44xx` library. Install from Arduino Library Manager or manually copy the AFE44xx library to your Arduino libraries folder. See `docs/PHASE4_OX_IMPLEMENTATION.md` for details.

## Hardware Requirements

### Sensything Cap
- ESP32-S3-WROOM-1 module (8MB Flash, 2MB PSRAM)
- FDC1004 capacitance sensor (onboard)
- SD card slot with SDIO interface
- QWIIC I2C connectors
- USB-C for power and data

### Sensything OX
- ESP32-S3-WROOM-1 module (8MB Flash, 2MB PSRAM)
- AFE4400 PPG/SpO2 sensor (onboard)
- SD card slot with SDIO interface
- USB-C for power and data

### Arduino IDE Settings (ESP32-S3)
- Board: **ESP32S3 Dev Module**
- USB Mode: **Hardware CDC and JTAG**
- USB CDC on Boot: **Enabled**
- Upload Mode: **UART0 / Hardware CDC**
- PSRAM: **QSPI PSRAM**
- Partition Scheme: **Huge APP (3MB No OTA/1MB SPIFFS)**
- Upload Speed: **921600**

## Dependencies

### Required Libraries (Cap Board)
- [Protocentral_FDC1004](https://github.com/Protocentral/Protocentral_FDC1004) - FDC1004 sensor driver

### Required Libraries (OX Board)
- [Protocentral_AFE44xx](https://github.com/Protocentral/Protocentral_AFE44xx) - AFE4400 sensor driver ✅

### Optional (for Phase 2/3)
- WebSocketsServer - WebSocket communication (for WiFi dashboard)

Install via Arduino Library Manager.

## Examples

### 01.Basic
- **Cap_USB_Streaming**: Simple USB Serial streaming (Cap board)
- **OX_USB_Streaming**: Simple USB Serial streaming (OX board, requires AFE44xx library)

### 02.Communication (Phase 2)
- Cap_BLE_Streaming: BLE with OPENVIEW protocol
- Cap_WiFi_Dashboard: WebSocket + web interface

### 03.DataLogging (Phase 2)
- Cap_SD_Logger: High-speed SD card logging

### 04.Advanced (Phase 2)
- Cap_MultiInterface: All interfaces simultaneously

## API Reference

### Core Methods

```cpp
// Initialization
bool initPlatform();                    // Initialize everything
bool initSensorOnly();                  // Sensor only, no communication

// Interface Control
void enableUSB(bool enable);
void enableBLE(bool enable);
void enableWiFi(bool enable, const char* ssid = nullptr, const char* password = nullptr);
void enableSDCard(bool enable);
void enableAll();
void disableAll();

// Measurement Control
bool setSampleRate(unsigned long intervalMs);  // 20-10000ms
float getSampleRateHz();
void startMeasurements();
void stopMeasurements();
void resetMeasurementCount();

// Main Loop
void update();  // Call in loop()

// Commands & Status
void processCommand(String cmd);
void printStatus();
void printHelp();
```

### Available Serial Commands

Type these in Serial Monitor (115200 baud, Newline):

- `help` - Show available commands
- `status` - Display system status
- `start_all` - Enable all interfaces
- `stop_all` - Disable all interfaces
- `set_rate <ms>` - Set sample rate (e.g., `set_rate 100`)
- `reset_count` - Reset measurement counter

## Data Format

### USB Serial Output (CSV with Emoji)
```
📊 timestamp,ch0_pf,ch1_pf,ch2_pf,ch3_pf,capdac_0,capdac_1,capdac_2,capdac_3,status_flags,count
📊 1523,12.3456,15.6789,10.2345,13.4567,5,5,5,5,0x00,1
📊 1623,12.3478,15.6801,10.2367,13.4589,5,5,5,5,0x00,2
```

### Status Flags (Hex Bitmask)
- `0x01` - Channel 0 measurement failed
- `0x02` - Channel 1 measurement failed
- `0x04` - Channel 2 measurement failed
- `0x08` - Channel 3 measurement failed
- `0x40` - CAPDAC adjusting (normal during stabilization)

## Architecture

The library uses an object-oriented design with:

- **SensythingCore**: Abstract base class with common functionality
- **SensythingCap**: Board-specific implementation for Cap
- **Communication Modules**: Pluggable USB/BLE/WiFi/SD modules
- **Unified Data Structures**: Common measurement format across all boards

This allows easy addition of new boards while maintaining API consistency.

## Development Status

- ✅ **Phase 1 Complete**: Core library + USB streaming for Cap
- 🔨 **Phase 2 In Progress**: BLE, WiFi, SD Card modules
- 📅 **Phase 3 Planned**: Sensything OX support

## Troubleshooting

### Sensor Not Detected
- Verify I2C connections (SDA=GPIO21, SCL=GPIO22)
- Check power supply (3.3V, sufficient current)
- Try scanning I2C bus with [I2C Scanner](https://playground.arduino.cc/Main/I2cScanner/)

### Compilation Errors
- Ensure ESP32 board support installed (Arduino Boards Manager)
- Update to latest ESP32 Arduino Core (2.0.0+)
- Verify all dependencies installed

### Upload Failures
- Press and hold BOOT button during upload
- Check USB cable (data-capable, not charge-only)
- Try lower upload speed (115200)

## License

**Software**: MIT License  
**Hardware**: CERN-OHL-P v2
