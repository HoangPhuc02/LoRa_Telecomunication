#ifndef __FIREBASE_MANAGER_H
#define __FIREBASE_MANAGER_H
/** UART config*/
/*Pin */
#include <Arduino.h>
#include <WiFiManager.h>
#include <SPIFFS_FB.h>
#include <string.h>
#include <cstdlib>
#include <string>
// #include <FirebaseClient.h>

#include <FirebaseESP32.h>
#include <HTTPClient.h>

#include <ProjectConfig.h>




// File path on the spiffs
// const String& localFilePath = "/firmware.bin";

class FirebaseManager {
public:
    FirebaseManager(WiFiManager& wifiManager, SPIFFSManager& spiffsManager) ;

    void begin() ;

    String getDownloadURLFromFirebase() ;

    void startMonitoringVariable() ;

    void firebase_init();

    SPIFFSManager& getSPIFFSManager() ;

    WiFiManager& getWiFiManager() ;

    bool checkForFirmwareUpdate();
    bool setDownloadCompleteAndReadyToFlash(bool state);
    void updateFirmwareStatus(const char* status);

    void updateSensorDataNode();
   // void parseJsonData();
    String parseStringValue(FirebaseJson &json, const String &path, const String &targetType = "string") ;
    int parseIntValue(FirebaseJson &json, const String &path) ;
    void downloadJson(const String &path);

    String parseSensorData(const String &data, uint8_t *sensor_number, uint16_t *sensor_data) ;
    void updateSensorValues(FirebaseJson &json, uint8_t *sensor_number, const uint16_t *sensor_data);
    //FirebaseJson createInitialJson(uint8_t sensor_number, const uint16_t *sensor_data);

    static void uploadDataSensor(const char *data, FirebaseData& fbdo1) ;
    void uploadDataSensorToFB(const String &path,uint8_t *sensor_number, uint16_t *sensor_data) ;
    //void uploadDataSensor(const String &data);
    bool getHeaderFrame();

    

    char header[HEADER_SIZE] ;
    uint8_t x2i(char *s) ;
    String i2x(uint8_t num);

    QueueHandle_t firebaseDataQueue;
    TaskHandle_t firebaseUploadTaskHandle ; 
    
private:
    WiFiManager&_wifiManager;
    SPIFFSManager& _spiffsManager;
    FirebaseData fbdo;
    FirebaseData fbdo1;
    FirebaseData fbdo2;
    FirebaseConfig config;
    FirebaseAuth auth;
    // FirebaseJson json;

    bool downloadCompleteAndReadyToFlash = false;

    static void streamTask(void* pvParameters) ;

    void handleStreamData(FirebaseData& fbdo) ;

    // Task function to handle firmware update
    static void updateFirmwareTask(void* pvParameters) ;

    void handleUploadData();
    static void firebaseUploadTask(void *pvParameters);
};

#endif