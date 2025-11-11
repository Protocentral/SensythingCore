//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything Platform - Unified Type Definitions
//    Common data structures and types for all Sensything boards
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
//   INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
//   PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
//   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
//   OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
//   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////////////////////

#ifndef SENSYTHING_TYPES_H
#define SENSYTHING_TYPES_H

#include <Arduino.h>

// =================================================================================================
// CONSTANTS
// =================================================================================================

#define SENSYTHING_MAX_CHANNELS 4      // Maximum number of data channels
#define SENSYTHING_MAX_METADATA 8      // Maximum metadata bytes
#define SENSYTHING_MAX_LABEL_LEN 32    // Maximum label length

// =================================================================================================
// BOARD TYPES
// =================================================================================================

enum SensythingBoardType {
    BOARD_TYPE_UNKNOWN = 0,
    BOARD_TYPE_CAP     = 1,    // FDC1004 Capacitance sensor
    BOARD_TYPE_OX      = 2,    // AFE4400 PPG/SpO2 sensor
    BOARD_TYPE_CUSTOM  = 99    // Custom/future boards
};

// =================================================================================================
// COMMUNICATION INTERFACE FLAGS
// =================================================================================================

enum SensythingInterface {
    INTERFACE_NONE    = 0x00,
    INTERFACE_USB     = 0x01,
    INTERFACE_BLE     = 0x02,
    INTERFACE_WIFI    = 0x04,
    INTERFACE_SD_CARD = 0x08,
    INTERFACE_ALL     = 0x0F
};

// =================================================================================================
// STATUS FLAGS
// =================================================================================================

// Channel failure flags (bits 0-3)
#define SENSYTHING_STATUS_CH0_FAIL      0x01
#define SENSYTHING_STATUS_CH1_FAIL      0x02
#define SENSYTHING_STATUS_CH2_FAIL      0x04
#define SENSYTHING_STATUS_CH3_FAIL      0x08

// Cap board specific flags
#define SENSYTHING_STATUS_CAPDAC_ADJ    0x10     // CAPDAC out of range (adjusting)

// OX board specific flags
#define SENSYTHING_STATUS_BUFFER_OVERFLOW 0x20   // AFE4400 buffer overflow
#define SENSYTHING_STATUS_NO_SIGNAL       0x40   // No valid PPG signal detected
#define SENSYTHING_STATUS_INVALID_DATA    0x80   // Invalid SpO2/HR calculation

// =================================================================================================
// MEASUREMENT DATA STRUCTURE
// =================================================================================================

typedef struct {
    uint32_t timestamp;                          // Milliseconds since boot
    float channels[SENSYTHING_MAX_CHANNELS];     // Measurement data (units vary by board)
    uint8_t metadata[SENSYTHING_MAX_METADATA];   // Additional data (CAPDAC, etc.)
    uint32_t measurement_count;                  // Sequential measurement number
    uint8_t channel_count;                       // Number of active channels
    uint8_t status_flags;                        // Status bits (see SENSYTHING_STATUS_* defines)
} MeasurementData;

// =================================================================================================
// SYSTEM STATE STRUCTURE
// =================================================================================================

typedef struct {
    // Timing control
    unsigned long sampleInterval;                // Sample interval in milliseconds
    unsigned long lastMeasurement;               // Timestamp of last measurement
    unsigned long lastStatusUpdate;              // Timestamp of last status update
    unsigned long lastFileRotation;              // Timestamp of last SD file rotation
    
    // Measurement control
    bool measurementActive;                      // Master enable/disable
    uint32_t measurementCount;                   // Total measurements taken
    
    // Communication interface flags
    bool usbStreamingEnabled;                    // USB Serial streaming
    bool bleStreamingEnabled;                    // BLE streaming
    bool wifiStreamingEnabled;                   // WiFi streaming
    bool sdLoggingEnabled;                       // SD card logging
    bool mqttStreamingEnabled;                   // MQTT streaming
    
    // Connection status
    bool bleConnected;                           // BLE client connected
    bool wifiConnected;                          // WiFi connected (STA or AP)
    bool mqttConnected;                          // MQTT broker connected
    bool sdCardReady;                            // SD card mounted and ready
    
    // Data management
    String currentFileName;                      // Current SD log file
    uint32_t fileCount;                          // Number of files created
    
} SystemState;

// =================================================================================================
// SYSTEM STATUS STRUCTURE (for reporting)
// =================================================================================================

typedef struct {
    SensythingBoardType boardType;               // Board type identifier
    String boardName;                            // Human-readable board name
    String sensorType;                           // Sensor description
    String firmwareVersion;                      // Firmware version string
    
    unsigned long uptimeSeconds;                 // System uptime
    float sampleRateHz;                          // Current sample rate
    uint32_t totalMeasurements;                  // Total measurements
    
    uint8_t activeInterfaces;                    // Bitmask of active interfaces
    bool bleConnected;
    bool wifiConnected;
    bool mqttConnected;
    bool sdCardReady;
    
    String currentSDFile;                        // Current log file name
    uint32_t sdFileSize;                         // Current file size in bytes
} SystemStatus;

// =================================================================================================
// CHANNEL INFORMATION STRUCTURE
// =================================================================================================

typedef struct {
    char label[SENSYTHING_MAX_LABEL_LEN];       // Channel label (e.g., "Channel 0", "SpO2")
    char unit[16];                               // Unit string (e.g., "pF", "%", "bpm")
    float minValue;                              // Expected minimum value
    float maxValue;                              // Expected maximum value
    bool active;                                 // Channel is active
} ChannelInfo;

// =================================================================================================
// BOARD CONFIGURATION STRUCTURE
// =================================================================================================

typedef struct {
    SensythingBoardType boardType;
    String boardName;
    String sensorType;
    uint8_t channelCount;
    ChannelInfo channels[SENSYTHING_MAX_CHANNELS];
    
    // Pin configuration (board-specific)
    int8_t i2c_sda;
    int8_t i2c_scl;
    int8_t spi_cs;
    int8_t spi_mosi;
    int8_t spi_miso;
    int8_t spi_sck;
    
    // Sample rate limits
    unsigned long minSampleInterval;             // Minimum interval in ms
    unsigned long maxSampleInterval;             // Maximum interval in ms
} BoardConfig;

#endif // SENSYTHING_TYPES_H
