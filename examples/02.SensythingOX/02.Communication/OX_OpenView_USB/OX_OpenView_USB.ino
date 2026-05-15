// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Ashwin Whitchurch, Protocentral Electronics <info@protocentral.com>
//
// Sensything OX - OpenView2 USB Streaming Example
//
// Streams 4-channel PPG/SpO2/HR data over USB Serial as binary OpenView2
// packets (https://github.com/Protocentral/protocentral_openview2) instead of
// the default CSV format.
//
// Packet on the wire (per measurement):
//   0x0A 0xFA | len_LSB len_MSB | 0x02 | ir_LSB ir_MSB | red_LSB red_MSB
//                                       | spo2_LSB spo2_MSB | hr_LSB hr_MSB | 0x0B
// Each channel is a signed 16-bit little-endian integer. Note that the raw
// IR/RED ADC values from the AFE4400 are 19-bit and will be truncated when
// cast to int16 — this matches the existing BLE OpenView mapping.
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <SensythingCore.h>

SensythingOX sensything;

void setup() {
    if (!sensything.initPlatform()) {
        while (1) {
            delay(1000);
        }
    }

    // Switch USB stream to binary OpenView2 packets.
    sensything.setUSBFormat(USB_FORMAT_OPENVIEW);
}

void loop() {
    sensything.update();
}
