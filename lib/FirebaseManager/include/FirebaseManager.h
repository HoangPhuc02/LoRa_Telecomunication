#ifndef __FIREBASE_MANAGER_H
#define __FIREBASE_MANAGER_H
/** UART config*/
/*Pin */
#include <ProjectConfig.h>

#include <WiFiManager.h>
#include <SPIFFS_FB.h>

// #include <FirebaseClient.h>

#include <FirebaseESP32.h>
#include <HTTPClient.h>


typedef enum{
    SensorData = 0,
    UpdateStatusDone,
    UpdateStatusFail
}UploadDataType_t;
typedef struct {
    String data;
    String path;
    UploadDataType_t type;
}
QueueUploadData_t;

// File path on the spiffs
// const String& localFilePath = "/firmware.bin";

class FirebaseManager {
public:
    FirebaseManager(WiFiManager& wifiManager, SPIFFSManager& spiffsManager) ;
    SPIFFSManager& getSPIFFSManager() ;
    WiFiManager& getWiFiManager() ;

    void begin() ;
    void firebase_init();

    String getDownloadURLFromFirebase() ;
    void startMonitoringVariable() ;
    

    bool checkForFirmwareUpdate();
    bool setDownloadCompleteAndReadyToFlash(bool state);


    void updateSensorDataNode();
   // void parseJsonData();


    String parseSensorData(const String &data, uint8_t *sensor_number, uint16_t *sensor_data) ;
    void updateSensorValues(FirebaseJson &json, uint8_t *sensor_number, const uint16_t *sensor_data);
    //FirebaseJson createInitialJson(uint8_t sensor_number, const uint16_t *sensor_data);
    static void uploadDataSensor(UploadDataType_t type, const String &path, const char *data, FirebaseData& fbdo) ;
    void uploadDataSensorToFB(const String &path,uint8_t *sensor_number, uint16_t *sensor_data) ;
    //void uploadDataSensor(const String &data);
    bool getHeaderFrame(FirebaseData& fbdo);

    bool updateStringFB(FirebaseData& fbdo,const char *data,const String &path);


    char header[HEADER_SIZE] ;
    uint8_t x2i(char *s) ;
    String i2x(uint8_t num);

    QueueHandle_t firebaseDataQueue;
    SemaphoreHandle_t firebaseSemaphore;

    TaskHandle_t firebaseStreamTaskHandle ; 
    TaskHandle_t firebaseDownloadFWTaskHandle ; 
    TaskHandle_t firebaseUploadTaskHandle ; 
    
    FirebaseData fbdo;
    FirebaseData fbdo1;
    FirebaseData fbdo2;
    
    
private:
    WiFiManager&_wifiManager;
    SPIFFSManager& _spiffsManager;
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