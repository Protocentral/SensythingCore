# SensythingES3 - Drivers Directory

## Overview

SensythingES3 now uses published Protocentral Arduino libraries for all sensor drivers. This provides consistency, maintainability, and easy updates.

**Local driver files have been removed** - the platform depends entirely on published libraries for sensor communication.

## Currently Used Published Libraries

### FDC1004 Capacitance Sensor

**Library**: `Protocentral_FDC1004` (Arduino Library Manager)

**Purpose**: 4-channel capacitive touch/proximity sensing

**Features**:
- I2C communication (0x50 default address)
- 4-channel simultaneous measurement
- Integrated CAPDAC for baseline adjustment
- Up to 100 measurements per second

**Integration**:
- Used by `SensythingES3 Cap` board class
- Automatically installed as a dependency

### AFE4490/AFE4400 Pulse Oximetry Sensor

**Library**: `ProtoCentral AFE4490 PPG and SpO2 boards` (Arduino Library Manager)

**Header**: `#include <protocentral_afe44xx.h>`

**Purpose**: PPG (Photoplethysmograph) acquisition and SpO2 calculation

**Features**:
- SPI communication interface (2 MHz)
- Complete PPG waveform capture (IR & RED)
- Automatic SpO2 percentage calculation (70-100%)
- Heart rate detection (40-250 bpm)
- MAXIM Integrated algorithm implementation
- Support for both AFE4490 and AFE4400 sensors

**Integration**:
- Used by `SensythingES3 OX` board class
- Automatically installed as a dependency

## Local Driver Reference (Legacy)

The `AFE44xx.h/.cpp` files in this directory are **retained as reference implementations** but are **NOT used by the platform**. The published `ProtoCentral AFE4490 PPG and SpO2 boards` library is included instead.

If you need to modify or customize driver behavior:
1. Fork the [Protocentral AFE4490 Arduino library](https://github.com/Protocentral/protocentral-afe4490-arduino)
2. Modify SensythingOX board class to use your custom library version
3. Update library.properties dependencies accordingly

## References

- Protocentral FDC1004 Library: https://github.com/Protocentral/Protocentral_fdc1004_breakout
- Protocentral AFE4490 Library: https://github.com/Protocentral/protocentral-afe4490-arduino
- AFE4400/AFE4490 Datasheets: https://www.ti.com/product/AFE4400

---

Part of the Sensything Platform - Unified Arduino library for ESP32-S3 sensor boards
