//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything Platform - USB Serial Communication Module Implementation
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "SensythingUSB.h"

SensythingUSB::SensythingUSB() {
    format = USB_FORMAT_CSV;
    useEmojis = SENSYTHING_USB_USE_EMOJIS;
    useTimestamp = SENSYTHING_USB_TIMESTAMP;
    csvSeparator = ',';
    headerPrinted = false;
}

void SensythingUSB::setFormat(SensythingUSBFormat fmt) {
    format = fmt;
    // Force CSV header to print again if we ever switch back to CSV
    headerPrinted = false;
}

void SensythingUSB::setUseEmojis(bool enable) {
    useEmojis = enable;
}

void SensythingUSB::setUseTimestamp(bool enable) {
    useTimestamp = enable;
}

void SensythingUSB::setSeparator(char separator) {
    csvSeparator = separator;
}

void SensythingUSB::printCSVHeader(const BoardConfig& config) {
    if (headerPrinted) return;

    String header = "";

    if (useEmojis) {
        header += String(EMOJI_DATA) + " ";
    }

    if (useTimestamp) {
        header += "timestamp" + String(csvSeparator);
    }

    // Add channel headers
    for (int i = 0; i < config.channelCount; i++) {
        header += String(config.channels[i].label);
        if (i < config.channelCount - 1) {
            header += csvSeparator;
        }
    }

    // Add metadata headers based on board type
    if (config.boardType == BOARD_TYPE_CAP) {
        for (int i = 0; i < config.channelCount; i++) {
            header += csvSeparator;
            header += "capdac_" + String(i);
        }
    }

    header += csvSeparator + "status_flags";
    header += csvSeparator + "count";

    Serial.println(header);
    headerPrinted = true;
}

String SensythingUSB::formatAsCSV(const MeasurementData& data, const BoardConfig& config) {
    String csv = "";

    if (useEmojis) {
        csv += String(EMOJI_DATA) + " ";
    }

    if (useTimestamp) {
        csv += String(data.timestamp) + csvSeparator;
    }

    // Add channel data
    for (int i = 0; i < data.channel_count; i++) {
        csv += String(data.channels[i], 4);  // 4 decimal places
        if (i < data.channel_count - 1) {
            csv += csvSeparator;
        }
    }

    // Add metadata based on board type
    if (config.boardType == BOARD_TYPE_CAP) {
        for (int i = 0; i < data.channel_count; i++) {
            csv += csvSeparator;
            csv += String(data.metadata[i]);
        }
    } else if (config.boardType == BOARD_TYPE_OX) {
        // For OX, metadata might contain different info
        // Can be extended based on needs
    }

    csv += csvSeparator + "0x" + String(data.status_flags, HEX);
    csv += csvSeparator + String(data.measurement_count);

    return csv;
}

void SensythingUSB::streamOpenViewPacket(const MeasurementData& data, const BoardConfig& config) {
    // Build payload: one int16 little-endian per channel.
    // Matches the BLE OpenView mapping in SensythingBLE::formatAsInt16Array().
    const uint8_t channelCount = config.channelCount;
    const uint16_t payloadLen = (uint16_t)channelCount * 2;

    // Static-sized buffer: 5 byte header + max payload + 1 byte stop.
    // Max channels = SENSYTHING_MAX_CHANNELS (4) → 6 + 8 = 14 bytes.
    uint8_t pkt[6 + SENSYTHING_MAX_CHANNELS * 2];

    pkt[0] = SENSYTHING_OPENVIEW_PKT_START_1;       // 0x0A
    pkt[1] = SENSYTHING_OPENVIEW_PKT_START_2;       // 0xFA
    pkt[2] = (uint8_t)(payloadLen & 0xFF);          // length LSB
    pkt[3] = (uint8_t)((payloadLen >> 8) & 0xFF);   // length MSB
    pkt[4] = SENSYTHING_OPENVIEW_PKT_TYPE_DATA;     // 0x02

    uint8_t idx = 5;
    for (uint8_t i = 0; i < channelCount; i++) {
        int16_t value;
        if (data.status_flags & (1 << i)) {
            value = 0;  // failed channel
        } else {
            value = (int16_t)(data.channels[i]);
        }
        pkt[idx++] = (uint8_t)(value & 0xFF);          // LSB
        pkt[idx++] = (uint8_t)((value >> 8) & 0xFF);   // MSB
    }
    pkt[idx++] = SENSYTHING_OPENVIEW_PKT_STOP;      // 0x0B

    Serial.write(pkt, idx);
}

void SensythingUSB::streamData(const MeasurementData& data, const BoardConfig& config) {
    if (format == USB_FORMAT_OPENVIEW) {
        streamOpenViewPacket(data, config);
        return;
    }

    // CSV path
    if (!headerPrinted) {
        printCSVHeader(config);
    }
    String csvLine = formatAsCSV(data, config);
    Serial.println(csvLine);
}
