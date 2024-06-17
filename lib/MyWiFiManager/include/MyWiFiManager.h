#ifndef __WIFI_MANAGER_LIB_H
#define __WIFI_MANAGER_LIB_H
/** UART config*/
/*Pin */
#include <ProjectConfig.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
// #include <FirebaseClient.h>

#include <FirebaseESP32.h>
#include <HTTPClient.h>
#include <SPIFFS_FB.h>

typedef void (*UARTDataSendFunc)(char*, uint32_t); 

class MyWiFiManager {
public:
    MyWiFiManager( LittleFSManager& litterfsManager);

    void begin() ;
    bool initWifi();
    void settingIPAddress();
    void startWiFiManager();
    bool isConnected() ;
    void setOnDisconnectCallback(UARTDataSendFunc callback);

    char dataToSend[1];
    uint8_t connectionAttemptTime;
    uint8_t wifiState;
    LittleFSManager &_littlefsManager;
    String _ssid, _pass, _ip, _gateway;
    
    
    IPAddress localIP, localGateway;

private:
    UARTDataSendFunc onDisconnectCallback;
    // const char* _ssid;
    // const char* _password;
    AsyncWebServer server;


    void WiFiEvent(WiFiEvent_t event) ;

    void connectToWiFi() ;

};

#endif
