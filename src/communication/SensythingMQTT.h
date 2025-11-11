//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    SensythingES3 - MQTT Communication Module
//    Remote data streaming to MQTT broker
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#ifndef SENSYTHING_MQTT_H
#define SENSYTHING_MQTT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "../core/SensythingTypes.h"
#include "../core/SensythingConfig.h"

class SensythingMQTT {
public:
    SensythingMQTT();
    ~SensythingMQTT();
    
    /**
     * Initialize MQTT connection with broker details
     * @param brokerAddress MQTT broker hostname or IP
     * @param brokerPort MQTT broker port (default 1883)
     * @param clientID Unique MQTT client ID
     * @param config Board configuration for channel information
     * @return true if initialization successful
     */
    bool init(const char* brokerAddress, 
              uint16_t brokerPort, 
              const char* clientID,
              const BoardConfig& config);
    
    /**
     * Set MQTT username and password for authentication
     * @param username MQTT broker username
     * @param password MQTT broker password
     * @return true if credentials set successfully
     */
    bool setCredentials(const char* username, const char* password);
    
    /**
     * Establish connection to MQTT broker
     * @return true if connection successful
     */
    bool connect();
    
    /**
     * Disconnect from MQTT broker gracefully
     */
    void disconnect();
    
    /**
     * Attempt reconnection (called automatically in update)
     * @return true if reconnected
     */
    bool reconnect();
    
    /**
     * Check if connected to MQTT broker
     * @return true if connected and authenticated
     */
    bool isConnected() const { return connected; }
    
    /**
     * Stream measurement data to MQTT topics
     * Publishes individual channels + JSON consolidated data
     * @param data Measurement data structure
     * @param config Board configuration for format adaptation
     */
    void streamData(const MeasurementData& data, const BoardConfig& config);
    
    /**
     * Handle periodic MQTT tasks (reconnection, keepalive)
     * Call frequently in main loop (non-blocking)
     */
    void update();
    
    /**
     * Set base topic for publishing
     * Individual channels: {baseTopic}/{channelName}
     * Consolidated: {baseTopic}/data
     * @param baseTopic Base MQTT topic (e.g., "sensything/device1")
     */
    void setBaseTopic(const char* baseTopic);
    
    /**
     * Configure message retention
     * Retained messages persist on broker
     * @param retain true to enable message retention
     */
    void setRetain(bool retain) { useRetain = retain; }
    
    /**
     * Set Quality of Service level
     * @param qos 0 (fire and forget), 1 (at least once), 2 (exactly once)
     */
    void setQoS(uint8_t qos);
    
    /**
     * Publish a custom message to topic
     * Used primarily for testing connectivity
     * @param topic MQTT topic
     * @param payload Message payload
     * @return true if publish successful
     */
    bool publish(const char* topic, const char* payload);
    
    // ===== DIAGNOSTICS =====
    
    /**
     * Get broker address
     * @return Broker hostname/IP
     */
    String getBrokerAddress() const { return brokerAddress; }
    
    /**
     * Get broker port
     * @return Broker port number
     */
    uint16_t getBrokerPort() const { return brokerPort; }
    
    /**
     * Get last error code from PubSubClient
     * @return Error code (0 = no error)
     */
    int getLastError() const;
    
    /**
     * Get connection status string
     * @return Status message (e.g., "Connected", "Disconnected")
     */
    String getConnectionStatus() const;
    
    /**
     * Get client ID
     * @return MQTT client ID
     */
    String getClientID() const { return clientID; }

private:
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    
    bool initialized;
    bool connected;
    
    String brokerAddress;
    uint16_t brokerPort;
    String clientID;
    String username;
    String password;
    String baseTopic;
    
    bool useRetain;
    uint8_t qosLevel;
    
    BoardConfig boardConfig;
    
    unsigned long lastReconnectAttempt;
    static const unsigned long RECONNECT_INTERVAL = 5000;  // 5 seconds
    
    // Callback for MQTT messages (for future subscription support)
    static SensythingMQTT* instance;
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
    void handleMessage(const char* topic, const byte* payload, unsigned int length);
    
    /**
     * Format measurement data as JSON payload
     * @param data Measurement data
     * @param config Board configuration
     * @return JSON string
     */
    String formatAsJSON(const MeasurementData& data, const BoardConfig& config);
};

#endif // SENSYTHING_MQTT_H
