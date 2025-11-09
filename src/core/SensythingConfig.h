//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything Platform - Configuration Constants
//    Platform-wide configuration, pin definitions, and constants
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

#ifndef SENSYTHING_CONFIG_H
#define SENSYTHING_CONFIG_H

// =================================================================================================
// FIRMWARE VERSION
// =================================================================================================

#define SENSYTHING_ES3_VERSION "1.0.0"
#define SENSYTHING_ES3_NAME "SensythingES3"

// =================================================================================================
// HARDWARE PLATFORM
// =================================================================================================

#define SENSYTHING_MCU "ESP32-S3-WROOM-1"
#define SENSYTHING_FLASH_SIZE_MB 8
#define SENSYTHING_PSRAM_SIZE_MB 2

// =================================================================================================
// COMMON PIN DEFINITIONS (ESP32-S3)
// =================================================================================================

// I2C Interface (FDC1004 + QWIIC)
// Try default ESP32-S3 I2C pins first
#define SENSYTHING_I2C_SDA 8
#define SENSYTHING_I2C_SCL 9
#define SENSYTHING_I2C_FREQ 400000  // 400kHz Fast Mode

// SDIO Interface (SD Card)
#define SENSYTHING_SDIO_CMD  41
#define SENSYTHING_SDIO_CLK  38
#define SENSYTHING_SDIO_D0   39
#define SENSYTHING_SDIO_D1   40
#define SENSYTHING_SDIO_D2   47
#define SENSYTHING_SDIO_D3   42

// SPI Interface (for boards using SPI sensors like AFE4400)
#define SENSYTHING_SPI_MOSI  11
#define SENSYTHING_SPI_MISO  13
#define SENSYTHING_SPI_SCK   12

// =================================================================================================
// DEFAULT TIMING CONFIGURATION
// =================================================================================================

#define SENSYTHING_DEFAULT_SAMPLE_INTERVAL_MS 100     // 10Hz default
#define SENSYTHING_MIN_SAMPLE_INTERVAL_MS 20          // 50Hz maximum
#define SENSYTHING_MAX_SAMPLE_INTERVAL_MS 10000       // 0.1Hz minimum

#define SENSYTHING_STATUS_UPDATE_INTERVAL_MS 10000    // Status update every 10s
#define SENSYTHING_FILE_ROTATION_INTERVAL_MS 3600000  // New file every hour

// =================================================================================================
// SD CARD CONFIGURATION
// =================================================================================================

#define SENSYTHING_SD_BUFFER_LINES 10                 // Flush after 10 lines
#define SENSYTHING_SD_CSV_HEADER true                 // Include CSV header
#define SENSYTHING_SD_FILE_PREFIX "sensything_"       // File name prefix

// =================================================================================================
// WIFI CONFIGURATION
// =================================================================================================

#define SENSYTHING_WIFI_CONNECT_TIMEOUT_MS 10000      // WiFi connection timeout
#define SENSYTHING_WIFI_AP_CHANNEL 1                  // AP mode channel
#define SENSYTHING_WIFI_AP_MAX_CONNECTIONS 4          // Maximum AP clients

// Default WiFi credentials (should be overridden by user)
#define SENSYTHING_DEFAULT_WIFI_SSID "YourWiFiNetwork"
#define SENSYTHING_DEFAULT_WIFI_PASSWORD "YourPassword"
#define SENSYTHING_DEFAULT_AP_SSID "SensythingCap-Setup"
#define SENSYTHING_DEFAULT_AP_PASSWORD "sensything123"

// Web server configuration
#define SENSYTHING_WEB_SERVER_PORT 80                 // HTTP server port
#define SENSYTHING_WEBSOCKET_PORT 81                  // WebSocket port

// =================================================================================================
// BLE CONFIGURATION
// =================================================================================================

// OPENVIEW Compatible Service UUIDs (primary - used by default)
#define SENSYTHING_BLE_SERVICE_UUID "0001A7D3-D8A4-4FEA-8174-1736E808C066"
#define SENSYTHING_BLE_DATA_CHAR_UUID "0002A7D3-D8A4-4FEA-8174-1736E808C066"

// Legacy aliases for backward compatibility
#define SENSYTHING_BLE_SERVICE_UUID_CAP SENSYTHING_BLE_SERVICE_UUID
#define SENSYTHING_BLE_CHAR_UUID_CAP    SENSYTHING_BLE_DATA_CHAR_UUID

// Standard Health Service UUIDs (for Sensything OX - Heart Rate & SpO2)
#define SENSYTHING_BLE_HEARTRATE_SERVICE_UUID     0x180D
#define SENSYTHING_BLE_HEARTRATE_CHAR_UUID        0x2A37
#define SENSYTHING_BLE_SPO2_SERVICE_UUID          0x1822
#define SENSYTHING_BLE_SPO2_CHAR_UUID             0x2A5E

// Custom UUID for HRV/PPG data stream
#define SENSYTHING_BLE_HRV_SERVICE_UUID     "cd5c7491-4448-7db8-ae4c-d1da8cba36d0"
#define SENSYTHING_BLE_HRV_CHAR_UUID        "cd5c1525-4448-7db8-ae4c-d1da8cba36d0"
#define SENSYTHING_BLE_DATASTREAM_SERVICE_UUID 0x1122
#define SENSYTHING_BLE_DATASTREAM_CHAR_UUID    0x1424

// BLE device name format
#define SENSYTHING_BLE_NAME_PREFIX "Sensything-"

// =================================================================================================
// USB SERIAL CONFIGURATION
// =================================================================================================

#define SENSYTHING_SERIAL_BAUD_RATE 115200
#define SENSYTHING_SERIAL_DATA_BITS SERIAL_8N1
#define SENSYTHING_SERIAL_TIMEOUT_MS 1000

// =================================================================================================
// DATA FORMAT CONFIGURATION
// =================================================================================================

// USB Serial format
#define SENSYTHING_USB_FORMAT_CSV true
#define SENSYTHING_USB_USE_EMOJIS true
#define SENSYTHING_USB_TIMESTAMP true

// JSON configuration
#define SENSYTHING_JSON_BUFFER_SIZE 512
#define SENSYTHING_JSON_PRECISION 4  // Decimal places for float values

// =================================================================================================
// STATUS FLAGS
// =================================================================================================

// Status flag bits (used in status_flags field)
#define SENSYTHING_STATUS_CH0_FAIL   0x01
#define SENSYTHING_STATUS_CH1_FAIL   0x02
#define SENSYTHING_STATUS_CH2_FAIL   0x04
#define SENSYTHING_STATUS_CH3_FAIL   0x08
#define SENSYTHING_STATUS_SENSOR_ERR 0x10
#define SENSYTHING_STATUS_OVERFLOW   0x20
#define SENSYTHING_STATUS_CAPDAC_ADJ 0x40
#define SENSYTHING_STATUS_RESERVED   0x80

// =================================================================================================
// EMOJI PREFIXES (for user-friendly serial output)
// =================================================================================================

#define EMOJI_SUCCESS "✓"
#define EMOJI_ERROR "✗"
#define EMOJI_DATA "📊"
#define EMOJI_STORAGE "💾"
#define EMOJI_NETWORK "📡"
#define EMOJI_BLUETOOTH "📱"
#define EMOJI_WIFI "🌐"
#define EMOJI_CONFIG "⚙️"
#define EMOJI_WARNING "⚠️"
#define EMOJI_INFO "ℹ️"
#define EMOJI_TIME "⏱️"

// =================================================================================================
// COMMAND STRINGS
// =================================================================================================

#define CMD_START_ALL "start_all"
#define CMD_STOP_ALL "stop_all"
#define CMD_STATUS "status"
#define CMD_HELP "help"
#define CMD_RESET_COUNT "reset_count"
#define CMD_TOGGLE_SD "toggle_sd"
#define CMD_ROTATE_FILE "rotate_file"
#define CMD_SET_RATE "set_rate"

// =================================================================================================
// DEBUGGING
// =================================================================================================

#ifndef SENSYTHING_DEBUG
#define SENSYTHING_DEBUG 0  // Set to 1 to enable debug output
#endif

#if SENSYTHING_DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(...)
#endif

#endif // SENSYTHING_CONFIG_H
