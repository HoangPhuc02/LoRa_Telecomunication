#ifndef __WIFI_MANAGER_LIB_H
#define __WIFI_MANAGER_LIB_H
/** UART config*/
/*Pin */
#include <Arduino.h>
#include <WiFi.h>
#include <String.h>

// #include <FirebaseClient.h>

#include <FirebaseESP32.h>
#include <HTTPClient.h>
#include <ProjectConfig.h>












class WiFiManager {
public:
    WiFiManager(const char* ssid, const char* password );


    void begin() ;

    bool isConnected() ;

private:
    const char* _ssid;
    const char* _password;

    void WiFiEvent(WiFiEvent_t event) ;

    void connectToWiFi() ;

};

#endif
