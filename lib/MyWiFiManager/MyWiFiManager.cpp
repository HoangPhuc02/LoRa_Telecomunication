#include <MyWiFiManager.h>
#include <WiFi.h>



    MyWiFiManager::MyWiFiManager( const char* ssid, const char* password)
        :  _ssid(ssid), _password(password) { 
        dataToSend[0] = 0x00;
        wifiState = 0;
        connectionAttemptTime = 0;
    }

    void MyWiFiManager::begin() {
        WiFi.mode(WIFI_STA); // Set ESP32 to station mode
        // Register event handler
        WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
            WiFiEvent(event);
        });
        connectToWiFi();
    }

    bool MyWiFiManager::isConnected() {
        return WiFi.status() == WL_CONNECTED;
    }


    void MyWiFiManager::settingIPAddress()
    {
        IPAddress localIP;
        //IPAddress localIP(192, 168, 1, 200); // hardcoded

        // Set your Gateway IP address
        IPAddress localGateway;
        //IPAddress localGateway(192, 168, 1, 1); //hardcoded
        IPAddress subnet(255, 255, 0, 0);
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
        WiFi.begin(_ssid, _password);
        //char dataToSend[1] = { 0xF0 }; // Create an array with one element
        // Timeout if connection fails
        unsigned long startAttemptTime = millis();
        if(connectionAttemptTime == MAX_CONNECT_TIME)
        {
            ESP.restart();
        }
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
            delay(500);    
            Serial.print(".");
        }
        connectionAttemptTime += 1;
        
    }

 


