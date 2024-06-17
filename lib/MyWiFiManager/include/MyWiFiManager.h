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


typedef void (*UARTDataSendFunc)(char*, uint32_t); 

class MyWiFiManager {
public:
    MyWiFiManager(const char* ssid, const char* password );

    void begin() ;
    void settingIPAddress();
    bool isConnected() ;
    void setOnDisconnectCallback(UARTDataSendFunc callback);

    char dataToSend[1];
    uint8_t connectionAttemptTime;
    uint8_t wifiState;
private:
    UARTDataSendFunc onDisconnectCallback;
    const char* _ssid;
    const char* _password;
    // AsyncWebServer server(80);

    // // Search for parameter in HTTP POST request
    // const char* PARAM_INPUT_1 = "ssid";
    // const char* PARAM_INPUT_2 = "pass";
    // const char* PARAM_INPUT_3 = "ip";
    // const char* PARAM_INPUT_4 = "gateway";

    // //Variables to save values from HTML form
    // String ssid;
    // String pass;
    // String ip;
    // String gateway;

    void WiFiEvent(WiFiEvent_t event) ;

    void connectToWiFi() ;

};

#endif
