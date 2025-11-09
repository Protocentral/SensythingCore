//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    SensythingES3 - USB Serial Communication Module
//    Handles USB Serial streaming with CSV format and emoji prefixes
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
     * Set whether to use emoji prefixes
     * @param enable true to enable emojis, false to disable
     */
    void setUseEmojis(bool enable);
    
    /**
     * Set whether to include timestamps
     * @param enable true to include timestamps, false to disable
     */
    void setUseTimestamp(bool enable);
    
    /**
     * Set CSV separator character
     * @param separator Character to use (default: ',')
     */
    void setSeparator(char separator);
    
private:
    bool useEmojis;
    bool useTimestamp;
    char csvSeparator;
    
    /**
     * Format and print CSV header (called once at start)
     */
    void printCSVHeader(const BoardConfig& config);
    
    /**
     * Format a single measurement as CSV
     */
    String formatAsCSV(const MeasurementData& data, const BoardConfig& config);
    
    bool headerPrinted;
};

#endif // SENSYTHING_USB_H
