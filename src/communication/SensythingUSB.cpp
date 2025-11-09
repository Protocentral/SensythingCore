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
    useEmojis = SENSYTHING_USB_USE_EMOJIS;
    useTimestamp = SENSYTHING_USB_TIMESTAMP;
    csvSeparator = ',';
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

void SensythingUSB::streamData(const MeasurementData& data, const BoardConfig& config) {
    // Print header once
    if (!headerPrinted) {
        printCSVHeader(config);
    }
    
    // Format and print data
    String csvLine = formatAsCSV(data, config);
    Serial.println(csvLine);
}
