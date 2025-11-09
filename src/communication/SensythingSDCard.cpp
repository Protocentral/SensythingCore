//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything Platform - SD Card Communication Module
//    SDIO-based CSV data logging with buffered writes
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "SensythingSDCard.h"

SensythingSDCard::SensythingSDCard() {
    cardReady = false;
    fileOpen = false;
    headerWritten = false;
    fileCount = 0;
    linesWritten = 0;
    bufferLineCount = 0;
    currentFileName = "";
    buffer = "";
}

SensythingSDCard::~SensythingSDCard() {
    if (cardReady) {
        flush();  // Write remaining buffer data
        SD_MMC.end();
    }
}

bool SensythingSDCard::init() {
    if (cardReady) {
        Serial.println(String(EMOJI_WARNING) + " SD Card already initialized");
        return true;
    }
    
    Serial.print(String(EMOJI_STORAGE) + " Initializing SD Card (SDIO)... ");
    
    // Initialize SD_MMC with 1-bit mode (uses only D0 line for better compatibility)
    // 4-bit mode can be enabled by passing true as second parameter
    if (!SD_MMC.begin("/sdcard", true)) {  // true = 1-bit mode
        Serial.println(String(EMOJI_ERROR) + " Failed");
        Serial.println(String(EMOJI_INFO) + " Check SD card insertion and SDIO connections");
        return false;
    }
    
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println(String(EMOJI_ERROR) + " No SD card detected");
        SD_MMC.end();
        return false;
    }
    
    // Print card info
    Serial.println(String(EMOJI_SUCCESS) + " Success");
    Serial.print(String(EMOJI_INFO) + " Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }
    
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("%s Card Size: %lluMB\n", EMOJI_INFO, cardSize);
    
    uint64_t usedBytes = SD_MMC.usedBytes() / (1024 * 1024);
    Serial.printf("%s Used Space: %lluMB\n", EMOJI_INFO, usedBytes);
    
    cardReady = true;
    return true;
}

void SensythingSDCard::logData(const MeasurementData& data, const BoardConfig& config) {
    if (!cardReady) {
        return;  // Card not ready, skip silently
    }
    
    // Create file if needed
    if (!fileOpen) {
        if (!createNewFile(config)) {
            Serial.println(String(EMOJI_ERROR) + " Failed to create log file");
            cardReady = false;  // Disable logging on file error
            return;
        }
    }
    
    // Format data as CSV and add to buffer
    buffer += formatAsCSV(data, config);
    buffer += "\n";
    bufferLineCount++;
    
    // Write buffer if full
    if (bufferLineCount >= SENSYTHING_SD_BUFFER_LINES) {
        writeBuffer(false);
    }
}

void SensythingSDCard::flush() {
    if (bufferLineCount > 0) {
        writeBuffer(true);
    }
}

bool SensythingSDCard::rotateFile() {
    if (!cardReady) {
        return false;
    }
    
    // Flush current buffer
    flush();
    
    // Close current file (by resetting state)
    fileOpen = false;
    headerWritten = false;
    linesWritten = 0;
    
    Serial.println(String(EMOJI_STORAGE) + " File rotation triggered");
    
    return true;
}

bool SensythingSDCard::createNewFile(const BoardConfig& config) {
    // Generate filename with timestamp
    fileCount++;
    currentFileName = String(SENSYTHING_SD_FILE_PREFIX) + String(fileCount) + ".csv";
    
    Serial.print(String(EMOJI_STORAGE) + " Creating file: ");
    Serial.println(currentFileName);
    
    // Write header if enabled
    if (SENSYTHING_SD_CSV_HEADER && !headerWritten) {
        writeHeader(config);
        headerWritten = true;
    }
    
    fileOpen = true;
    linesWritten = 0;
    
    return true;
}

void SensythingSDCard::writeHeader(const BoardConfig& config) {
    File file = SD_MMC.open("/" + currentFileName, FILE_APPEND);
    if (!file) {
        Serial.println(String(EMOJI_ERROR) + " Failed to open file for header");
        return;
    }
    
    // Build CSV header
    String header = "timestamp,count";
    
    for (int i = 0; i < config.channelCount; i++) {
        header += ",";
        header += config.channels[i].label;
        
        // Add metadata column for Cap board (CAPDAC)
        if (config.boardType == BOARD_TYPE_CAP) {
            header += ",";
            header += config.channels[i].label;
            header += "_capdac";
        }
    }
    
    header += ",status_flags\n";
    
    file.print(header);
    file.close();
    
    Serial.println(String(EMOJI_SUCCESS) + " Header written");
}

String SensythingSDCard::formatAsCSV(const MeasurementData& data, const BoardConfig& config) {
    String csv = String(data.timestamp);
    csv += ",";
    csv += String(data.measurement_count);
    
    // Add channel data
    for (int i = 0; i < config.channelCount; i++) {
        csv += ",";
        
        // Check if channel is valid
        bool channelValid = !(data.status_flags & (1 << i));
        if (channelValid) {
            csv += String(data.channels[i], 4);  // 4 decimal places
        } else {
            csv += "NaN";
        }
        
        // Add metadata for Cap board
        if (config.boardType == BOARD_TYPE_CAP) {
            csv += ",";
            csv += String(data.metadata[i]);
        }
    }
    
    // Add status flags
    csv += ",";
    csv += String(data.status_flags, HEX);
    
    return csv;
}

void SensythingSDCard::writeBuffer(bool force) {
    if (!cardReady || bufferLineCount == 0) {
        return;
    }
    
    if (!force && bufferLineCount < SENSYTHING_SD_BUFFER_LINES) {
        return;  // Buffer not full yet
    }
    
    // Open file in append mode
    File file = SD_MMC.open("/" + currentFileName, FILE_APPEND);
    if (!file) {
        Serial.println(String(EMOJI_ERROR) + " Failed to open file for writing");
        cardReady = false;  // Disable logging on error
        return;
    }
    
    // Write buffer
    file.print(buffer);
    linesWritten += bufferLineCount;
    
    file.close();
    
    // Clear buffer
    buffer = "";
    bufferLineCount = 0;
}
