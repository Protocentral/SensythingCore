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
    pDNSServer = nullptr;
    wifiMode = SENSYTHING_WIFI_MODE_AP;
    clientCount = 0;
    initialized = false;
    captivePortalActive = false;
    instance = this;  // Set static instance for callbacks
}

SensythingWiFi::~SensythingWiFi() {
    if (pWebSocket) {
        delete pWebSocket;
    }
    if (pWebServer) {
        delete pWebServer;
    }
    if (pDNSServer) {
        delete pDNSServer;
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
    Serial.flush();
    
    // Small delay before mDNS
    delay(100);
    
    // Start mDNS responder for AP mode
    if (MDNS.begin("sensything")) {
        Serial.println(String(EMOJI_SUCCESS) + " mDNS responder started: sensything.local");
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("ws", "tcp", 81);
    } else {
        Serial.println(String(EMOJI_WARNING) + " mDNS initialization skipped");
    }
    Serial.flush();
    
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
    
    // Setup captive portal DNS redirect
    if (!pDNSServer) {
        pDNSServer = new DNSServer();
    }
    // Redirect all DNS requests to AP IP address (192.168.4.1)
    pDNSServer->setErrorReplyCode(DNSReplyCode::NoError);
    pDNSServer->start(53, "*", IP);  // Port 53, catch all domains, AP IP
    captivePortalActive = true;
    Serial.println(String(EMOJI_SUCCESS) + " Captive portal DNS started on port 53");
    
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
    Serial.flush();
    
    delay(100);
    
    // Start mDNS responder
    if (MDNS.begin("sensything")) {
        Serial.println(String(EMOJI_SUCCESS) + " mDNS responder started: sensything.local");
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("ws", "tcp", 81);
    } else {
        Serial.println(String(EMOJI_WARNING) + " mDNS initialization skipped");
    }
    Serial.flush();
    
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
        } else {
            Serial.println(String(EMOJI_WARNING) + " Station connection failed, AP-only mode");
        }
    } else {
        Serial.println(String(EMOJI_INFO) + " No Station credentials, AP-only mode");
    }
    
    Serial.flush();
    delay(100);
    
    // Start mDNS responder once, after WiFi is configured
    if (MDNS.begin("sensything")) {
        Serial.println(String(EMOJI_SUCCESS) + " mDNS: sensything.local");
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("ws", "tcp", 81);
    } else {
        Serial.println(String(EMOJI_WARNING) + " mDNS initialization skipped");
    }
    Serial.flush();
    
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
    
    // Setup captive portal DNS redirect
    if (!pDNSServer) {
        pDNSServer = new DNSServer();
    }
    // Redirect all DNS requests to AP IP address
    pDNSServer->setErrorReplyCode(DNSReplyCode::NoError);
    pDNSServer->start(53, "*", apIP);
    captivePortalActive = true;
    Serial.println(String(EMOJI_SUCCESS) + " Captive portal DNS started on port 53");
    
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
        Serial.flush();
        
        // Save credentials to NVS for auto-connect on next boot
        saveCredentials(ssid, password);
        
        delay(100);
        
        // Start/restart mDNS
        if (MDNS.begin("sensything")) {
            Serial.println(String(EMOJI_SUCCESS) + " mDNS: sensything.local");
            MDNS.addService("http", "tcp", 80);
            MDNS.addService("ws", "tcp", 81);
        } else {
            Serial.println(String(EMOJI_WARNING) + " mDNS initialization skipped");
        }
        Serial.flush();
        
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
    
    // Handle DNS requests for captive portal
    if (captivePortalActive && pDNSServer) {
        pDNSServer->processNextRequest();
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
                
                // Determine board type string
                String boardType = "UNKNOWN";
                String sampleRateStr = "[100]";  // Default fallback
                
                if (boardConfig.boardType == BOARD_TYPE_OX) {
                    boardType = "OX";
                    // OX runs at ~125Hz (8ms), offer range from 50-125Hz
                    sampleRateStr = "[8,10,12,16,20]";  // ms periods
                } else if (boardConfig.boardType == BOARD_TYPE_CAP) {
                    boardType = "CAP";
                    // CAP typically 10Hz, offer 2-20Hz range
                    sampleRateStr = "[50,100,200,500]";  // ms periods
                }
                
                // Send enhanced init message with board detection info
                String welcome = "{\"type\":\"init\","
                               "\"board\":\"" + boardType + "\","
                               "\"boardName\":\"" + String(boardConfig.boardName) + "\","
                               "\"channels\":" + String(boardConfig.channelCount) + ","
                               "\"sampleRates\":" + sampleRateStr + ","
                               "\"sampleInterval\":" + String(boardConfig.minSampleInterval) + "}";
                
                pWebSocket->sendTXT(num, welcome);
                
                Serial.println(String(EMOJI_INFO) + " Sent board info: " + boardType);
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
    
    // ========================================================================
    // Captive Portal Detection Endpoints
    // ========================================================================
    
    // Apple Captive Portal detection - must return 200 OK with NO redirect
    pWebServer->on("/hotspot-detect.html", [this]() {
        pWebServer->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        pWebServer->sendHeader("Pragma", "no-cache");
        pWebServer->sendHeader("Expires", "0");
        pWebServer->send(200, "text/plain", "Success");
    });
    
    // Apple Captive Portal alternative endpoint
    pWebServer->on("/Library/Test/Success.html", [this]() {
        pWebServer->send(200, "text/plain", "Success");
    });
    
    // Windows NCSI (Network Connectivity Status Indicator) endpoint
    pWebServer->on("/ncsi.txt", [this]() {
        pWebServer->send(200, "text/plain", "Microsoft NCSI");
    });
    
    // Android CaptivePortalCheck endpoint
    pWebServer->on("/generate_204", [this]() {
        pWebServer->send(204);  // 204 No Content
    });
    
    // Generic captive.apple.com detection
    pWebServer->on("/captive.apple.com", [this]() {
        pWebServer->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        pWebServer->send(200, "text/plain", "Success");
    });
    
    // All other paths redirect to dashboard for captive portal
    // 404 handler
    pWebServer->onNotFound([this]() {
        // For captive portal, redirect to dashboard instead of 404
        String redirectURL = "http://";
        redirectURL += WiFi.softAPIP().toString();
        redirectURL += "/dashboard";
        
        pWebServer->sendHeader("Location", redirectURL);
        pWebServer->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        pWebServer->send(302, "text/plain", "");
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
            background: #000000;
            color: #333;
        }
        .container {
            max-width: 1600px;
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
        
        /* ===== SIDE-BY-SIDE LAYOUT FOR DESKTOP ===== */
        .content-wrapper {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
            align-items: start;
        }
        
        .chart-section {
            display: flex;
            flex-direction: column;
            gap: 12px;
        }
        
        .right-panel {
            display: flex;
            flex-direction: column;
            gap: 12px;
            overflow-y: auto;
            max-height: 600px;
            padding-right: 8px;
        }
        
        /* Scrollbar styling for right panel */
        .right-panel::-webkit-scrollbar {
            width: 6px;
        }
        
        .right-panel::-webkit-scrollbar-track {
            background: #f1f1f1;
            border-radius: 10px;
        }
        
        .right-panel::-webkit-scrollbar-thumb {
            background: #ccc;
            border-radius: 10px;
        }
        
        .right-panel::-webkit-scrollbar-thumb:hover {
            background: #999;
        }
        
        /* Responsive: Stack on smaller screens */
        @media (max-width: 1200px) {
            .content-wrapper {
                grid-template-columns: 1fr;
            }
            
            .chart-container {
                height: 400px;
            }
            
            .right-panel {
                max-height: none;
            }
        }
        
        /* ===== CHANNEL VALUES GRID ===== */
        .info-grid {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 12px;
            margin-bottom: 15px;
        }
        
        /* For right panel, show cards in 2x2 grid */
        .right-panel .info-grid {
            grid-template-columns: repeat(2, 1fr);
        }
        
        @media (max-width: 1400px) {
            .info-grid {
                grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
            }
        }
        .info-card {
            padding: 12px;
            background: linear-gradient(135deg, #f9f9f9 0%, #f0f0f0 100%);
            border-radius: 8px;
            border-left: 4px solid #667eea;
            border-right: 1px solid #e0e0e0;
            text-align: center;
            box-shadow: 0 2px 4px rgba(0,0,0,0.05);
            transition: all 0.2s ease;
        }
        .info-card:hover {
            box-shadow: 0 4px 12px rgba(102, 126, 234, 0.15);
            transform: translateY(-2px);
        }
        .info-label {
            font-size: 11px;
            color: #888;
            margin-bottom: 4px;
            text-transform: uppercase;
            font-weight: 600;
            letter-spacing: 0.5px;
        }
        .info-value {
            font-size: 24px;
            font-weight: bold;
            color: #667eea;
            line-height: 1.2;
        }
        .info-unit {
            font-size: 10px;
            color: #999;
            margin-top: 2px;
        }
        /* ===== CONFIGURATION PANEL (COLLAPSIBLE) ===== */
        .config-panel {
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            background: #f9f9f9;
            margin-bottom: 0;
            overflow: hidden;
            box-shadow: 0 2px 8px rgba(0,0,0,0.05);
        }
        .config-header {
            padding: 12px;
            background: linear-gradient(135deg, #f0f0f0 0%, #e8e8e8 100%);
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
        
        /* ===== OX BOARD SPECIFIC STYLES ===== */
        
        /* OX Vitals Container */
        .ox-vitals-container {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(160px, 1fr));
            gap: 16px;
            margin-bottom: 20px;
        }
        
        /* OX Vital Card - Large and Prominent */
        .ox-vital-card {
            padding: 20px;
            background: white;
            border: 2px solid #e0e0e0;
            border-radius: 12px;
            text-align: center;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            min-height: 180px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.08);
            transition: all 0.3s;
        }
        
        .ox-vital-card:hover {
            box-shadow: 0 4px 12px rgba(0,0,0,0.12);
        }
        
        .ox-vital-value {
            font-size: 48px;
            font-weight: bold;
            color: #333;
            line-height: 1;
            margin: 12px 0;
        }
        
        .ox-vital-unit {
            font-size: 18px;
            color: #999;
            font-weight: 500;
        }
        
        .ox-vital-status {
            padding: 6px 12px;
            border-radius: 20px;
            font-weight: 600;
            display: inline-block;
            font-size: 12px;
            margin-top: 8px;
        }
        
        .ox-status-normal {
            background: #d4edda;
            color: #155724;
        }
        
        .ox-status-warning {
            background: #fff3cd;
            color: #856404;
        }
        
        .ox-status-critical {
            background: #f8d7da;
            color: #721c24;
        }
        
        /* OX PPG Section */
        .ox-ppg-section {
            margin-bottom: 20px;
            padding: 16px;
            background: #f9f9f9;
            border-radius: 8px;
            border: 1px solid #e0e0e0;
        }
        
        .ox-ppg-chart {
            height: 250px !important;
        }
        
        /* CAP Grid Adjustment */
        .cap-grid {
            grid-template-columns: repeat(4, 1fr);
        }
        
        @media (max-width: 1200px) {
            .cap-grid {
                grid-template-columns: repeat(2, 1fr);
            }
        }
        
        @media (max-width: 768px) {
            .cap-grid {
                grid-template-columns: 1fr;
            }
            
            .ox-vitals-container {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <!-- HEADER -->
        <div class="header">
            <div style="display: flex; align-items: center; gap: 12px;">
                <svg width="120" height="30" viewBox="0 0 661.01 135.5" xmlns="http://www.w3.org/2000/svg">
                    <style>
                        .cls-1 { fill: #e3031a; }
                        .cls-2 { fill: #404040; }
                    </style>
                    <path class="cls-1" d="M39.21,38.16c3.56-8.54,7.2-15.34,10.92-20.38,3.71-5.04,7.82-8.66,12.33-10.87,4.5-2.21,9.73-3.31,15.67-3.31,2.97,0,5.35.24,7.13.72v18c-.4-.09-.92-.17-1.56-.22-.65-.05-1.26-.07-1.86-.07-4.85,0-8.98,1.58-12.4,4.75s-6.76,8.59-10.02,16.27l-11.44,27.22c-3.56,8.45-7.18,15.17-10.84,20.16-3.66,4.99-7.72,8.57-12.18,10.73-4.46,2.16-9.75,3.24-15.89,3.24-1.68,0-3.32-.1-4.9-.29-1.58-.19-2.97-.43-4.16-.72v-18c1.39.38,3.12.58,5.2.58,5.05,0,9.3-1.56,12.77-4.68,3.46-3.12,6.73-8.42,9.8-15.91l11.44-27.22Z"/>
                    <path class="cls-2" d="M124.45,105.98c-8.91,0-16.63-1.51-23.17-4.54-6.53-3.02-11.56-7.32-15.07-12.89-3.52-5.57-5.27-12.19-5.27-19.87,0-6.82,1.51-12.84,4.53-18.07,3.02-5.23,7.18-9.36,12.48-12.38,5.29-3.02,11.36-4.54,18.19-4.54,6.14,0,11.63,1.35,16.48,4.03,4.85,2.69,8.66,6.39,11.44,11.09,2.77,4.71,4.16,10.03,4.16,15.98,0,1.25-.03,2.66-.07,4.25-.05,1.58-.17,2.71-.37,3.38l-46.04,6.77c3.27,6.82,10.99,10.22,23.17,10.22,2.87,0,5.94-.26,9.21-.79,3.27-.53,6.04-1.22,8.32-2.09v16.56c-1.58.77-4.18,1.44-7.8,2.02-3.62.58-7.01.86-10.17.86ZM115.99,50.26c-4.36,0-7.92,1.27-10.69,3.82-2.77,2.54-4.46,5.98-5.05,10.3l28.51-4.32c-.59-2.97-2-5.35-4.23-7.13-2.23-1.78-5.08-2.66-8.54-2.66Z"/>
                    <path class="cls-2" d="M159.65,36h18.42v5.62c1.78-2.4,4.21-4.32,7.28-5.76,3.07-1.44,6.48-2.16,10.25-2.16,6.83,0,12.23,2.09,16.19,6.26,3.96,4.18,5.94,9.82,5.94,16.92v46.8h-19.31v-42.19c0-6.81-3.17-10.22-9.5-10.22-2.18,0-4.23.5-6.16,1.51s-3.19,2.28-3.79,3.82v47.09h-19.31V36Z"/>
                    <path class="cls-2" d="M232.86,86.98c2.47.58,5.02,1.03,7.65,1.37,2.62.34,5.12.5,7.5.5,3.46,0,5.94-.43,7.43-1.3s2.23-2.02,2.23-3.46c0-1.15-.5-2.25-1.49-3.31-.99-1.05-2.87-2.35-5.64-3.89l-7.87-4.46c-3.76-2.11-6.68-4.75-8.76-7.92s-3.12-6.53-3.12-10.08c0-5.95,2.52-10.75,7.57-14.4,5.05-3.65,12.03-5.47,20.94-5.47,2.67,0,5.35.12,8.02.36,2.67.24,4.95.6,6.83,1.08v15.84c-2.67-.48-5.05-.81-7.13-1.01-2.08-.19-4.16-.29-6.24-.29-3.37,0-5.74.38-7.13,1.15-1.39.77-2.08,1.73-2.08,2.88,0,.77.32,1.54.96,2.3.64.77,1.91,1.68,3.79,2.74l7.87,4.46c9.21,5.18,13.81,11.95,13.81,20.3,0,6.62-2.5,11.74-7.5,15.34-5,3.6-12.05,5.4-21.16,5.4-3.07,0-6.01-.14-8.84-.43-2.82-.29-5.37-.62-7.65-1.01v-16.7Z"/>
                    <path class="cls-2" d="M313.95,133.92h-21.39l21.39-40.18-31.48-57.74h22.72l19.16,38.02,18.71-38.02h21.24l-50.34,97.92Z"/>
                    <path class="cls-2" d="M394.59,103.68h-19.31v-51.55h-9.65v-16.13h9.65v-14.26l18.12-9.07h1.19v23.33h10.84v16.13h-10.84v51.55Z"/>
                    <path class="cls-2" d="M415.23,0h19.31v40.46c1.78-2.11,4.13-3.77,7.05-4.97,2.92-1.2,6.11-1.8,9.58-1.8,6.83,0,12.23,2.09,16.19,6.26,3.96,4.18,5.94,9.82,5.94,16.92v46.8h-19.31v-42.19c0-6.81-3.17-10.22-9.5-10.22-2.18,0-4.23.5-6.16,1.51s-3.19,2.28-3.79,3.82v47.09h-19.31V0Z"/>
                    <path class="cls-2" d="M490.45,20.59c-2.23-2.11-3.34-4.8-3.34-8.06s1.11-6.07,3.34-8.14c2.23-2.06,5.07-3.1,8.54-3.1s6.31,1.03,8.54,3.1c2.23,2.07,3.34,4.78,3.34,8.14s-1.11,5.95-3.34,8.06c-2.23,2.11-5.07,3.17-8.54,3.17s-6.31-1.06-8.54-3.17ZM489.33,36h19.31v67.68h-19.31V36Z"/>
                    <path class="cls-2" d="M524.97,36h18.42v5.62c1.78-2.4,4.21-4.32,7.28-5.76,3.07-1.44,6.48-2.16,10.25-2.16,6.83,0,12.23,2.09,16.19,6.26,3.96,4.18,5.94,9.82,5.94,16.92v46.8h-19.31v-42.19c0-6.81-3.17-10.22-9.5-10.22-2.18,0-4.23.5-6.16,1.51s-3.19,2.28-3.79,3.82v47.09h-19.31V36Z"/>
                    <path class="cls-2" d="M661.01,103.97c0,10.17-3.14,17.98-9.43,23.4-6.29,5.42-15.32,8.14-27.1,8.14-1.78,0-3.81-.12-6.09-.36-2.28-.24-4.41-.53-6.39-.86-1.98-.34-3.47-.7-4.46-1.08v-16.99c2.57.67,5.25,1.2,8.02,1.58,2.77.38,5.15.58,7.13.58,12.67,0,19.01-4.46,19.01-13.39v-.72c-2.97.67-5.74,1.01-8.32,1.01-11.68,0-20.94-3.14-27.77-9.43-6.83-6.29-10.25-14.81-10.25-25.56,0-7.29,1.83-13.63,5.5-19.01,3.66-5.38,8.84-9.53,15.52-12.46,6.68-2.93,14.53-4.39,23.54-4.39,3.46,0,7.08.17,10.84.5,3.76.34,7.18.79,10.25,1.37v67.68ZM637.4,51.55c-6.73,0-12.13,1.71-16.19,5.11-4.06,3.41-6.09,7.94-6.09,13.61s1.73,9.94,5.2,13.1c3.46,3.17,8.17,4.75,14.11,4.75,2.57,0,5-.38,7.28-1.15v-35.14c-1.29-.19-2.72-.29-4.31-.29Z"/>
                </svg>
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
        
        <!-- SIDE-BY-SIDE LAYOUT: CHART + CONFIGURATION -->
        <div class="content-wrapper">
            <!-- LEFT PANEL: CHART -->
            <div class="chart-section">
                <div class="chart-container">
                    <canvas id="chartCanvas"></canvas>
                </div>
            </div>
            
            <!-- RIGHT PANEL: VALUES + CONFIG -->
            <div class="right-panel">
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
                    <button class="btn btn-sm" onclick="scanNetworks(event)">🔍 Scan</button>
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
            </div>
        </div>
        
        <div class="footer">
            <p>Protocentral Electronics © 2025 | WebSocket: <span id="wsUrl"></span></p>
        </div>
    </div>

    <!-- OX BOARD DASHBOARD TEMPLATE -->
    <template id="oxDashboard">
        <div id="oxVitalsSection">
            <div class="ox-vitals-container">
                <div class="ox-vital-card">
                    <div class="ox-vital-label">Heart Rate</div>
                    <div class="ox-vital-value" id="oxHR">-- </div>
                    <div class="ox-vital-unit">BPM</div>
                    <div class="ox-vital-status ox-status-normal" id="oxHRStatus">✓ Normal</div>
                </div>
                
                <div class="ox-vital-card">
                    <div class="ox-vital-label">SpO₂</div>
                    <div class="ox-vital-value" id="oxSPO2">-- </div>
                    <div class="ox-vital-unit">%</div>
                    <div class="ox-vital-status ox-status-normal" id="oxSPO2Status">✓ Normal</div>
                </div>
            </div>
            
            <div class="ox-ppg-section">
                <h3 style="margin: 0 0 12px 0; color: #333;">📈 PPG Waveforms</h3>
                <canvas id="oxChartCanvas" style="width: 100%; height: 300px;"></canvas>
            </div>
        </div>
    </template>

    <!-- CAP BOARD DASHBOARD TEMPLATE -->
    <template id="capDashboard">
        <div id="capChannelsSection">
            <h3 style="margin: 0 0 12px 0; color: #333;">📊 Channel Data</h3>
            <div class="cap-grid info-grid" id="capChannelValues">
                <!-- Populated by JavaScript -->
            </div>
        </div>
    </template>

    <script>
        let canvas = null;
        let ctx = null;
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
        
        function initCanvasAfterDOMReady() {
            canvas = document.getElementById('chartCanvas');
            if (!canvas) {
                console.error('Canvas element not found!');
                return false;
            }
            ctx = canvas.getContext('2d');
            resizeCanvas();
            return true;
        }
        
        function resizeCanvas() {
            if (!canvas) return;
            canvas.width = canvas.clientWidth;
            canvas.height = canvas.clientHeight;
            drawChart();
        }
        
        window.addEventListener('resize', resizeCanvas);
        
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
                
                // Handle board initialization message
                if (data.type === 'init') {
                    console.log('Board detected:', data.board);
                    handleBoardInit(data);
                    return;
                }
                
                // Legacy 'info' message (backwards compatibility)
                if (data.type === 'info') {
                    initializeChart(data.channels);
                    return;
                }
                
                // Handle measurement data
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
        
        // Board detection and routing
        let currentBoard = null;
        
        function handleBoardInit(initData) {
            currentBoard = initData.board;
            console.log('Initializing dashboard for:', currentBoard);
            console.log('Init data:', initData);
            
            if (currentBoard === 'OX') {
                initializeOXDashboard(initData);
            } else if (currentBoard === 'CAP') {
                initializeCAPDashboard(initData);
            } else {
                console.warn('Unknown board type:', currentBoard);
                initializeChart(initData.channels || 4);
            }
        }
        
        function initializeOXDashboard(config) {
            console.log('Setting up OX Dashboard');
            
            // Hide generic chart and channel values
            const chartContainer = document.querySelector('.chart-container');
            const channelValues = document.getElementById('channelValues');
            
            if (chartContainer) chartContainer.style.display = 'none';
            if (channelValues) channelValues.style.display = 'none';
            
            // Show the template by moving it into the main area
            const oxTemplate = document.getElementById('oxDashboard');
            if (oxTemplate) {
                // Clone the template content
                const oxDom = oxTemplate.content.cloneNode(true);
                
                // Insert right after the toolbar
                const toolbar = document.querySelector('.toolbar');
                if (toolbar && toolbar.parentNode) {
                    toolbar.parentNode.insertBefore(oxDom, toolbar.nextSibling);
                }
            }
            
            // Initialize chart canvas for OX (PPG waveforms)
            const oxCanvas = document.getElementById('oxChartCanvas');
            if (oxCanvas) {
                canvas = oxCanvas;  // Update global canvas reference
                ctx = canvas.getContext('2d');
                maxDataPoints = 200;  // Set window size to 200 samples for OX
                document.getElementById('windowValue').textContent = '200';
                resizeCanvas();
                drawChart();
            }
            
            console.log('OX Dashboard ready');
        }
        
        function initializeCAPDashboard(config) {
            console.log('Setting up CAP Dashboard');
            
            // Set window size to 500 samples for CAP
            maxDataPoints = 500;
            document.getElementById('windowValue').textContent = '500';
            
            // Show generic chart and channel values (already visible)
            const chartContainer = document.querySelector('.chart-container');
            const channelValues = document.getElementById('channelValues');
            
            if (chartContainer) chartContainer.style.display = 'block';
            if (channelValues) channelValues.style.display = 'block';
            
            // Initialize channel grid
            const grid = channelValues;
            if (grid) {
                grid.classList.add('cap-grid');
                grid.innerHTML = '';
                for (let i = 0; i < (config.channels || 4); i++) {
                    grid.innerHTML += `
                        <div class="info-card">
                            <div class="info-label">Channel ${i}</div>
                            <div class="info-value" id="ch${i}">--</div>
                        </div>
                    `;
                }
            }
            
            // Initialize chart
            const chartCanvas = document.getElementById('chartCanvas');
            if (chartCanvas) {
                canvas = chartCanvas;  // Update global canvas reference
                ctx = canvas.getContext('2d');
                resizeCanvas();
                drawChart();
            }
            
            console.log('CAP Dashboard ready');
        }
        
        function updateChart(data) {
            // If OX board, use different update logic
            if (currentBoard === 'OX') {
                updateOXChart(data);
                return;
            }
            
            // Default CAP chart update
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
        
        function updateOXChart(data) {
            // OX board: ch[0]=IR, ch[1]=RED, ch[2]=SpO2, ch[3]=HR
            if (data.ch && data.ch.length >= 4) {
                // Update PPG waveforms (IR and RED)
                for (let i = 0; i < 2; i++) {
                    const value = data.ch[i];
                    if (value !== null) {
                        if (!channelData[i]) channelData[i] = [];
                        channelData[i].push({
                            x: data.cnt,
                            y: value
                        });
                        
                        if (channelData[i].length > maxDataPoints) {
                            channelData[i].shift();
                        }
                    }
                }
                
                // Update vital signs (SpO2 at ch[2], HR at ch[3])
                const spo2 = data.ch[2];
                const hr = data.ch[3];
                
                if (spo2 !== null && spo2 !== undefined) {
                    const spo2El = document.getElementById('oxSPO2');
                    if (spo2El) {
                        spo2El.textContent = Math.round(spo2);
                        updateVitalStatus('SPO2', spo2);
                    }
                }
                
                if (hr !== null && hr !== undefined) {
                    const hrEl = document.getElementById('oxHR');
                    if (hrEl) {
                        hrEl.textContent = Math.round(hr);
                        updateVitalStatus('HR', hr);
                    }
                }
            }
            
            drawChart();
        }
        
        function updateVitalStatus(vital, value) {
            if (vital === 'HR') {
                const statusEl = document.getElementById('oxHRStatus');
                if (statusEl) {
                    if (value < 60 || value > 100) {
                        statusEl.textContent = '⚠ Check HR';
                        statusEl.className = 'ox-vital-status ox-status-warning';
                    } else {
                        statusEl.textContent = '✓ Normal';
                        statusEl.className = 'ox-vital-status ox-status-normal';
                    }
                }
            } else if (vital === 'SPO2') {
                const statusEl = document.getElementById('oxSPO2Status');
                if (statusEl) {
                    if (value < 95) {
                        statusEl.textContent = '⚠ Low SpO₂';
                        statusEl.className = 'ox-vital-status ox-status-critical';
                    } else if (value < 98) {
                        statusEl.textContent = '⚠ Check';
                        statusEl.className = 'ox-vital-status ox-status-warning';
                    } else {
                        statusEl.textContent = '✓ Normal';
                        statusEl.className = 'ox-vital-status ox-status-normal';
                    }
                }
            }
        }
        
        
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
        
        function scanNetworks(event) {
            console.log('Scan started');
            
            // Get the button - use event.currentTarget if available, otherwise find it
            let btn = event ? event.currentTarget : document.querySelector('button[onclick*="scanNetworks"]');
            if (!btn) {
                btn = document.querySelectorAll('.btn-sm')[0]; // Fallback
            }
            
            if (btn) {
                btn.disabled = true;
                btn.textContent = '🔄 Scanning...';
            }
            
            console.log('Fetching /api/wifi/scan');
            
            fetch('/api/wifi/scan')
                .then(r => {
                    console.log('Response status: ' + r.status);
                    if (!r.ok) {
                        throw new Error('HTTP error, status: ' + r.status);
                    }
                    return r.json();
                })
                .then(data => {
                    console.log('Scan response: ' + JSON.stringify(data).substring(0, 100));
                    const select = document.getElementById('wifiSSID');
                    if (!select) {
                        console.error('wifiSSID select not found');
                        return;
                    }
                    
                    select.innerHTML = '<option value="">-- Select Network --</option>';
                    
                    if (data.networks && data.networks.length > 0) {
                        console.log('Adding ' + data.networks.length + ' networks');
                        data.networks.forEach(net => {
                            const option = document.createElement('option');
                            option.value = net.ssid;
                            option.textContent = net.ssid + ' (' + net.rssi + ' dBm) ' + (net.secure ? '🔒' : '');
                            option.dataset.secure = net.secure;
                            select.appendChild(option);
                        });
                    } else {
                        console.log('No networks found');
                    }
                    
                    if (btn) {
                        btn.disabled = false;
                        btn.textContent = '🔍 Scan Networks';
                    }
                })
                .catch(e => {
                    console.error('Scan error: ' + e.message);
                    console.error('Stack: ' + e.stack);
                    if (btn) {
                        btn.disabled = false;
                        btn.textContent = '🔍 Scan Networks';
                    }
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
        
        function autoScanNetworks() {
            // Auto-scan on page load - create a mock event for the button
            const mockEvent = {
                currentTarget: document.querySelector('button[onclick="scanNetworks(event)"]') || {
                    disabled: false,
                    textContent: '🔍 Scan',
                    setAttribute: function() {},
                    removeAttribute: function() {}
                }
            };
            scanNetworks(mockEvent);
        }
        
        // Update system info every 5 seconds
        setInterval(updateSystemInfo, 5000);
        updateSystemInfo();
        
        // Auto-scan networks on load
        setTimeout(autoScanNetworks, 1000);
    </script>
</body>
</html>
)rawliteral";
    
    return html;
}
