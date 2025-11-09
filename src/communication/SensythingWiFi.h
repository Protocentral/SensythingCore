//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    SensythingES3 - WiFi Communication Module
//    WebSocket streaming with built-in web dashboard
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#ifndef SENSYTHING_WIFI_H
#define SENSYTHING_WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include "../core/SensythingTypes.h"
#include "../core/SensythingConfig.h"

// WiFi mode enumeration (prefixed with SENSYTHING_ to avoid ESP32 WiFi.h conflicts)
typedef enum {
    SENSYTHING_WIFI_MODE_AP,      // Access Point mode
    SENSYTHING_WIFI_MODE_STA,     // Station mode (connect to existing network)
    SENSYTHING_WIFI_MODE_APSTA    // Both AP and STA
} SensythingWiFiMode;

class SensythingWiFi {
public:
    SensythingWiFi();
    ~SensythingWiFi();
    
    /**
     * Initialize WiFi in Access Point mode
     * @param ssid AP name (e.g., "Sensything-Cap-XXXX")
     * @param password AP password (min 8 chars, empty = open network)
     * @param config Board configuration for web dashboard
     * @return true if initialization successful
     */
    bool initAP(String ssid, String password, const BoardConfig& config);
    
    /**
     * Initialize WiFi in Station mode (connect to existing network)
     * @param ssid Network SSID to connect to
     * @param password Network password
     * @param config Board configuration for web dashboard
     * @return true if connection successful
     */
    bool initStation(String ssid, String password, const BoardConfig& config);
    
    /**
     * Initialize WiFi in AP+Station mode (both simultaneously)
     * Starts AP for configuration portal while maintaining Station connection
     * @param apSSID AP network name
     * @param apPassword AP password (empty = open)
     * @param staSSID Station SSID (empty = AP only)
     * @param staPassword Station password
     * @param config Board configuration
     * @return true if initialization successful
     */
    bool initAPStation(String apSSID, String apPassword, String staSSID, String staPassword, const BoardConfig& config);
    
    /**
     * Connect to WiFi network (can be called from web interface)
     * @param ssid Network SSID
     * @param password Network password
     * @return true if connection successful
     */
    bool connectToNetwork(String ssid, String password);
    
    /**
     * Handle WiFi and WebSocket events (call in loop)
     */
    void update();
    
    /**
     * Stream measurement data via WebSocket
     * @param data Measurement data structure
     * @param config Board configuration for format adaptation
     */
    void streamData(const MeasurementData& data, const BoardConfig& config);
    
    /**
     * Check if WiFi is connected
     * @return true if connected (AP has clients or STA connected to network)
     */
    bool isConnected() const { return (wifiMode == SENSYTHING_WIFI_MODE_AP && clientCount > 0) || 
                                       (wifiMode == SENSYTHING_WIFI_MODE_STA && WiFi.status() == WL_CONNECTED); }
    
    /**
     * Check if WebSocket client is connected
     * @return true if at least one WebSocket client connected
     */
    bool hasClients() const { return clientCount > 0; }
    
    /**
     * Get IP address
     * @return IP address string (AP IP or STA IP)
     */
    String getIPAddress() const;
    
    /**
     * Get WiFi mode
     * @return Current WiFi mode
     */
    SensythingWiFiMode getMode() const { return wifiMode; }
    
    /**
     * Save WiFi credentials to NVS (persistent storage)
     * @param ssid Network SSID
     * @param password Network password
     * @return true if saved successfully
     */
    bool saveCredentials(String ssid, String password);
    
    /**
     * Load WiFi credentials from NVS
     * @param ssid Output - loaded SSID
     * @param password Output - loaded password
     * @return true if credentials exist and loaded successfully
     */
    bool loadCredentials(String& ssid, String& password);
    
    /**
     * Clear saved WiFi credentials from NVS
     * @return true if cleared successfully
     */
    bool clearCredentials();
    
    /**
     * Check if WiFi credentials are saved
     * @return true if credentials exist in NVS
     */
    bool hasStoredCredentials();
    
    /**
     * Get connected client count
     * @return Number of WebSocket clients
     */
    uint8_t getClientCount() const { return clientCount; }
    
private:
    WebServer* pWebServer;
    WebSocketsServer* pWebSocket;
    SensythingWiFiMode wifiMode;
    uint8_t clientCount;
    bool initialized;
    BoardConfig boardConfig;
    Preferences preferences;  // For persistent WiFi credential storage
    
    // Web server handlers
    void handleRoot();
    void handleNotFound();
    void handleDashboard();
    void handleAPI();
    
    // WebSocket event handler
    void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    static void webSocketEventStatic(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    static SensythingWiFi* instance;  // For static callback
    
    /**
     * Format measurement data as JSON for WebSocket
     * @param data Measurement data
     * @param config Board configuration
     * @return JSON string
     */
    String formatAsJSON(const MeasurementData& data, const BoardConfig& config);
    
    /**
     * Generate HTML dashboard
     * @return HTML string with embedded JavaScript for real-time plotting
     */
    String generateDashboardHTML();
    
    /**
     * Setup web server routes
     */
    void setupWebServer();
};

#endif // SENSYTHING_WIFI_H
