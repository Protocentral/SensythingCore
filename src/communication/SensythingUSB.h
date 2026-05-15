//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    SensythingES3 - USB Serial Communication Module
//    Streams measurements over USB Serial as CSV text or as a binary
//    OpenView2 packet (https://github.com/Protocentral/protocentral_openview2)
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#ifndef SENSYTHING_USB_H
#define SENSYTHING_USB_H

#include <Arduino.h>
#include "../core/SensythingTypes.h"
#include "../core/SensythingConfig.h"

class SensythingUSB {
public:
    SensythingUSB();

    /**
     * Stream measurement data to USB Serial
     * @param data Measurement data to stream
     * @param config Board configuration for formatting
     */
    void streamData(const MeasurementData& data, const BoardConfig& config);

    /**
     * Select the wire format used by streamData().
     * USB_FORMAT_CSV      : human-readable CSV (default)
     * USB_FORMAT_OPENVIEW : binary OpenView2 packet
     */
    void setFormat(SensythingUSBFormat fmt);
    SensythingUSBFormat getFormat() const { return format; }

    /**
     * Set whether to use emoji prefixes (CSV format only)
     */
    void setUseEmojis(bool enable);

    /**
     * Set whether to include timestamps (CSV format only)
     */
    void setUseTimestamp(bool enable);

    /**
     * Set CSV separator character
     */
    void setSeparator(char separator);

private:
    SensythingUSBFormat format;
    bool useEmojis;
    bool useTimestamp;
    char csvSeparator;
    bool headerPrinted;

    void printCSVHeader(const BoardConfig& config);
    String formatAsCSV(const MeasurementData& data, const BoardConfig& config);

    // OpenView2 packet: 0x0A 0xFA | len LSB | len MSB | 0x02 | payload | 0x0B
    // Payload is channelCount * int16 little-endian (matches BLE OpenView mapping).
    void streamOpenViewPacket(const MeasurementData& data, const BoardConfig& config);
};

#endif // SENSYTHING_USB_H
