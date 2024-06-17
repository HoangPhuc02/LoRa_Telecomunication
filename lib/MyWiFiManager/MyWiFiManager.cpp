#include <MyWiFiManager.h>
#include <WiFi.h>



MyWiFiManager::MyWiFiManager( LittleFSManager& litterfsManager)
    :  _littlefsManager(litterfsManager),server(80){ 
    dataToSend[0] = 0x00;
    
    wifiState = 0;
    connectionAttemptTime = 0;

}
bool MyWiFiManager::initWifi()
{
    IPAddress subnet(255, 255, 0, 0);
    if(_ssid=="" || _ip==""){
    Serial.println("Undefined SSID or IP address.");
    return false;
    }

    WiFi.mode(WIFI_STA);
    //localIP.fromString(_ip.c_str());
    //localGateway.fromString(_gateway.c_str());

    
    if (!WiFi.config(localIP, localGateway, subnet)){
      Serial.println("STA Failed to configure");
      return false;
    }
    WiFi.begin(_ssid.c_str(), _pass.c_str());
    Serial.println("Connecting to WiFi...");

    unsigned long currentMillis = millis();
    unsigned long previousMillis = currentMillis;

    while(WiFi.status() != WL_CONNECTED) {
      currentMillis = millis();
      if (currentMillis - previousMillis >= 10000) {
        Serial.println("Failed to connect.");
        return false;
      }
    }
    Serial.println(WiFi.localIP());
    return true;
}
void MyWiFiManager::begin() {
    _ssid    = _littlefsManager.readFile(ssidPath);
    _pass    = _littlefsManager.readFile(passPath);
    _ip      = _littlefsManager.readFile(ipPath);
    _gateway = _littlefsManager.readFile(gatewayPath);
    Serial.println(_ssid);
    // Serial.println(_pass);
    // Serial.println(_ip);
    // Serial.println(_gateway);
    // Register event handler
    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
        WiFiEvent(event);
    });
    if(!initWifi()){
        startWiFiManager();
        
    }

}

bool MyWiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}
void MyWiFiManager::startWiFiManager()
{
            // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 
    
        // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/wifimanager.html", "text/html");
    });
        
    server.serveStatic("/", LittleFS, "/");
    
    server.on("/", HTTP_POST, [this](AsyncWebServerRequest *request) {
        int params = request->params();
        for(int i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isPost()){
            // HTTP POST ssid value
            if (p->name() == PARAM_INPUT_1) {
                _ssid = p->value().c_str();
                Serial.print("SSID set to: ");
                Serial.println(_ssid);
                // Write file to save value
                _littlefsManager.writeFile(ssidPath, (uint8_t*)_ssid.c_str(),_ssid.length());
            }
            // HTTP POST pass value
            if (p->name() == PARAM_INPUT_2) {
                _pass = p->value().c_str();
                Serial.print("Password set to: ");
                Serial.println(_pass);
                // Write file to save value
                _littlefsManager.writeFile(passPath, (uint8_t*)_pass.c_str(),_pass.length());
            }
            // HTTP POST ip value
            if (p->name() == PARAM_INPUT_3) {
                _ip = p->value().c_str();
                Serial.print("IP Address set to: ");
                Serial.println(_ip);
                // Write file to save value
                _littlefsManager.writeFile( ipPath, (uint8_t*)_ip.c_str(),_ip.length());
            }
            // HTTP POST gateway value
            if (p->name() == PARAM_INPUT_4) {
                _gateway = p->value().c_str();
                Serial.print("Gateway set to: ");
                Serial.println(_gateway);
                // Write file to save value
                _littlefsManager.writeFile( gatewayPath, (uint8_t*)_gateway.c_str(),_gateway.length());
            }
            //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
        }
        request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + _ip);
        delay(3000);
        ESP.restart();
    });
    server.begin();
    }
    

    
void MyWiFiManager::WiFiEvent(WiFiEvent_t event) {
    char dataToSend[1] = { 0xF1 };
    switch (event) {
        case SYSTEM_EVENT_STA_START:
            Serial.println("ESP32 station started");
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            if(wifiState != SYSTEM_EVENT_STA_GOT_IP)
            {
                // dataToSend[0] = WiFI_CONNECT_SUCCESS ;
                // if (onDisconnectCallback) { // Check if callback is set
                //     onDisconnectCallback(dataToSend, 1); // Call it if set
                // }
                wifiState = SYSTEM_EVENT_STA_GOT_IP;
            }

             Serial.println("Got IP address: " + WiFi.localIP().toString());
            connectionAttemptTime = 0;
            
            //onDisconnectCallback(dataToSend, 1); // Call it if set

             break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            Serial.println("Disconnected from AP. Reconnecting...");
            if(wifiState != SYSTEM_EVENT_STA_DISCONNECTED)
            {
                // dataToSend[0] = WiFI_CONNECT_LOSS ;
                
                // if (onDisconnectCallback) { // Check if callback is set
                //     onDisconnectCallback(dataToSend, 1); // Call it if set
                // }
                wifiState = SYSTEM_EVENT_STA_DISCONNECTED;
            }
            WiFi.disconnect(); // Ensure disconnection before reconnecting
            connectToWiFi(); // Attempt to reconnect
            break;
    }
}
void MyWiFiManager::setOnDisconnectCallback(UARTDataSendFunc callback) {
    onDisconnectCallback = callback;
}
    void MyWiFiManager::connectToWiFi() {
    //     Serial.print("Connecting to ");
    //     Serial.println(_ssid);
    //     WiFi.begin(_ssid, _password);
    //     char dataToSend[1] = { 0xF0 }; // Create an array with one element
    //     // Timeout if connection fails
    //     unsigned long startAttemptTime = millis();
    //     while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
    //         delay(500);
           
    //         if (onDisconnectCallback) { // Check if callback is set
    //             onDisconnectCallback(dataToSend, 1); // Call it if set
    //         }
    //         Serial.print(".");
    //     }
 
    //    if (WiFi.status() == WL_CONNECTED) {  // Call callback ONLY if connected
    //         dataToSend[0] = { 0xF1 }; 
    //         if (onDisconnectCallback) {
    //         onDisconnectCallback(dataToSend, 1);
    //         }
    //     }
        Serial.print("Connecting to ");
        Serial.println(_ssid);
        WiFi.begin(_ssid, _pass);
        //char dataToSend[1] = { 0xF0 }; // Create an array with one element
        // Timeout if connection fails
        unsigned long startAttemptTime = millis();
        if(connectionAttemptTime == MAX_CONNECT_TIME)
        {
            startWiFiManager();
        }
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
            delay(500);    
            Serial.print(".");
        }
        connectionAttemptTime += 1;
        
    }

 


