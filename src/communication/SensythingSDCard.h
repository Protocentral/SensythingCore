//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    SensythingES3 - SD Card Communication Module
//    SDIO-based CSV data logging with buffered writes
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#ifndef SENSYTHING_SDCARD_H
#define SENSYTHING_SDCARD_H

#include <Arduino.h>
#include <SD_MMC.h>
#include "../core/SensythingTypes.h"
#include "../core/SensythingConfig.h"

class SensythingSDCard {
public:
    SensythingSDCard();
    ~SensythingSDCard();
    
    /**
     * Initialize SD card with SDIO interface
     * @return true if initialization successful
     */
    bool init();
    
    /**
     * Check if SD card is ready
     * @return true if card is mounted and ready
     */
    bool isReady() const { return cardReady; }
    
    /**
     * Log measurement data to SD card
     * @param data Measurement data structure
     * @param config Board configuration for format adaptation
     */
    void logData(const MeasurementData& data, const BoardConfig& config);
    
    /**
     * Force flush of buffer to file
     */
    void flush();
    
    /**
     * Rotate to new file (creates new timestamped file)
     * @return true if successful
     */
    bool rotateFile();
    
    /**
     * Get current file name
     * @return Current log file name
     */
    String getCurrentFileName() const { return currentFileName; }
    
    /**
     * Get number of files created
     * @return File counter
     */
    uint32_t getFileCount() const { return fileCount; }
    
    /**
     * Get buffer fill level
     * @return Number of lines in buffer (0-SENSYTHING_SD_BUFFER_LINES)
     */
    uint8_t getBufferLevel() const { return bufferLineCount; }
    
    /**
     * Get total lines written to current file
     * @return Line counter
     */
    uint32_t getLinesWritten() const { return linesWritten; }
    
private:
    bool cardReady;
    bool fileOpen;
    bool headerWritten;
    
    String currentFileName;
    uint32_t fileCount;
    uint32_t linesWritten;
    
    // Buffer for batch writes
    String buffer;
    uint8_t bufferLineCount;
    
    /**
     * Create new log file with timestamp
     * @param config Board configuration for header
     * @return true if successful
     */
    bool createNewFile(const BoardConfig& config);
    
    /**
     * Write CSV header to file
     * @param config Board configuration
     */
    void writeHeader(const BoardConfig& config);
    
    /**
     * Format measurement data as CSV line
     * @param data Measurement data
     * @param config Board configuration
     * @return CSV string
     */
    String formatAsCSV(const MeasurementData& data, const BoardConfig& config);
    
    /**
     * Write buffer to file if buffer is full or forced
     * @param force Force write even if buffer not full
     */
    void writeBuffer(bool force = false);
};

#endif // SENSYTHING_SDCARD_H
