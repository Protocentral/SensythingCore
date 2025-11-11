//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything Platform - Core Base Class Implementation
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "SensythingCore.h"
#include "../communication/SensythingUSB.h"
#include "../communication/SensythingBLE.h"
#include "../communication/SensythingWiFi.h"
#include "../communication/SensythingSDCard.h"
#include "../communication/SensythingMQTT.h"

// =================================================================================================
// CONSTRUCTOR / DESTRUCTOR
// =================================================================================================

SensythingCore::SensythingCore() {
    // Initialize pointers
    usbModule = nullptr;
    bleModule = nullptr;
    wifiModule = nullptr;
    sdModule = nullptr;
    mqttModule = nullptr;
    
    // Initialize system state
    initSystemState();
}

SensythingCore::~SensythingCore() {
    // Clean up communication modules
    if (usbModule) delete usbModule;
    if (bleModule) delete bleModule;
    if (wifiModule) delete wifiModule;
    if (sdModule) delete sdModule;
    if (mqttModule) delete mqttModule;
}

// =================================================================================================
// INITIALIZATION
// =================================================================================================

void SensythingCore::initSystemState() {
    sysState.sampleInterval = SENSYTHING_DEFAULT_SAMPLE_INTERVAL_MS;
    sysState.lastMeasurement = 0;
    sysState.lastStatusUpdate = 0;
    sysState.lastFileRotation = 0;
    
    sysState.measurementActive = false;
    sysState.measurementCount = 0;
    
    sysState.usbStreamingEnabled = false;
    sysState.bleStreamingEnabled = false;
    sysState.wifiStreamingEnabled = false;
    sysState.sdLoggingEnabled = false;
    
    sysState.bleConnected = false;
    sysState.wifiConnected = false;
    sysState.sdCardReady = false;
    
    sysState.currentFileName = "";
    sysState.fileCount = 0;
    
    // Initialize measurement data
    memset(&currentMeasurement, 0, sizeof(MeasurementData));
}

bool SensythingCore::initPlatform() {
    Serial.begin(SENSYTHING_SERIAL_BAUD_RATE);
    delay(100);
    
    Serial.println("=================================");
    Serial.printf("%s Initialization\n", SENSYTHING_ES3_NAME);
    Serial.printf("Version: %s\n", SENSYTHING_ES3_VERSION);
    Serial.println("=================================");
    
    // Get board configuration
    boardConfig = getBoardConfig();
    
    Serial.printf("Board: %s\n", getBoardName().c_str());
    Serial.printf("Sensor: %s\n", getSensorType().c_str());
    Serial.println();
    
    // Initialize sensor
    Serial.print("Initializing sensor... ");
    if (initSensor()) {
        Serial.println(String(EMOJI_SUCCESS) + " Success");
    } else {
        Serial.println(String(EMOJI_ERROR) + " Failed");
        return false;
    }
    
    // Create USB module
    usbModule = new SensythingUSB();
    
    // Enable USB streaming by default
    sysState.usbStreamingEnabled = true;
    
    Serial.println(String(EMOJI_SUCCESS) + " Platform initialized");
    Serial.println("Type 'help' for available commands");
    Serial.println("=================================");
    
    // Start measurements by default
    startMeasurements();
    
    return true;
}

bool SensythingCore::initSensorOnly() {
    Serial.begin(SENSYTHING_SERIAL_BAUD_RATE);
    delay(100);
    
    Serial.printf("Initializing %s sensor... ", getSensorType().c_str());
    
    if (initSensor()) {
        Serial.println(String(EMOJI_SUCCESS) + " Success");
        return true;
    } else {
        Serial.println(String(EMOJI_ERROR) + " Failed");
        return false;
    }
}

// =================================================================================================
// COMMUNICATION INTERFACE INITIALIZATION
// =================================================================================================

bool SensythingCore::initBLE() {
    if (bleModule) {
        Serial.println(String(EMOJI_WARNING) + " BLE already initialized");
        return true;
    }
    
    bleModule = new SensythingBLE();
    if (!bleModule->init(boardConfig)) {
        Serial.println(String(EMOJI_ERROR) + " BLE initialization failed");
        delete bleModule;
        bleModule = nullptr;
        return false;
    }
    
    Serial.println(String(EMOJI_BLUETOOTH) + " BLE module ready");
    return true;
}

bool SensythingCore::initWiFi(const char* ssid, const char* password) {
    if (wifiModule) {
        Serial.println(String(EMOJI_WARNING) + " WiFi already initialized");
        return true;
    }
    
    // Generate SSID if not provided
    String apSSID = ssid ? String(ssid) : (getBoardName() + "-" + String(ESP.getEfuseMac(), HEX).substring(6));
    String apPassword = password ? String(password) : "";
    
    wifiModule = new SensythingWiFi();
    if (!wifiModule->initAP(apSSID, apPassword, boardConfig)) {
        Serial.println(String(EMOJI_ERROR) + " WiFi AP initialization failed");
        delete wifiModule;
        wifiModule = nullptr;
        return false;
    }
    
    Serial.println(String(EMOJI_WIFI) + " WiFi module ready (AP mode)");
    return true;
}

bool SensythingCore::initWiFiStation(const char* ssid, const char* password) {
    if (wifiModule) {
        Serial.println(String(EMOJI_WARNING) + " WiFi already initialized");
        return true;
    }
    
    if (!ssid || !password) {
        Serial.println(String(EMOJI_ERROR) + " SSID and password required for Station mode");
        return false;
    }
    
    wifiModule = new SensythingWiFi();
    if (!wifiModule->initStation(String(ssid), String(password), boardConfig)) {
        Serial.println(String(EMOJI_ERROR) + " WiFi Station initialization failed");
        delete wifiModule;
        wifiModule = nullptr;
        return false;
    }
    
    Serial.println(String(EMOJI_WIFI) + " WiFi module ready (Station mode)");
    return true;
}

bool SensythingCore::initAPStation(const char* apSSID, const char* apPassword, const char* staSSID, const char* staPassword) {
    if (wifiModule) {
        Serial.println(String(EMOJI_WARNING) + " WiFi already initialized");
        return true;
    }
    
    wifiModule = new SensythingWiFi();
    if (!wifiModule->initAPStation(String(apSSID), String(apPassword), String(staSSID), String(staPassword), boardConfig)) {
        Serial.println(String(EMOJI_ERROR) + " WiFi AP+Station initialization failed");
        delete wifiModule;
        wifiModule = nullptr;
        return false;
    }
    
    Serial.println(String(EMOJI_WIFI) + " WiFi module ready (AP+Station mode)");
    return true;
}

bool SensythingCore::initSDCard() {
    if (sdModule) {
        Serial.println(String(EMOJI_WARNING) + " SD Card already initialized");
        return true;
    }
    
    sdModule = new SensythingSDCard();
    if (!sdModule->init()) {
        Serial.println(String(EMOJI_ERROR) + " SD Card initialization failed");
        delete sdModule;
        sdModule = nullptr;
        return false;
    }
    
    sysState.sdCardReady = true;
    Serial.println(String(EMOJI_STORAGE) + " SD Card module ready");
    return true;
}

bool SensythingCore::initMQTT(const char* brokerAddress, uint16_t brokerPort, const char* clientID) {
    if (mqttModule) {
        Serial.println(String(EMOJI_WARNING) + " MQTT already initialized");
        return true;
    }
    
    // Check WiFi connectivity (MQTT requires WiFi)
    if (!wifiModule || WiFi.status() != WL_CONNECTED) {
        Serial.println(String(EMOJI_ERROR) + " WiFi must be connected before MQTT. Call initWiFi() first.");
        return false;
    }
    
    mqttModule = new SensythingMQTT();
    
    // Generate client ID if not provided
    String finalClientID = clientID ? clientID : (getBoardName() + "_" + String(ESP.getEfuseMac(), HEX));
    
    if (!mqttModule->init(brokerAddress, brokerPort, finalClientID.c_str(), boardConfig)) {
        Serial.println(String(EMOJI_ERROR) + " MQTT initialization failed");
        delete mqttModule;
        mqttModule = nullptr;
        return false;
    }
    
    sysState.mqttConnected = false;  // Will be set by streaming
    Serial.println(String(EMOJI_NETWORK) + " MQTT module ready");
    return true;
}

// =================================================================================================
// COMMUNICATION INTERFACE CONTROL
// =================================================================================================

void SensythingCore::enableUSB(bool enable) {
    if (enable && !usbModule) {
        usbModule = new SensythingUSB();
    }
    sysState.usbStreamingEnabled = enable;
    
    if (enable) {
        Serial.println(String(EMOJI_SUCCESS) + " USB streaming enabled");
    } else {
        Serial.println(String(EMOJI_SUCCESS) + " USB streaming disabled");
    }
}

void SensythingCore::enableBLE(bool enable) {
    if (enable && !bleModule) {
        Serial.println(String(EMOJI_WARNING) + " BLE not initialized. Call initBLE() first.");
        return;
    }
    
    sysState.bleStreamingEnabled = enable;
    
    if (enable) {
        Serial.println(String(EMOJI_BLUETOOTH) + " BLE streaming enabled");
    } else {
        Serial.println(String(EMOJI_BLUETOOTH) + " BLE streaming disabled");
    }
}

void SensythingCore::enableWiFi(bool enable, const char* ssid, const char* password) {
    sysState.wifiStreamingEnabled = enable;
    
    if (enable) {
        if (wifiModule) {
            Serial.println(String(EMOJI_WIFI) + " WiFi streaming enabled");
        } else {
            Serial.println(String(EMOJI_WARNING) + " WiFi not initialized. Call initWiFi() first.");
        }
    } else {
        Serial.println(String(EMOJI_WIFI) + " WiFi streaming disabled");
    }
}

void SensythingCore::enableSDCard(bool enable) {
    if (enable && !sdModule) {
        Serial.println(String(EMOJI_WARNING) + " SD Card not initialized. Call initSDCard() first.");
        return;
    }
    
    sysState.sdLoggingEnabled = enable;
    
    if (enable) {
        Serial.println(String(EMOJI_STORAGE) + " SD Card logging enabled");
    } else {
        Serial.println(String(EMOJI_STORAGE) + " SD Card logging disabled");
    }
}

void SensythingCore::enableMQTT(bool enable) {
    if (enable && !mqttModule) {
        Serial.println(String(EMOJI_WARNING) + " MQTT not initialized. Call initMQTT() first.");
        return;
    }
    
    sysState.mqttStreamingEnabled = enable;
    
    if (enable) {
        Serial.println(String(EMOJI_NETWORK) + " MQTT streaming enabled");
    } else {
        Serial.println(String(EMOJI_NETWORK) + " MQTT streaming disabled");
    }
}

bool SensythingCore::setMQTTCredentials(const char* username, const char* password) {
    if (!mqttModule) {
        Serial.println(String(EMOJI_WARNING) + " MQTT not initialized. Call initMQTT() first.");
        return false;
    }
    
    return mqttModule->setCredentials(username, password);
}

void SensythingCore::setMQTTBaseTopic(const char* baseTopic) {
    if (!mqttModule) {
        Serial.println(String(EMOJI_WARNING) + " MQTT not initialized. Call initMQTT() first.");
        return;
    }
    
    mqttModule->setBaseTopic(baseTopic);
}

void SensythingCore::setMQTTQoS(uint8_t qos) {
    if (!mqttModule) {
        Serial.println(String(EMOJI_WARNING) + " MQTT not initialized. Call initMQTT() first.");
        return;
    }
    
    mqttModule->setQoS(qos);
}

void SensythingCore::enableAll() {
    enableUSB(true);
    enableBLE(true);
    enableWiFi(true);
    enableSDCard(true);
    enableMQTT(true);
}

void SensythingCore::disableAll() {
    enableUSB(false);
    enableBLE(false);
    enableWiFi(false);
    enableSDCard(false);
    enableMQTT(false);
}

// =================================================================================================
// MEASUREMENT CONTROL
// =================================================================================================

bool SensythingCore::setSampleRate(unsigned long intervalMs) {
    if (intervalMs < SENSYTHING_MIN_SAMPLE_INTERVAL_MS || 
        intervalMs > SENSYTHING_MAX_SAMPLE_INTERVAL_MS) {
        Serial.printf("%s Invalid sample rate (valid range: %lu-%lu ms)\n", 
                     EMOJI_ERROR,
                     SENSYTHING_MIN_SAMPLE_INTERVAL_MS,
                     SENSYTHING_MAX_SAMPLE_INTERVAL_MS);
        return false;
    }
    
    sysState.sampleInterval = intervalMs;
    Serial.printf("%s Sample rate set to %.2f Hz (%lu ms interval)\n", 
                 EMOJI_SUCCESS, getSampleRateHz(), intervalMs);
    return true;
}

float SensythingCore::getSampleRateHz() {
    return 1000.0f / sysState.sampleInterval;
}

void SensythingCore::startMeasurements() {
    sysState.measurementActive = true;
    Serial.println(String(EMOJI_SUCCESS) + " Measurements started");
}

void SensythingCore::stopMeasurements() {
    sysState.measurementActive = false;
    Serial.println(String(EMOJI_SUCCESS) + " Measurements stopped");
}

void SensythingCore::resetMeasurementCount() {
    sysState.measurementCount = 0;
    Serial.println(String(EMOJI_SUCCESS) + " Measurement count reset");
}

// =================================================================================================
// MAIN LOOP HANDLER
// =================================================================================================

bool SensythingCore::shouldTakeMeasurement() {
    if (!sysState.measurementActive) return false;
    
    unsigned long now = millis();
    if (now - sysState.lastMeasurement >= sysState.sampleInterval) {
        sysState.lastMeasurement = now;
        return true;
    }
    return false;
}

void SensythingCore::streamMeasurement() {
    // Stream to USB if enabled
    if (sysState.usbStreamingEnabled && usbModule) {
        usbModule->streamData(currentMeasurement, boardConfig);
    }
    
    // Stream to BLE if enabled
    if (sysState.bleStreamingEnabled && bleModule) {
        bleModule->streamData(currentMeasurement, boardConfig);
        // Update connection state
        sysState.bleConnected = bleModule->isConnected();
    }
    
    // Stream to WiFi if enabled
    if (sysState.wifiStreamingEnabled && wifiModule) {
        wifiModule->streamData(currentMeasurement, boardConfig);
        // Update connection state
        sysState.wifiConnected = wifiModule->hasClients();
    }
    
    // Log to SD Card if enabled
    if (sysState.sdLoggingEnabled && sdModule) {
        sdModule->logData(currentMeasurement, boardConfig);
        // Update SD card state
        sysState.sdCardReady = sdModule->isReady();
        
        // Check for file rotation
        unsigned long now = millis();
        if (now - sysState.lastFileRotation >= SENSYTHING_FILE_ROTATION_INTERVAL_MS) {
            sdModule->rotateFile();
            sysState.lastFileRotation = now;
        }
    }
    
    // Stream to MQTT if enabled
    if (sysState.mqttStreamingEnabled && mqttModule) {
        mqttModule->streamData(currentMeasurement, boardConfig);
        mqttModule->update();  // Handle reconnection and message processing
        // Update connection state
        sysState.mqttConnected = mqttModule->isConnected();
    }
}

void SensythingCore::periodicStatusUpdate() {
    unsigned long now = millis();
    if (now - sysState.lastStatusUpdate >= SENSYTHING_STATUS_UPDATE_INTERVAL_MS) {
        sysState.lastStatusUpdate = now;
        
        DEBUG_PRINTLN("=== Periodic Status Update ===");
        DEBUG_PRINTF("Uptime: %lu s, Measurements: %u, Rate: %.2f Hz\n",
                    now / 1000, sysState.measurementCount, getSampleRateHz());
    }
}

void SensythingCore::update() {
    // Check for serial commands
    processSerialCommands();
    
    // Handle WiFi/WebSocket events
    if (wifiModule) {
        wifiModule->update();
    }
    
    // Take measurement if it's time
    if (shouldTakeMeasurement()) {
        if (readMeasurement(currentMeasurement)) {
            currentMeasurement.measurement_count = ++sysState.measurementCount;
            streamMeasurement();
        }
    }
    
    // Periodic status update
    periodicStatusUpdate();
}

// =================================================================================================
// COMMAND PROCESSING
// =================================================================================================

void SensythingCore::processSerialCommands() {
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        processCommand(cmd);
    }
}

void SensythingCore::processCommand(String command) {
    command.trim();
    command.toLowerCase();
    
    if (command.length() == 0) return;
    
    if (command == CMD_START_ALL) {
        enableAll();
        startMeasurements();
        
    } else if (command == CMD_STOP_ALL) {
        disableAll();
        stopMeasurements();
        
    } else if (command == CMD_STATUS) {
        printStatus();
        
    } else if (command == CMD_HELP) {
        printHelp();
        
    } else if (command == CMD_RESET_COUNT) {
        resetMeasurementCount();
        
    } else if (command == CMD_TOGGLE_SD) {
        enableSDCard(!sysState.sdLoggingEnabled);
        
    } else if (command == CMD_ROTATE_FILE) {
        if (sdModule && sysState.sdLoggingEnabled) {
            sdModule->rotateFile();
        } else {
            Serial.println(String(EMOJI_ERROR) + " SD Card not active");
        }
        
    } else if (command.startsWith(CMD_SET_RATE)) {
        int spaceIndex = command.indexOf(' ');
        if (spaceIndex > 0) {
            String valueStr = command.substring(spaceIndex + 1);
            unsigned long interval = valueStr.toInt();
            setSampleRate(interval);
        } else {
            Serial.println(String(EMOJI_ERROR) + " Usage: set_rate <milliseconds>");
        }
        
    } else if (command == "forget_wifi" || command == "clear_wifi") {
        if (wifiModule) {
            Serial.println(String(EMOJI_INFO) + " Clearing saved WiFi credentials...");
            bool success = wifiModule->clearCredentials();
            if (success) {
                Serial.println(String(EMOJI_SUCCESS) + " WiFi credentials cleared!");
                Serial.println(String(EMOJI_INFO) + " Board will restart in AP-only mode on next boot");
            } else {
                Serial.println(String(EMOJI_ERROR) + " Failed to clear credentials");
            }
        } else {
            Serial.println(String(EMOJI_ERROR) + " WiFi module not initialized");
        }
        
    } else if (command.startsWith("init_mqtt")) {
        // Format: init_mqtt <broker> <port>
        int firstSpace = command.indexOf(' ');
        int secondSpace = command.indexOf(' ', firstSpace + 1);
        
        if (firstSpace > 0 && secondSpace > firstSpace) {
            String broker = command.substring(firstSpace + 1, secondSpace);
            String portStr = command.substring(secondSpace + 1);
            uint16_t port = portStr.toInt();
            
            if (port == 0) port = 1883;  // Default MQTT port
            
            Serial.println(String(EMOJI_INFO) + " Initializing MQTT: " + broker + ":" + String(port));
            bool success = initMQTT(broker.c_str(), port);
            
            if (success) {
                enableMQTT(true);
                Serial.println(String(EMOJI_SUCCESS) + " MQTT initialized and enabled!");
            } else {
                Serial.println(String(EMOJI_ERROR) + " MQTT initialization failed");
            }
        } else {
            Serial.println(String(EMOJI_ERROR) + " Usage: init_mqtt <broker> <port>");
        }
        
    } else if (command.startsWith("mqtt_auth")) {
        // Format: mqtt_auth <username> <password>
        int firstSpace = command.indexOf(' ');
        int secondSpace = command.indexOf(' ', firstSpace + 1);
        
        if (firstSpace > 0 && secondSpace > firstSpace) {
            String user = command.substring(firstSpace + 1, secondSpace);
            String pass = command.substring(secondSpace + 1);
            
            bool success = setMQTTCredentials(user.c_str(), pass.c_str());
            if (success) {
                Serial.println(String(EMOJI_SUCCESS) + " MQTT credentials set!");
            }
        } else {
            Serial.println(String(EMOJI_ERROR) + " Usage: mqtt_auth <username> <password>");
        }
        
    } else if (command.startsWith("mqtt_topic")) {
        // Format: mqtt_topic <topic>
        int spaceIndex = command.indexOf(' ');
        
        if (spaceIndex > 0) {
            String topic = command.substring(spaceIndex + 1);
            setMQTTBaseTopic(topic.c_str());
            Serial.println(String(EMOJI_SUCCESS) + " MQTT topic set to: " + topic);
        } else {
            Serial.println(String(EMOJI_ERROR) + " Usage: mqtt_topic <topic>");
        }
        
    } else {
        Serial.printf("%s Unknown command: '%s' (type 'help' for commands)\n", 
                     EMOJI_ERROR, command.c_str());
    }
}

void SensythingCore::printHelp() {
    Serial.println("=================================");
    Serial.println("AVAILABLE COMMANDS");
    Serial.println("=================================");
    Serial.println("start_all       - Start all interfaces");
    Serial.println("stop_all        - Stop all interfaces");
    Serial.println("status          - Show system status");
    Serial.println("reset_count     - Reset measurement count");
    Serial.println("toggle_sd       - Toggle SD Card logging");
    Serial.println("rotate_file     - Force new SD file");
    Serial.println("set_rate <ms>   - Set sample rate (20-10000)");
    Serial.println("forget_wifi     - Clear saved WiFi credentials");
    Serial.println("init_mqtt <br> <port> - Initialize MQTT");
    Serial.println("mqtt_auth <user> <pass> - Set MQTT credentials");
    Serial.println("mqtt_topic <topic> - Set MQTT base topic");
    Serial.println("help            - Show this help");
    Serial.println("=================================");
}

// =================================================================================================
// STATUS AND DIAGNOSTICS
// =================================================================================================

SystemStatus SensythingCore::getStatus() {
    SystemStatus status;
    
    status.boardType = boardConfig.boardType;
    status.boardName = getBoardName();
    status.sensorType = getSensorType();
    status.firmwareVersion = SENSYTHING_ES3_VERSION;
    
    status.uptimeSeconds = millis() / 1000;
    status.sampleRateHz = getSampleRateHz();
    status.totalMeasurements = sysState.measurementCount;
    
    status.activeInterfaces = 0;
    if (sysState.usbStreamingEnabled) status.activeInterfaces |= INTERFACE_USB;
    if (sysState.bleStreamingEnabled) status.activeInterfaces |= INTERFACE_BLE;
    if (sysState.wifiStreamingEnabled) status.activeInterfaces |= INTERFACE_WIFI;
    if (sysState.sdLoggingEnabled) status.activeInterfaces |= INTERFACE_SD_CARD;
    
    status.bleConnected = sysState.bleConnected;
    status.wifiConnected = sysState.wifiConnected;
    status.sdCardReady = sysState.sdCardReady;
    
    status.currentSDFile = sysState.currentFileName;
    status.sdFileSize = 0;
    
    return status;
}

void SensythingCore::printStatus() {
    Serial.println("=================================");
    Serial.printf("%s SYSTEM STATUS\n", EMOJI_INFO);
    Serial.println("=================================");
    Serial.printf("Board: %s\n", getBoardName().c_str());
    Serial.printf("Sensor: %s\n", getSensorType().c_str());
    Serial.printf("Firmware: %s\n", SENSYTHING_ES3_VERSION);
    Serial.println();
    Serial.printf("%s Uptime: %lu seconds\n", EMOJI_TIME, millis() / 1000);
    Serial.printf("%s Sample Rate: %.2f Hz (%lu ms)\n", EMOJI_CONFIG,
                 getSampleRateHz(), sysState.sampleInterval);
    Serial.printf("%s Measurements: %u\n", EMOJI_DATA, sysState.measurementCount);
    Serial.println();
    Serial.println("Active Interfaces:");
    Serial.printf("  %s USB: %s\n", EMOJI_NETWORK, 
                 sysState.usbStreamingEnabled ? "ON" : "OFF");
    Serial.printf("  %s BLE: %s%s\n", EMOJI_BLUETOOTH,
                 sysState.bleStreamingEnabled ? "ON" : "OFF",
                 sysState.bleConnected ? " (Connected)" : "");
    Serial.printf("  %s WiFi: %s%s\n", EMOJI_WIFI,
                 sysState.wifiStreamingEnabled ? "ON" : "OFF",
                 sysState.wifiConnected ? " (Connected)" : "");
    Serial.printf("  %s SD Card: %s%s\n", EMOJI_STORAGE,
                 sysState.sdLoggingEnabled ? "ON" : "OFF",
                 sysState.sdCardReady ? " (Ready)" : "");
    Serial.println("=================================");
}

const MeasurementData& SensythingCore::getCurrentMeasurement() {
    return currentMeasurement;
}
