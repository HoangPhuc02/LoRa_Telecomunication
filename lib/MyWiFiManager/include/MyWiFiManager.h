#ifndef __WIFI_MANAGER_LIB_H
#define __WIFI_MANAGER_LIB_H
/** UART config*/
/*Pin */
#include <ProjectConfig.h>
#include <WiFi.h>
#include <WiFiManager.h>
// #include <FirebaseClient.h>

#include <FirebaseESP32.h>
#include <HTTPClient.h>


typedef void (*UARTDataSendFunc)(char*, uint32_t); 

class MyWiFiManager {
public:
    MyWiFiManager(const char* ssid, const char* password );

    void begin() ;

    bool isConnected() ;
    void setOnDisconnectCallback(UARTDataSendFunc callback);

    char dataToSend[1];
    uint8_t connectionAttemptTime;
    uint8_t wifiState;
private:
    UARTDataSendFunc onDisconnectCallback;
    const char* _ssid;
    const char* _password;

    void WiFiEvent(WiFiEvent_t event) ;

    void connectToWiFi() ;

};

#endif
