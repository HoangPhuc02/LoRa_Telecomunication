#include <WiFiManager.h>
#include <WiFi.h>



    WiFiManager::WiFiManager(const char* ssid, const char* password)
        : _ssid(ssid), _password(password) { 
        
    }

    void WiFiManager::begin() {
        WiFi.mode(WIFI_STA); // Set ESP32 to station mode
        // Register event handler
        WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
            WiFiEvent(event);
        });
        connectToWiFi();
    }

    bool WiFiManager::isConnected() {
        return WiFi.status() == WL_CONNECTED;
    }



    void WiFiManager::WiFiEvent(WiFiEvent_t event) {
        switch (event) {
            case SYSTEM_EVENT_STA_START:
                Serial.println("ESP32 station started");
                break;
            case SYSTEM_EVENT_STA_GOT_IP:
                Serial.println("Got IP address: " + WiFi.localIP().toString());
                break;
            case SYSTEM_EVENT_STA_DISCONNECTED:
                Serial.println("Disconnected from AP. Reconnecting...");
                WiFi.disconnect(); // Ensure disconnection before reconnecting
                connectToWiFi(); // Attempt to reconnect
                break;
        }
    }

    void WiFiManager::connectToWiFi() {
        Serial.print("Connecting to ");
        Serial.println(_ssid);
        WiFi.begin(_ssid, _password);

        // Timeout if connection fails
        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
            delay(500);
            Serial.print(".");
        }

        // if (WiFi.status() != WL_CONNECTED) {
        //     Serial.println("\nFailed to connect. Starting AP...");
        //     startAPMode();
        // }
    }

 


