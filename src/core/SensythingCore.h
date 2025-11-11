//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    SensythingES3 - Core Base Class
//    Abstract base class for all SensythingES3 board implementations
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

#ifndef SENSYTHING_CORE_H
#define SENSYTHING_CORE_H

#include <Arduino.h>
#include "SensythingTypes.h"
#include "SensythingConfig.h"

// Forward declarations for communication modules
class SensythingUSB;
class SensythingBLE;
class SensythingWiFi;
class SensythingSDCard;
class SensythingMQTT;

class SensythingCore {
public:
    SensythingCore();
    virtual ~SensythingCore();
    
    // =================================================================================================
    // PURE VIRTUAL METHODS - Must be implemented by board-specific classes
    // =================================================================================================
    
    /**
     * Initialize the sensor hardware
     * @return true if initialization successful, false otherwise
     */
    virtual bool initSensor() = 0;
    
    /**
     * Read a measurement from the sensor
     * @param data Reference to MeasurementData structure to fill
     * @return true if measurement successful, false otherwise
     */
    virtual bool readMeasurement(MeasurementData& data) = 0;
    
    /**
     * Get the board name (e.g., "Sensything Cap", "Sensything OX")
     * @return Board name string
     */
    virtual String getBoardName() = 0;
    
    /**
     * Get the sensor type description (e.g., "FDC1004 Capacitance", "AFE4400 PPG/SpO2")
     * @return Sensor type string
     */
    virtual String getSensorType() = 0;
    
    /**
     * Get board-specific configuration
     * @return BoardConfig structure with board details
     */
    virtual BoardConfig getBoardConfig() = 0;
    
    // =================================================================================================
    // PLATFORM INITIALIZATION
    // =================================================================================================
    
    /**
     * Initialize the entire platform (sensor + communication interfaces)
     * @return true if successful, false otherwise
     */
    bool initPlatform();
    
    /**
     * Initialize only the sensor (without communication interfaces)
     * @return true if successful, false otherwise
     */
    bool initSensorOnly();
    
    // =================================================================================================
    // COMMUNICATION INTERFACE INITIALIZATION
    // =================================================================================================
    
    /**
     * Initialize BLE module
     * @return true if successful, false otherwise
     */
    bool initBLE();
    
    /**
     * Initialize WiFi module (Access Point mode)
     * @param ssid AP name (nullptr = auto-generate from board name)
     * @param password AP password (nullptr = open network)
     * @return true if successful, false otherwise
     */
    bool initWiFi(const char* ssid = nullptr, const char* password = nullptr);
    
    /**
     * Initialize WiFi module in Station mode (connect to existing network)
     * @param ssid Network SSID
     * @param password Network password
     * @return true if successful, false otherwise
     */
    bool initWiFiStation(const char* ssid, const char* password);
    
    /**
     * Initialize WiFi in AP+Station mode (configuration portal)
     * @param apSSID Access Point name
     * @param apPassword AP password (min 8 chars, empty = open)
     * @param staSSID Station SSID (empty = AP only)
     * @param staPassword Station password
     * @return true if initialization successful
     */
    bool initAPStation(const char* apSSID, const char* apPassword, const char* staSSID = "", const char* staPassword = "");
    
    /**
     * Initialize SD Card module
     * @return true if successful, false otherwise
     */
    bool initSDCard();
    
    /**
     * Initialize MQTT module
     * @param brokerAddress MQTT broker IP/hostname
     * @param brokerPort MQTT broker port (default 1883)
     * @param clientID MQTT client ID (nullptr = auto-generate from board name)
     * @return true if successful, false otherwise
     */
    bool initMQTT(const char* brokerAddress, uint16_t brokerPort = 1883, const char* clientID = nullptr);
    
    // =================================================================================================
    // COMMUNICATION INTERFACE CONTROL
    // =================================================================================================
    
    /**
     * Enable/disable USB Serial streaming
     * @param enable true to enable, false to disable
     */
    void enableUSB(bool enable);
    
    /**
     * Enable/disable BLE streaming
     * @param enable true to enable, false to disable
     */
    void enableBLE(bool enable);
    
    /**
     * Enable/disable WiFi streaming
     * @param enable true to enable, false to disable
     * @param ssid WiFi SSID (nullptr for AP mode)
     * @param password WiFi password
     */
    void enableWiFi(bool enable, const char* ssid = nullptr, const char* password = nullptr);
    
    /**
     * Enable/disable SD card logging
     * @param enable true to enable, false to disable
     */
    void enableSDCard(bool enable);
    
    /**
     * Enable/disable MQTT streaming
     * @param enable true to enable, false to disable
     */
    void enableMQTT(bool enable);
    
    /**
     * Set MQTT credentials (username/password)
     * @param username MQTT username
     * @param password MQTT password
     * @return true if successful
     */
    bool setMQTTCredentials(const char* username, const char* password);
    
    /**
     * Set MQTT base topic
     * @param baseTopic Base topic (e.g., "sensything")
     */
    void setMQTTBaseTopic(const char* baseTopic);
    
    /**
     * Set MQTT QoS level (0, 1, or 2)
     * @param qos Quality of Service level
     */
    void setMQTTQoS(uint8_t qos);
    
    /**
     * Enable all communication interfaces
     */
    void enableAll();
    
    /**
     * Disable all communication interfaces
     */
    void disableAll();
    
    // =================================================================================================
    // MEASUREMENT CONTROL
    // =================================================================================================
    
    /**
     * Set the sample rate
     * @param intervalMs Sample interval in milliseconds
     * @return true if successful, false if out of range
     */
    bool setSampleRate(unsigned long intervalMs);
    
    /**
     * Get current sample rate in Hz
     * @return Sample rate in Hz
     */
    float getSampleRateHz();
    
    /**
     * Start measurements
     */
    void startMeasurements();
    
    /**
     * Stop measurements
     */
    void stopMeasurements();
    
    /**
     * Reset measurement counter
     */
    void resetMeasurementCount();
    
    // =================================================================================================
    // MAIN LOOP HANDLER
    // =================================================================================================
    
    /**
     * Main update function - call this in Arduino loop()
     * Handles timing, measurements, and streaming
     */
    void update();
    
    // =================================================================================================
    // COMMAND PROCESSING
    // =================================================================================================
    
    /**
     * Process a text command
     * @param command Command string to process
     */
    void processCommand(String command);
    
    /**
     * Process commands from Serial input
     */
    void processSerialCommands();
    
    /**
     * Print available commands
     */
    void printHelp();
    
    // =================================================================================================
    // STATUS AND DIAGNOSTICS
    // =================================================================================================
    
    /**
     * Get current system status
     * @return SystemStatus structure
     */
    SystemStatus getStatus();
    
    /**
     * Print system status to Serial
     */
    void printStatus();
    
    /**
     * Get current measurement data
     * @return Reference to last measurement
     */
    const MeasurementData& getCurrentMeasurement();
    
    // =================================================================================================
    // GETTER METHODS
    // =================================================================================================
    
    bool isUSBEnabled() { return sysState.usbStreamingEnabled; }
    bool isBLEEnabled() { return sysState.bleStreamingEnabled; }
    bool isWiFiEnabled() { return sysState.wifiStreamingEnabled; }
    bool isSDEnabled() { return sysState.sdLoggingEnabled; }
    bool isMQTTEnabled() { return sysState.mqttStreamingEnabled; }
    bool isBLEConnected() { return sysState.bleConnected; }
    bool isWiFiConnected() { return sysState.wifiConnected; }
    bool isMQTTConnected() { return sysState.mqttConnected; }
    bool isSDReady() { return sysState.sdCardReady; }
    uint32_t getMeasurementCount() { return sysState.measurementCount; }
    
protected:
    // =================================================================================================
    // PROTECTED MEMBER VARIABLES
    // =================================================================================================
    
    SystemState sysState;
    MeasurementData currentMeasurement;
    BoardConfig boardConfig;
    
    // Communication module pointers (managed by platform)
    SensythingUSB* usbModule;
    SensythingBLE* bleModule;
    SensythingWiFi* wifiModule;
    SensythingSDCard* sdModule;
    SensythingMQTT* mqttModule;
    
    // =================================================================================================
    // PROTECTED HELPER METHODS
    // =================================================================================================
    
    /**
     * Initialize system state with defaults
     */
    void initSystemState();
    
    /**
     * Check if it's time for a new measurement
     * @return true if measurement should be taken
     */
    bool shouldTakeMeasurement();
    
    /**
     * Stream current measurement to all active interfaces
     */
    void streamMeasurement();
    
    /**
     * Periodic status update
     */
    void periodicStatusUpdate();
};

#endif // SENSYTHING_CORE_H
