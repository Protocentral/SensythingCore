// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2025 Ashwin Whitchurch, Protocentral Electronics <info@protocentral.com>
//
// Sensything Cap - OpenView2 USB Streaming Example
//
// Streams 4-channel capacitance data over USB Serial as binary OpenView2
// packets (https://github.com/Protocentral/protocentral_openview2) instead of
// the default CSV format.
//
// Packet on the wire (per measurement):
//   0x0A 0xFA | len_LSB len_MSB | 0x02 | ch0_LSB ch0_MSB | ch1_LSB ch1_MSB
//                                       | ch2_LSB ch2_MSB | ch3_LSB ch3_MSB | 0x0B
// Each channel is a signed 16-bit little-endian integer (truncated from pF).
//
// Pair this sketch with the Protocentral OpenView2 app and select the
// matching serial port.
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <SensythingCore.h>

SensythingCap sensything;

void setup() {
    if (!sensything.initPlatform()) {
        while (1) {
            delay(1000);
        }
    }

    // Switch USB stream to binary OpenView2 packets.
    // Note: after this point, avoid Serial.print() calls — they would corrupt
    // the binary stream the host parser is reading.
    sensything.setUSBFormat(USB_FORMAT_OPENVIEW);
}

void loop() {
    sensything.update();
}
