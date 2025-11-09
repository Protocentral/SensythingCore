//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything Platform - WiFi Communication Module
//    WebSocket streaming with built-in web dashboard
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "SensythingWiFi.h"

// Static instance for callback
SensythingWiFi* SensythingWiFi::instance = nullptr;

SensythingWiFi::SensythingWiFi() {
    pWebServer = nullptr;
    pWebSocket = nullptr;
    wifiMode = SENSYTHING_WIFI_MODE_AP;
    clientCount = 0;
    initialized = false;
    instance = this;  // Set static instance for callbacks
}

SensythingWiFi::~SensythingWiFi() {
    if (pWebSocket) {
        delete pWebSocket;
    }
    if (pWebServer) {
        delete pWebServer;
    }
    if (initialized) {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    }
}

bool SensythingWiFi::initAP(String ssid, String password, const BoardConfig& config) {
    if (initialized) {
        Serial.println(String(EMOJI_WARNING) + " WiFi already initialized");
        return true;
    }
    
    this->boardConfig = config;
    this->wifiMode = SENSYTHING_WIFI_MODE_AP;
    
    Serial.print(String(EMOJI_INFO) + " Starting WiFi AP: ");
    Serial.println(ssid);
    
    // Configure AP
    WiFi.mode(WIFI_AP);
    bool success;
    
    if (password.length() >= 8) {
        success = WiFi.softAP(ssid.c_str(), password.c_str());
        Serial.print(String(EMOJI_INFO) + " Password: ");
        Serial.println(password);
    } else {
        success = WiFi.softAP(ssid.c_str());  // Open network
        Serial.println(String(EMOJI_WARNING) + " Open network (no password)");
    }
    
    if (!success) {
        Serial.println(String(EMOJI_ERROR) + " Failed to start AP");
        return false;
    }
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print(String(EMOJI_SUCCESS) + " AP started. IP: ");
    Serial.println(IP);
    
    // Create web server (port 80)
    pWebServer = new WebServer(80);
    setupWebServer();
    pWebServer->begin();
    Serial.println(String(EMOJI_SUCCESS) + " Web server started on port 80");
    
    // Create WebSocket server (port 81)
    pWebSocket = new WebSocketsServer(81);
    Serial.println(String(EMOJI_INFO) + " Created WebSocket server object");
    
    // Set event handler BEFORE begin()
    pWebSocket->onEvent(webSocketEventStatic);
    Serial.println(String(EMOJI_INFO) + " WebSocket event handler registered");
    
    // Start WebSocket server
    pWebSocket->begin();
    Serial.println(String(EMOJI_SUCCESS) + " WebSocket server started on port 81");
    Serial.print(String(EMOJI_INFO) + " Static instance pointer: ");
    Serial.println(instance ? "SET" : "NULL");
    
    Serial.println(String(EMOJI_INFO) + " Connect to WiFi and open: http://" + IP.toString());
    Serial.println(String(EMOJI_INFO) + " WebSocket URL: ws://" + IP.toString() + ":81/");
    
    initialized = true;
    return true;
}

bool SensythingWiFi::initStation(String ssid, String password, const BoardConfig& config) {
    if (initialized) {
        Serial.println(String(EMOJI_WARNING) + " WiFi already initialized");
        return true;
    }
    
    this->boardConfig = config;
    this->wifiMode = SENSYTHING_WIFI_MODE_STA;
    
    Serial.print(String(EMOJI_INFO) + " Connecting to WiFi: ");
    Serial.println(ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // Wait for connection (30 second timeout)
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 60) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(String(EMOJI_ERROR) + " WiFi connection failed");
        return false;
    }
    
    IPAddress IP = WiFi.localIP();
    Serial.print(String(EMOJI_SUCCESS) + " Connected! IP: ");
    Serial.println(IP);
    
    // Start mDNS responder
    if (MDNS.begin("sensything")) {
        Serial.println(String(EMOJI_SUCCESS) + " mDNS responder started: sensything.local");
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("ws", "tcp", 81);
    } else {
        Serial.println(String(EMOJI_WARNING) + " mDNS failed to start");
    }
    
    // Create web server (port 80)
    pWebServer = new WebServer(80);
    setupWebServer();
    pWebServer->begin();
    Serial.println(String(EMOJI_SUCCESS) + " Web server started on port 80");
    
    // Create WebSocket server (port 81)
    pWebSocket = new WebSocketsServer(81);
    Serial.println(String(EMOJI_INFO) + " Created WebSocket server object");
    
    // Set event handler BEFORE begin()
    pWebSocket->onEvent(webSocketEventStatic);
    Serial.println(String(EMOJI_INFO) + " WebSocket event handler registered");
    
    // Start WebSocket server
    pWebSocket->begin();
    Serial.println(String(EMOJI_SUCCESS) + " WebSocket server started on port 81");
    Serial.print(String(EMOJI_INFO) + " Static instance pointer: ");
    Serial.println(instance ? "SET" : "NULL");
    
    Serial.println(String(EMOJI_INFO) + " Dashboard: http://sensything.local (or http://" + IP.toString() + ")");
    Serial.println(String(EMOJI_INFO) + " WebSocket: ws://sensything.local:81/ (or ws://" + IP.toString() + ":81/)");
    
    initialized = true;
    return true;
}

bool SensythingWiFi::initAPStation(String apSSID, String apPassword, String staSSID, String staPassword, const BoardConfig& config) {
    if (initialized) {
        Serial.println(String(EMOJI_WARNING) + " WiFi already initialized");
        return true;
    }
    
    this->boardConfig = config;
    this->wifiMode = SENSYTHING_WIFI_MODE_APSTA;
    
    Serial.println(String(EMOJI_INFO) + " Starting WiFi in AP+Station mode...");
    
    // Configure AP+Station mode
    WiFi.mode(WIFI_AP_STA);
    
    // Start Access Point
    bool apSuccess;
    if (apPassword.length() >= 8) {
        apSuccess = WiFi.softAP(apSSID.c_str(), apPassword.c_str());
        Serial.print(String(EMOJI_INFO) + " AP Password: ");
        Serial.println(apPassword);
    } else {
        apSuccess = WiFi.softAP(apSSID.c_str());
        Serial.println(String(EMOJI_WARNING) + " AP open network (no password)");
    }
    
    if (!apSuccess) {
        Serial.println(String(EMOJI_ERROR) + " Failed to start AP");
        return false;
    }
    
    IPAddress apIP = WiFi.softAPIP();
    Serial.print(String(EMOJI_SUCCESS) + " AP started. IP: ");
    Serial.println(apIP);
    
    // Try to load saved credentials if not provided
    String savedSSID = staSSID;
    String savedPassword = staPassword;
    
    if (savedSSID.length() == 0) {
        if (loadCredentials(savedSSID, savedPassword)) {
            Serial.println(String(EMOJI_INFO) + " Found saved WiFi credentials");
            staSSID = savedSSID;
            staPassword = savedPassword;
        }
    }
    
    // Try to connect to Station network if credentials available
    if (staSSID.length() > 0) {
        Serial.print(String(EMOJI_INFO) + " Connecting to WiFi: ");
        Serial.println(staSSID);
        
        WiFi.begin(staSSID.c_str(), staPassword.c_str());
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 30) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        Serial.println();
        
        if (WiFi.status() == WL_CONNECTED) {
            IPAddress staIP = WiFi.localIP();
            Serial.print(String(EMOJI_SUCCESS) + " Station connected! IP: ");
            Serial.println(staIP);
            
            // Save credentials if this was a new connection
            if (savedSSID.length() == 0 || savedSSID != staSSID) {
                saveCredentials(staSSID, staPassword);
            }
            
            // Start mDNS
            if (MDNS.begin("sensything")) {
                Serial.println(String(EMOJI_SUCCESS) + " mDNS: sensything.local");
                MDNS.addService("http", "tcp", 80);
                MDNS.addService("ws", "tcp", 81);
            }
        } else {
            Serial.println(String(EMOJI_WARNING) + " Station connection failed, AP-only mode");
        }
    } else {
        Serial.println(String(EMOJI_INFO) + " No Station credentials, AP-only mode");
    }
    
    // Create web server
    pWebServer = new WebServer(80);
    setupWebServer();
    pWebServer->begin();
    Serial.println(String(EMOJI_SUCCESS) + " Web server started on port 80");
    
    // Create WebSocket server
    pWebSocket = new WebSocketsServer(81);
    pWebSocket->onEvent(webSocketEventStatic);
    pWebSocket->begin();
    Serial.println(String(EMOJI_SUCCESS) + " WebSocket server started on port 81");
    
    Serial.println(String(EMOJI_INFO) + " Configuration portal: http://" + apIP.toString());
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(String(EMOJI_INFO) + " Dashboard: http://sensything.local");
    }
    
    initialized = true;
    return true;
}

bool SensythingWiFi::connectToNetwork(String ssid, String password) {
    Serial.print(String(EMOJI_INFO) + " Connecting to: ");
    Serial.println(ssid);
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        IPAddress ip = WiFi.localIP();
        Serial.print(String(EMOJI_SUCCESS) + " Connected! IP: ");
        Serial.println(ip);
        
        // Save credentials to NVS for auto-connect on next boot
        saveCredentials(ssid, password);
        
        // Start/restart mDNS
        if (MDNS.begin("sensything")) {
            Serial.println(String(EMOJI_SUCCESS) + " mDNS: sensything.local");
            MDNS.addService("http", "tcp", 80);
            MDNS.addService("ws", "tcp", 81);
        }
        
        return true;
    }
    
    Serial.println(String(EMOJI_ERROR) + " Connection failed");
    return false;
}

bool SensythingWiFi::saveCredentials(String ssid, String password) {
    preferences.begin("wifi", false);  // false = read/write mode
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.putBool("saved", true);
    preferences.end();
    
    Serial.println(String(EMOJI_SUCCESS) + " WiFi credentials saved to NVS");
    return true;
}

bool SensythingWiFi::loadCredentials(String& ssid, String& password) {
    preferences.begin("wifi", true);  // true = read-only mode
    
    bool saved = preferences.getBool("saved", false);
    if (!saved) {
        preferences.end();
        return false;
    }
    
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");
    preferences.end();
    
    if (ssid.length() == 0) {
        return false;
    }
    
    Serial.println(String(EMOJI_INFO) + " Loaded saved WiFi credentials");
    return true;
}

bool SensythingWiFi::clearCredentials() {
    preferences.begin("wifi", false);
    preferences.clear();
    preferences.end();
    
    Serial.println(String(EMOJI_INFO) + " WiFi credentials cleared");
    return true;
}

bool SensythingWiFi::hasStoredCredentials() {
    preferences.begin("wifi", true);
    bool saved = preferences.getBool("saved", false);
    preferences.end();
    return saved;
}

void SensythingWiFi::update() {
    if (!initialized) {
        return;
    }
    
    // Handle HTTP requests
    pWebServer->handleClient();
    
    // Handle WebSocket events
    if (pWebSocket) {
        pWebSocket->loop();
    }
}

void SensythingWiFi::streamData(const MeasurementData& data, const BoardConfig& config) {
    if (!initialized) {
        return;
    }
    
    if (clientCount == 0) {
        return;  // No clients connected
    }
    
    // Format as JSON and broadcast to all WebSocket clients
    String jsonData = formatAsJSON(data, config);
    pWebSocket->broadcastTXT(jsonData);
}

String SensythingWiFi::getIPAddress() const {
    if (wifiMode == SENSYTHING_WIFI_MODE_AP) {
        return WiFi.softAPIP().toString();
    } else {
        return WiFi.localIP().toString();
    }
}

void SensythingWiFi::webSocketEventStatic(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (instance) {
        instance->webSocketEvent(num, type, payload, length);
    }
}

void SensythingWiFi::webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    Serial.print("[WS] Event type: ");
    Serial.print(type);
    Serial.print(" from client #");
    Serial.println(num);
    
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.print(String(EMOJI_INFO) + " WebSocket client #");
            Serial.print(num);
            Serial.println(" disconnected");
            if (clientCount > 0) clientCount--;
            break;
            
        case WStype_CONNECTED:
            {
                IPAddress ip = pWebSocket->remoteIP(num);
                Serial.print(String(EMOJI_SUCCESS) + " WebSocket client #");
                Serial.print(num);
                Serial.print(" connected from ");
                Serial.println(ip.toString());
                clientCount++;
                
                // Send welcome message with board info
                String welcome = "{\"type\":\"info\",\"board\":\"" + String(boardConfig.channels[0].label) + 
                               "\",\"channels\":" + String(boardConfig.channelCount) + "}";
                pWebSocket->sendTXT(num, welcome);
            }
            break;
            
        case WStype_TEXT:
            // Handle commands from web dashboard
            Serial.print(String(EMOJI_INFO) + " WebSocket message from #");
            Serial.print(num);
            Serial.print(": ");
            Serial.println((char*)payload);
            // TODO: Implement command handling (start/stop, set rate, etc.)
            break;
            
        case WStype_ERROR:
            Serial.print(String(EMOJI_ERROR) + " WebSocket error on #");
            Serial.println(num);
            break;
            
        default:
            break;
    }
}

String SensythingWiFi::formatAsJSON(const MeasurementData& data, const BoardConfig& config) {
    // Lightweight JSON formatting (no library needed for simple structure)
    String json = "{\"ts\":";
    json += String(data.timestamp);
    json += ",\"cnt\":";
    json += String(data.measurement_count);
    json += ",\"ch\":[";
    
    for (int i = 0; i < config.channelCount; i++) {
        if (i > 0) json += ",";
        
        // Check if channel is valid
        bool valid = !(data.status_flags & (1 << i));
        
        if (valid) {
            json += String(data.channels[i], 4);  // 4 decimal places
        } else {
            json += "null";
        }
    }
    
    json += "],\"flags\":";
    json += String(data.status_flags);
    json += "}";
    
    return json;
}

void SensythingWiFi::setupWebServer() {
    // Root page - redirect to dashboard
    pWebServer->on("/", [this]() {
        pWebServer->sendHeader("Location", "/dashboard");
        pWebServer->send(302);
    });
    
    // Dashboard page
    pWebServer->on("/dashboard", [this]() {
        pWebServer->send(200, "text/html", generateDashboardHTML());
    });
    
    // API endpoint for status
    pWebServer->on("/api/status", [this]() {
        String json = "{\"connected\":true,\"clients\":";
        json += String(clientCount);
        json += ",\"board\":\"" + String(boardConfig.channels[0].label) + "\"";
        json += ",\"channels\":" + String(boardConfig.channelCount);
        json += ",\"mode\":\"";
        json += (wifiMode == SENSYTHING_WIFI_MODE_AP) ? "AP" : 
                (wifiMode == SENSYTHING_WIFI_MODE_STA) ? "Station" : "AP+Station";
        json += "\",\"staConnected\":";
        json += (WiFi.status() == WL_CONNECTED) ? "true" : "false";
        json += ",\"staIP\":\"" + WiFi.localIP().toString() + "\"";
        json += ",\"apIP\":\"" + WiFi.softAPIP().toString() + "\"";
        json += ",\"savedCreds\":";
        json += hasStoredCredentials() ? "true" : "false";
        json += "}";
        pWebServer->send(200, "application/json", json);
    });
    
    // API endpoint for WiFi configuration
    pWebServer->on("/api/wifi/connect", HTTP_POST, [this]() {
        if (!pWebServer->hasArg("ssid") || !pWebServer->hasArg("password")) {
            pWebServer->send(400, "application/json", "{\"success\":false,\"error\":\"Missing parameters\"}");
            return;
        }
        
        String ssid = pWebServer->arg("ssid");
        String password = pWebServer->arg("password");
        
        Serial.println(String(EMOJI_INFO) + " WiFi connect request via API");
        bool success = connectToNetwork(ssid, password);
        
        String json = "{\"success\":";
        json += success ? "true" : "false";
        if (success) {
            json += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
        }
        json += "}";
        
        pWebServer->send(200, "application/json", json);
    });
    
    // API endpoint for WiFi scan
    pWebServer->on("/api/wifi/scan", [this]() {
        Serial.println(String(EMOJI_INFO) + " WiFi scan request");
        int n = WiFi.scanNetworks();
        
        String json = "{\"networks\":[";
        for (int i = 0; i < n; i++) {
            if (i > 0) json += ",";
            json += "{\"ssid\":\"" + WiFi.SSID(i) + "\"";
            json += ",\"rssi\":" + String(WiFi.RSSI(i));
            json += ",\"secure\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
            json += "}";
        }
        json += "]}";
        
        pWebServer->send(200, "application/json", json);
    });
    
    // API endpoint for sending commands to device
    pWebServer->on("/api/command", HTTP_POST, [this]() {
        if (!pWebServer->hasArg("cmd")) {
            pWebServer->send(400, "application/json", "{\"success\":false}");
            return;
        }
        
        String cmd = pWebServer->arg("cmd");
        Serial.println(String(EMOJI_INFO) + " Command via API: " + cmd);
        
        // Commands will be handled by the main application
        // For now, just acknowledge receipt
        pWebServer->send(200, "application/json", "{\"success\":true,\"cmd\":\"" + cmd + "\"}");
    });
    
    // API endpoint to clear saved credentials
    pWebServer->on("/api/wifi/forget", HTTP_POST, [this]() {
        bool success = clearCredentials();
        String json = "{\"success\":";
        json += success ? "true" : "false";
        json += "}";
        pWebServer->send(200, "application/json", json);
    });
    
    // 404 handler
    pWebServer->onNotFound([this]() {
        pWebServer->send(404, "text/plain", "404: Not found");
    });
}

String SensythingWiFi::generateDashboardHTML() {
    // Embedded HTML dashboard with real-time chart
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sensything Platform - Live Dashboard</title>
    <style>
        * {
            box-sizing: border-box;
        }
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
            margin: 0;
            padding: 10px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: #333;
        }
        .container {
            max-width: 1400px;
            margin: 0 auto;
            background: white;
            border-radius: 12px;
            padding: 20px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.2);
        }
        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 15px;
            padding-bottom: 15px;
            border-bottom: 2px solid #e0e0e0;
        }
        h1 {
            margin: 0;
            color: #667eea;
            font-size: 28px;
        }
        .subtitle {
            margin: 0;
            color: #666;
            font-size: 13px;
        }
        .status {
            display: flex;
            gap: 15px;
            align-items: center;
        }
        .status-item {
            display: flex;
            align-items: center;
            gap: 6px;
            padding: 6px 12px;
            background: #f5f5f5;
            border-radius: 20px;
            font-size: 13px;
        }
        .status-dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
            background: #4caf50;
            animation: pulse 2s infinite;
        }
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        /* ===== TOOLBAR SECTION ===== */
        .toolbar {
            background: #f8f9fa;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            padding: 12px;
            margin-bottom: 15px;
            display: flex;
            gap: 12px;
            align-items: center;
            flex-wrap: wrap;
        }
        .toolbar-label {
            font-size: 12px;
            font-weight: 600;
            color: #667;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            min-width: 80px;
        }
        .toolbar-divider {
            width: 1px;
            height: 24px;
            background: #ddd;
        }
        .toolbar-group {
            display: flex;
            gap: 8px;
            align-items: center;
        }
        .toolbar-select {
            padding: 6px 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
            font-size: 12px;
            background: white;
            cursor: pointer;
            min-width: 120px;
        }
        .toolbar-select:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 2px rgba(102, 126, 234, 0.1);
        }
        .toolbar-btn {
            padding: 6px 12px;
            border: 1px solid #ddd;
            border-radius: 4px;
            background: white;
            color: #333;
            cursor: pointer;
            font-size: 12px;
            font-weight: 500;
            transition: all 0.2s;
            white-space: nowrap;
        }
        .toolbar-btn:hover {
            background: #f0f0f0;
            border-color: #bbb;
        }
        .toolbar-btn.active {
            background: #667eea;
            color: white;
            border-color: #667eea;
        }
        .toolbar-btn-sm {
            padding: 5px 10px;
            font-size: 11px;
        }
        .window-control {
            display: flex;
            gap: 6px;
            align-items: center;
        }
        .window-value {
            min-width: 40px;
            text-align: center;
            font-weight: 600;
            color: #667eea;
        }
        /* ===== CHART SECTION ===== */
        .chart-container {
            position: relative;
            height: 500px;
            margin-bottom: 15px;
            background: white;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            overflow: hidden;
        }
        #chartCanvas {
            width: 100%;
            height: 100%;
        }
        /* ===== CHANNEL VALUES GRID ===== */
        .info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
            gap: 12px;
            margin-bottom: 15px;
        }
        .info-card {
            padding: 12px;
            background: #f9f9f9;
            border-radius: 6px;
            border-left: 3px solid #667eea;
            text-align: center;
        }
        .info-label {
            font-size: 11px;
            color: #666;
            margin-bottom: 4px;
            text-transform: uppercase;
        }
        .info-value {
            font-size: 22px;
            font-weight: bold;
            color: #333;
        }
        /* ===== CONFIGURATION PANEL (COLLAPSIBLE) ===== */
        .config-panel {
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            background: #f9f9f9;
            margin-bottom: 15px;
            overflow: hidden;
        }
        .config-header {
            padding: 12px;
            background: #f0f0f0;
            border-bottom: 1px solid #e0e0e0;
            cursor: pointer;
            display: flex;
            justify-content: space-between;
            align-items: center;
            user-select: none;
        }
        .config-header:hover {
            background: #e8e8e8;
        }
        .config-header h3 {
            margin: 0;
            font-size: 14px;
            color: #667eea;
        }
        .config-toggle {
            font-size: 18px;
            transition: transform 0.3s;
        }
        .config-toggle.collapsed {
            transform: rotate(-90deg);
        }
        .config-content {
            padding: 15px;
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 15px;
            max-height: 1000px;
            overflow-y: auto;
            transition: max-height 0.3s, opacity 0.3s;
        }
        .config-content.hidden {
            max-height: 0;
            padding: 0;
            opacity: 0;
            display: none;
        }
        .config-item {
            display: flex;
            flex-direction: column;
            gap: 5px;
        }
        .config-label {
            font-size: 12px;
            font-weight: 600;
            color: #333;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        .config-input, .config-select {
            padding: 8px 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
            font-size: 13px;
        }
        .config-input:focus, .config-select:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 2px rgba(102, 126, 234, 0.1);
        }
        .config-buttons {
            display: flex;
            gap: 8px;
            flex-wrap: wrap;
        }
        .btn {
            padding: 8px 14px;
            border: none;
            border-radius: 4px;
            background: #667eea;
            color: white;
            cursor: pointer;
            font-size: 12px;
            font-weight: 500;
            transition: background 0.2s;
            white-space: nowrap;
        }
        .btn:hover {
            background: #5568d3;
        }
        .btn:disabled {
            background: #ccc;
            cursor: not-allowed;
        }
        .btn-success {
            background: #48bb78;
        }
        .btn-success:hover {
            background: #38a169;
        }
        .btn-danger {
            background: #f56565;
        }
        .btn-danger:hover {
            background: #e53e3e;
        }
        .btn-sm {
            padding: 6px 10px;
            font-size: 11px;
        }
        .config-info {
            font-size: 12px;
            color: #666;
            margin-top: 8px;
        }
        .status-badge {
            display: inline-block;
            padding: 3px 10px;
            border-radius: 12px;
            font-size: 11px;
            font-weight: 600;
        }
        .badge-success {
            background: #c6f6d5;
            color: #22543d;
        }
        .badge-warning {
            background: #feebc8;
            color: #7c2d12;
        }
        .badge-info {
            background: #bee3f8;
            color: #2c5282;
        }
        .footer {
            margin-top: 15px;
            padding-top: 15px;
            border-top: 1px solid #e0e0e0;
            text-align: center;
            color: #666;
            font-size: 12px;
        }
    </style>
</head>
<body>
    <div class="container">
        <!-- HEADER -->
        <div class="header">
            <div>
                <h1>🎛️ Sensything</h1>
                <p class="subtitle">Real-time Sensor Monitoring</p>
            </div>
            <div class="status">
                <div class="status-item">
                    <div class="status-dot" id="wsStatus"></div>
                    <span id="wsText">Connecting...</span>
                </div>
                <div class="status-item">
                    📊 <span id="sampleCount">0</span> samples
                </div>
            </div>
        </div>
        
        <!-- COMPACT TOOLBAR -->
        <div class="toolbar">
            <span class="toolbar-label">⚙️ Controls</span>
            <div class="toolbar-divider"></div>
            
            <div class="toolbar-group">
                <button class="toolbar-btn toolbar-btn-sm" onclick="sendCommand('start_all')">▶️ Start</button>
                <button class="toolbar-btn toolbar-btn-sm" onclick="sendCommand('stop_all')">⏹️ Stop</button>
            </div>
            
            <div class="toolbar-divider"></div>
            
            <div class="toolbar-group">
                <label style="font-size: 12px; margin: 0;">Sample Rate:</label>
                <select class="toolbar-select" id="sampleRate" onchange="setSampleRate()">
                    <option value="100">10 Hz</option>
                    <option value="50">20 Hz</option>
                    <option value="20">50 Hz</option>
                    <option value="10">100 Hz</option>
                    <option value="200">5 Hz</option>
                    <option value="500">2 Hz</option>
                    <option value="1000">1 Hz</option>
                </select>
            </div>
            
            <div class="toolbar-divider"></div>
            
            <div class="toolbar-group">
                <label style="font-size: 12px; margin: 0;">Window Size:</label>
                <div class="window-control">
                    <button class="toolbar-btn toolbar-btn-sm" onclick="decreaseWindowSize()">−</button>
                    <div class="window-value" id="windowValue">500</div>
                    <button class="toolbar-btn toolbar-btn-sm" onclick="increaseWindowSize()">+</button>
                </div>
                <select class="toolbar-select" id="windowPreset" onchange="setWindowPreset()">
                    <option value="">Presets</option>
                    <option value="100">100 pts</option>
                    <option value="250">250 pts</option>
                    <option value="500">500 pts</option>
                    <option value="1000">1000 pts</option>
                    <option value="2000">2000 pts</option>
                </select>
            </div>
            
            <div class="toolbar-divider"></div>
            
            <div class="toolbar-group">
                <button class="toolbar-btn toolbar-btn-sm" id="pauseBtn" onclick="togglePause()">⏸️ Pause</button>
                <button class="toolbar-btn toolbar-btn-sm" onclick="clearChart()">🗑️ Clear</button>
            </div>
        </div>
        
        <!-- CHART -->
        <div class="chart-container">
            <canvas id="chartCanvas"></canvas>
        </div>
        
        <!-- CHANNEL VALUES -->
        <div class="info-grid" id="channelValues"></div>
        
        <!-- CONFIGURATION PANEL (COLLAPSIBLE) -->
        <div class="config-panel">
            <div class="config-header" onclick="toggleConfigPanel()">
                <h3>⚙️ Configuration</h3>
                <span class="config-toggle" id="configToggle">▼</span>
            </div>
            <div class="config-content" id="configContent">
                <!-- WiFi Configuration -->
                <div class="config-item">
                    <div class="config-label">🌐 Network</div>
                    <select class="config-select" id="wifiSSID" onchange="updatePassword()">
                        <option value="">-- Select Network --</option>
                    </select>
                    <button class="btn btn-sm" onclick="scanNetworks()">🔍 Scan</button>
                </div>
                
                <div class="config-item">
                    <div class="config-label">🔐 Password</div>
                    <input type="password" class="config-input" id="wifiPassword" placeholder="Network password">
                    <div class="config-buttons">
                        <button class="btn btn-success btn-sm" onclick="connectWiFi()">Connect</button>
                        <button class="btn btn-danger btn-sm" onclick="forgetNetwork()">Forget</button>
                    </div>
                    <div id="wifiStatus" style="margin-top: 6px; font-size: 12px;"></div>
                </div>
                
                <!-- Measurement Settings -->
                <div class="config-item">
                    <div class="config-label">📊 SD Logging</div>
                    <div class="config-buttons">
                        <button class="btn btn-success btn-sm" onclick="sendCommand('enable_sd')">Enable</button>
                        <button class="btn btn-danger btn-sm" onclick="sendCommand('disable_sd')">Disable</button>
                    </div>
                </div>
                
                <!-- System Info -->
                <div class="config-item">
                    <div class="config-label">📡 WiFi Mode</div>
                    <span id="wifiMode" class="status-badge badge-info">Loading...</span>
                </div>
                
                <div class="config-item">
                    <div class="config-label">📍 Station Status</div>
                    <span id="staStatus" class="status-badge badge-warning">Disconnected</span>
                    <div id="staIP" style="font-family: monospace; font-size: 12px; margin-top: 4px; color: #666;">--</div>
                </div>
                
                <div class="config-item">
                    <div class="config-label">🏠 AP IP</div>
                    <div id="apIP" style="font-family: monospace; font-size: 12px; color: #666;">--</div>
                    <div id="savedCredsInfo" style="margin-top: 8px; font-size: 11px; color: #666;"></div>
                </div>
            </div>
        </div>
        
        <div class="footer">
            <p>Protocentral Electronics © 2025 | WebSocket: <span id="wsUrl"></span></p>
        </div>
    </div>

    <script>
        const canvas = document.getElementById('chartCanvas');
        const ctx = canvas.getContext('2d');
        let maxDataPoints = 500;
        let paused = false;
        let sampleCount = 0;
        let channelData = [];
        let channelCount = 4;
        const colors = ['#667eea', '#f56565', '#48bb78', '#ed8936'];
        
        // Initialize data arrays
        for (let i = 0; i < 4; i++) {
            channelData.push([]);
        }
        
        function resizeCanvas() {
            canvas.width = canvas.clientWidth;
            canvas.height = canvas.clientHeight;
            drawChart();
        }
        
        window.addEventListener('resize', resizeCanvas);
        resizeCanvas();
        
        // ===== WINDOW SIZE CONTROLS =====
        function increaseWindowSize() {
            const step = maxDataPoints >= 1000 ? 500 : (maxDataPoints >= 500 ? 250 : 100);
            setWindowSize(maxDataPoints + step);
        }
        
        function decreaseWindowSize() {
            const step = maxDataPoints > 1000 ? 500 : (maxDataPoints > 500 ? 250 : 100);
            if (maxDataPoints - step >= 50) {
                setWindowSize(maxDataPoints - step);
            }
        }
        
        function setWindowSize(size) {
            const minSize = 50;
            const maxSize = 5000;
            
            size = Math.max(minSize, Math.min(maxSize, size));
            maxDataPoints = size;
            document.getElementById('windowValue').textContent = size;
            document.getElementById('windowPreset').value = '';
            
            // Trim existing data if necessary
            channelData.forEach(channel => {
                if (channel.length > maxDataPoints) {
                    channel.splice(0, channel.length - maxDataPoints);
                }
            });
            
            drawChart();
        }
        
        function setWindowPreset() {
            const preset = document.getElementById('windowPreset').value;
            if (preset) {
                setWindowSize(parseInt(preset));
            }
        }
        
        // ===== CONFIGURATION PANEL TOGGLE =====
        function toggleConfigPanel() {
            const content = document.getElementById('configContent');
            const toggle = document.getElementById('configToggle');
            
            content.classList.toggle('hidden');
            toggle.classList.toggle('collapsed');
        }
        
        // WebSocket connection
        const wsUrl = 'ws://' + window.location.hostname + ':81/';
        document.getElementById('wsUrl').textContent = wsUrl;
        const ws = new WebSocket(wsUrl);
        
        ws.onopen = () => {
            document.getElementById('wsStatus').style.background = '#4caf50';
            document.getElementById('wsText').textContent = 'Connected';
            console.log('WebSocket connected');
        };
        
        ws.onclose = () => {
            document.getElementById('wsStatus').style.background = '#f44336';
            document.getElementById('wsText').textContent = 'Disconnected';
            console.log('WebSocket disconnected');
        };
        
        ws.onerror = (error) => {
            document.getElementById('wsStatus').style.background = '#ff9800';
            document.getElementById('wsText').textContent = 'Error';
            console.error('WebSocket error:', error);
        };
        
        ws.onmessage = (event) => {
            if (paused) return;
            
            try {
                const data = JSON.parse(event.data);
                
                if (data.type === 'info') {
                    // Board info message
                    initializeChart(data.channels);
                    return;
                }
                
                if (data.ch) {
                    updateChart(data);
                    updateChannelValues(data.ch);
                    sampleCount++;
                    document.getElementById('sampleCount').textContent = sampleCount;
                }
            } catch (e) {
                console.error('Parse error:', e);
            }
        };
        
        function drawChart() {
            const width = canvas.width;
            const height = canvas.height;
            const padding = 50;
            
            // Clear canvas
            ctx.clearRect(0, 0, width, height);
            ctx.fillStyle = '#f9f9f9';
            ctx.fillRect(0, 0, width, height);
            
            // Find min/max values
            let minVal = Infinity;
            let maxVal = -Infinity;
            channelData.forEach(channel => {
                channel.forEach(point => {
                    if (point.y < minVal) minVal = point.y;
                    if (point.y > maxVal) maxVal = point.y;
                });
            });
            
            if (minVal === Infinity) {
                minVal = 0;
                maxVal = 1;
            }
            
            const range = maxVal - minVal || 1;
            const xScale = (width - 2 * padding) / Math.max(1, maxDataPoints);
            const yScale = (height - 2 * padding) / range;
            
            // Draw grid
            ctx.strokeStyle = '#e0e0e0';
            ctx.lineWidth = 1;
            for (let i = 0; i <= 5; i++) {
                const y = padding + (height - 2 * padding) * i / 5;
                ctx.beginPath();
                ctx.moveTo(padding, y);
                ctx.lineTo(width - padding, y);
                ctx.stroke();
            }
            
            // Draw data lines
            channelData.forEach((channel, idx) => {
                if (channel.length === 0) return;
                
                ctx.strokeStyle = colors[idx];
                ctx.lineWidth = 2;
                ctx.beginPath();
                
                channel.forEach((point, i) => {
                    const x = padding + i * xScale;
                    const y = height - padding - (point.y - minVal) * yScale;
                    
                    if (i === 0) {
                        ctx.moveTo(x, y);
                    } else {
                        ctx.lineTo(x, y);
                    }
                });
                
                ctx.stroke();
            });
            
            // Draw legend
            ctx.font = '14px Arial';
            channelData.forEach((channel, idx) => {
                const x = padding + idx * 100;
                ctx.fillStyle = colors[idx];
                ctx.fillRect(x, 10, 15, 15);
                ctx.fillStyle = '#333';
                ctx.fillText('Ch' + idx, x + 20, 22);
            });
        }
        
        function initializeChart(channels) {
            channelCount = channels;
            const grid = document.getElementById('channelValues');
            grid.innerHTML = '';
            for (let i = 0; i < channels; i++) {
                grid.innerHTML += `
                    <div class="info-card">
                        <div class="info-label">Channel ${i}</div>
                        <div class="info-value" id="ch${i}">--</div>
                    </div>
                `;
            }
        }
        
        function updateChart(data) {
            data.ch.forEach((value, i) => {
                if (value !== null && i < channelData.length) {
                    channelData[i].push({
                        x: data.cnt,
                        y: value
                    });
                    
                    if (channelData[i].length > maxDataPoints) {
                        channelData[i].shift();
                    }
                }
            });
            
            drawChart();
        }
        
        function updateChannelValues(channels) {
            channels.forEach((value, i) => {
                const el = document.getElementById('ch' + i);
                if (el) {
                    el.textContent = value !== null ? value.toFixed(4) : 'Error';
                    el.style.color = value !== null ? '#333' : '#f44336';
                }
            });
        }
        
        function clearChart() {
            channelData.forEach(channel => channel.length = 0);
            drawChart();
            sampleCount = 0;
            document.getElementById('sampleCount').textContent = '0';
        }
        
        function togglePause() {
            paused = !paused;
            const btn = document.getElementById('pauseBtn');
            btn.textContent = paused ? '⏯️ Resume' : '⏸️ Pause';
            btn.classList.toggle('active', paused);
        }
        
        // Control Panel Functions
        
        function updateSystemInfo() {
            fetch('/api/status')
                .then(r => r.json())
                .then(data => {
                    document.getElementById('wifiMode').textContent = data.mode;
                    document.getElementById('staStatus').textContent = data.staConnected ? 'Connected' : 'Disconnected';
                    document.getElementById('staStatus').className = 'status-badge ' + (data.staConnected ? 'badge-success' : 'badge-warning');
                    document.getElementById('staIP').textContent = data.staConnected ? data.staIP : '--';
                    document.getElementById('apIP').textContent = data.apIP !== '0.0.0.0' ? data.apIP : '--';
                    
                    // Show saved credentials info
                    const savedInfo = document.getElementById('savedCredsInfo');
                    if (data.savedCreds) {
                        savedInfo.innerHTML = '💾 <span style="color: #48bb78;">Saved credentials will auto-connect on restart</span>';
                    } else {
                        savedInfo.innerHTML = 'ℹ️ No saved credentials - will start in AP mode after restart';
                    }
                })
                .catch(e => console.error('Status update failed:', e));
        }
        
        function scanNetworks() {
            const btn = event.target;
            btn.disabled = true;
            btn.textContent = '🔄 Scanning...';
            
            fetch('/api/wifi/scan')
                .then(r => r.json())
                .then(data => {
                    const select = document.getElementById('wifiSSID');
                    select.innerHTML = '<option value="">-- Select Network --</option>';
                    
                    data.networks.forEach(net => {
                        const option = document.createElement('option');
                        option.value = net.ssid;
                        option.textContent = net.ssid + ' (' + net.rssi + ' dBm) ' + (net.secure ? '🔒' : '');
                        option.dataset.secure = net.secure;
                        select.appendChild(option);
                    });
                    
                    btn.disabled = false;
                    btn.textContent = '🔍 Scan Networks';
                })
                .catch(e => {
                    console.error('Scan failed:', e);
                    btn.disabled = false;
                    btn.textContent = '🔍 Scan Networks';
                });
        }
        
        function updatePassword() {
            const select = document.getElementById('wifiSSID');
            const pwdInput = document.getElementById('wifiPassword');
            const option = select.options[select.selectedIndex];
            
            if (option && option.dataset.secure === 'false') {
                pwdInput.value = '';
                pwdInput.disabled = true;
                pwdInput.placeholder = 'Open network (no password)';
            } else {
                pwdInput.disabled = false;
                pwdInput.placeholder = 'Network password';
            }
        }
        
        function connectWiFi() {
            const ssid = document.getElementById('wifiSSID').value;
            const password = document.getElementById('wifiPassword').value;
            const status = document.getElementById('wifiStatus');
            
            if (!ssid) {
                status.innerHTML = '<span style="color: #f56565;">Please select a network</span>';
                return;
            }
            
            status.innerHTML = '<span style="color: #667eea;">Connecting...</span>';
            
            const formData = new FormData();
            formData.append('ssid', ssid);
            formData.append('password', password);
            
            fetch('/api/wifi/connect', {
                method: 'POST',
                body: new URLSearchParams(formData)
            })
                .then(r => r.json())
                .then(data => {
                    if (data.success) {
                        status.innerHTML = '<span style="color: #48bb78;">✓ Connected! IP: ' + data.ip + '</span>';
                        updateSystemInfo();
                    } else {
                        status.innerHTML = '<span style="color: #f56565;">✗ Connection failed</span>';
                    }
                })
                .catch(e => {
                    console.error('Connect failed:', e);
                    status.innerHTML = '<span style="color: #f56565;">✗ Error</span>';
                });
        }
        
        function forgetNetwork() {
            if (!confirm('Forget saved WiFi credentials? Device will start in AP-only mode after restart.')) {
                return;
            }
            
            fetch('/api/wifi/forget', {
                method: 'POST'
            })
                .then(r => r.json())
                .then(data => {
                    if (data.success) {
                        document.getElementById('wifiStatus').innerHTML = '<span style="color: #48bb78;">✓ Credentials cleared</span>';
                        updateSystemInfo();
                    } else {
                        document.getElementById('wifiStatus').innerHTML = '<span style="color: #f56565;">✗ Failed to clear</span>';
                    }
                })
                .catch(e => {
                    console.error('Forget failed:', e);
                });
        }
        
        function setSampleRate() {
            const rate = document.getElementById('sampleRate').value;
            sendCommand('set_rate ' + rate);
        }
        
        function sendCommand(cmd) {
            const formData = new FormData();
            formData.append('cmd', cmd);
            
            fetch('/api/command', {
                method: 'POST',
                body: new URLSearchParams(formData)
            })
                .then(r => r.json())
                .then(data => {
                    console.log('Command sent:', cmd, data);
                })
                .catch(e => console.error('Command failed:', e));
        }
        
        // Update system info every 5 seconds
        setInterval(updateSystemInfo, 5000);
        updateSystemInfo();
        
        // Auto-scan networks on load
        setTimeout(scanNetworks, 1000);
    </script>
</body>
</html>
)rawliteral";
    
    return html;
}
