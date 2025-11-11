//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    SensythingES3 - MQTT Communication Module Implementation
//    Remote data streaming to MQTT broker
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "SensythingMQTT.h"

// Static instance for callback
SensythingMQTT* SensythingMQTT::instance = nullptr;

SensythingMQTT::SensythingMQTT() {
    initialized = false;
    connected = false;
    useRetain = true;
    qosLevel = 1;  // QoS 1: At least once
    brokerPort = 1883;
    lastReconnectAttempt = 0;
    
    // Initialize PubSubClient with WiFi client
    mqttClient.setClient(wifiClient);
    mqttClient.setCallback(mqttCallback);
    
    instance = this;  // Set static instance for callbacks
}

SensythingMQTT::~SensythingMQTT() {
    if (connected) {
        disconnect();
    }
}

bool SensythingMQTT::init(const char* brokerAddress, 
                          uint16_t brokerPort,
                          const char* clientID,
                          const BoardConfig& config) {
    if (!WiFi.isConnected()) {
        Serial.println(String(EMOJI_ERROR) + " WiFi must be connected before MQTT");
        return false;
    }
    
    this->brokerAddress = brokerAddress;
    this->brokerPort = brokerPort;
    this->clientID = clientID;
    this->boardConfig = config;
    
    // Default base topic
    if (baseTopic.length() == 0) {
        baseTopic = "sensything";
    }
    
    // Configure PubSubClient
    mqttClient.setServer(brokerAddress, brokerPort);
    
    Serial.print(String(EMOJI_INFO) + " MQTT: Connecting to ");
    Serial.print(brokerAddress);
    Serial.print(":");
    Serial.println(brokerPort);
    
    if (connect()) {
        initialized = true;
        Serial.println(String(EMOJI_SUCCESS) + " MQTT initialized and connected");
        return true;
    }
    
    Serial.println(String(EMOJI_WARNING) + " MQTT initialization failed, will retry");
    initialized = true;  // Allow retries in update()
    return false;
}

bool SensythingMQTT::setCredentials(const char* username, const char* password) {
    if (!username || !password) {
        return false;
    }
    
    this->username = username;
    this->password = password;
    
    Serial.println(String(EMOJI_INFO) + " MQTT credentials set");
    return true;
}

bool SensythingMQTT::connect() {
    if (!initialized) {
        return false;
    }
    
    if (connected) {
        return true;
    }
    
    // Check WiFi
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(String(EMOJI_WARNING) + " MQTT: WiFi not connected");
        return false;
    }
    
    Serial.print(String(EMOJI_INFO) + " MQTT: Attempting connection as ");
    Serial.println(clientID.c_str());
    
    // Attempt connection with credentials if available
    bool success = false;
    if (username.length() > 0 && password.length() > 0) {
        success = mqttClient.connect(clientID.c_str(), username.c_str(), password.c_str());
    } else {
        success = mqttClient.connect(clientID.c_str());
    }
    
    if (success) {
        connected = true;
        Serial.println(String(EMOJI_SUCCESS) + " MQTT: Connected!");
        
        // Publish online status
        String statusTopic = baseTopic + "/status";
        mqttClient.publish(statusTopic.c_str(), "online", useRetain);
    } else {
        int state = mqttClient.state();
        Serial.print(String(EMOJI_ERROR) + " MQTT connection failed, code: ");
        Serial.println(state);
    }
    
    return success;
}

void SensythingMQTT::disconnect() {
    if (connected) {
        // Publish offline status
        String statusTopic = baseTopic + "/status";
        mqttClient.publish(statusTopic.c_str(), "offline", useRetain);
        
        mqttClient.disconnect();
        connected = false;
        
        Serial.println(String(EMOJI_INFO) + " MQTT: Disconnected");
    }
}

bool SensythingMQTT::reconnect() {
    if (connected) {
        return true;
    }
    
    // Check if enough time passed since last attempt
    unsigned long now = millis();
    if (now - lastReconnectAttempt < RECONNECT_INTERVAL) {
        return false;
    }
    
    lastReconnectAttempt = now;
    return connect();
}

void SensythingMQTT::streamData(const MeasurementData& data, const BoardConfig& config) {
    if (!initialized || !connected) {
        return;
    }
    
    // Publish individual channel values
    for (uint8_t i = 0; i < config.channelCount && i < data.channel_count; i++) {
        String topic = baseTopic + "/" + config.channels[i].label;
        String payload = String(data.channels[i], 2);  // 2 decimal places
        
        mqttClient.publish(topic.c_str(), payload.c_str(), useRetain);
    }
    
    // Publish consolidated JSON data
    String jsonTopic = baseTopic + "/data";
    String jsonData = formatAsJSON(data, config);
    mqttClient.publish(jsonTopic.c_str(), jsonData.c_str(), useRetain);
    
    // Update timestamp
    String timestampTopic = baseTopic + "/timestamp";
    String timestamp = String(millis() / 1000);  // Seconds since startup
    mqttClient.publish(timestampTopic.c_str(), timestamp.c_str(), useRetain);
}

void SensythingMQTT::update() {
    if (!initialized) {
        return;
    }
    
    if (!connected) {
        // Attempt reconnection
        reconnect();
    } else {
        // Process incoming messages
        mqttClient.loop();
    }
}

void SensythingMQTT::setBaseTopic(const char* baseTopic) {
    if (baseTopic) {
        this->baseTopic = baseTopic;
        Serial.print(String(EMOJI_INFO) + " MQTT base topic set to: ");
        Serial.println(baseTopic);
    }
}

void SensythingMQTT::setQoS(uint8_t qos) {
    if (qos <= 2) {
        qosLevel = qos;
        Serial.print(String(EMOJI_INFO) + " MQTT QoS set to: ");
        Serial.println(qos);
    }
}

bool SensythingMQTT::publish(const char* topic, const char* payload) {
    if (!connected) {
        Serial.println(String(EMOJI_WARNING) + " MQTT: Not connected, cannot publish");
        return false;
    }
    
    bool success = mqttClient.publish(topic, payload, useRetain);
    if (!success) {
        Serial.println(String(EMOJI_ERROR) + " MQTT: Publish failed");
    }
    return success;
}

int SensythingMQTT::getLastError() const {
    // Note: state() is const-safe, returning current state without modification
    return const_cast<PubSubClient&>(mqttClient).state();
}

String SensythingMQTT::getConnectionStatus() const {
    if (!initialized) {
        return "Not initialized";
    }
    
    if (connected) {
        return "Connected";
    }
    
    int state = const_cast<PubSubClient&>(mqttClient).state();
    switch (state) {
        case MQTT_CONNECTION_TIMEOUT:
            return "Connection timeout";
        case MQTT_CONNECTION_LOST:
            return "Connection lost";
        case MQTT_CONNECT_FAILED:
            return "Connection failed";
        case MQTT_DISCONNECTED:
            return "Disconnected";
        case MQTT_CONNECT_BAD_PROTOCOL:
            return "Bad protocol";
        case MQTT_CONNECT_BAD_CLIENT_ID:
            return "Bad client ID";
        case MQTT_CONNECT_UNAVAILABLE:
            return "Broker unavailable";
        case MQTT_CONNECT_BAD_CREDENTIALS:
            return "Bad credentials";
        case MQTT_CONNECT_UNAUTHORIZED:
            return "Unauthorized";
        default:
            return "Unknown state";
    }
}

String SensythingMQTT::formatAsJSON(const MeasurementData& data, const BoardConfig& config) {
    // Build JSON response
    // Format: {"timestamp":123456,"boardType":"Cap","channels":[...]}
    
    String json = "{\"timestamp\":" + String(data.timestamp);
    json += ",\"boardType\":\"" + String(config.boardType) + "\"";
    json += ",\"sampleCount\":" + String(data.measurement_count);
    json += ",\"channels\":[";
    
    for (uint8_t i = 0; i < config.channelCount && i < data.channel_count; i++) {
        if (i > 0) json += ",";
        json += "{\"name\":\"" + String(config.channels[i].label) + "\"";
        json += ",\"value\":" + String(data.channels[i], 2);
        json += ",\"unit\":\"" + String(config.channels[i].unit) + "\"}";
    }
    
    json += "]}";
    return json;
}

void SensythingMQTT::mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (instance) {
        instance->handleMessage(topic, payload, length);
    }
}

void SensythingMQTT::handleMessage(const char* topic, const byte* payload, unsigned int length) {
    // Future: Handle incoming MQTT messages (subscriptions)
    // Not implemented in MVP
    Serial.print(String(EMOJI_INFO) + " MQTT message received on topic: ");
    Serial.println(topic);
}
