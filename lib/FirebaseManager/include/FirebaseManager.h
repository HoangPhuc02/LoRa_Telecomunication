#ifndef __FIREBASE_MANAGER_H
#define __FIREBASE_MANAGER_H

/****************************************************************************************
 *   Include Header Files
 ****************************************************************************************/
#include <ProjectConfig.h> // Project-specific configuration
#include <MyWiFiManager.h> // Custom WiFi management library
#include <SPIFFS_FB.h>     // Library for SPIFFS with Firebase support
#include <FirebaseESP32.h> // Firebase library for ESP32
#include <HTTPClient.h>     // HTTP client for web requests

/****************************************************************************************
 *   Data Type Definitions
 ****************************************************************************************/
// Enum for Upload Data Types
typedef enum {
    SensorData = 0,
    UpdateStatusDone,
    UpdateStatusFail
} UploadDataType_t;

// Struct for Queued Upload Data
typedef struct {
    String data;     // Data to upload (string representation)
    String path;     // Firebase path where to store the data
    UploadDataType_t type; // Type of data being uploaded
} QueueUploadData_t;


/****************************************************************************************
 *   FirebaseManager Class Definition
 ****************************************************************************************/
class FirebaseManager {
public:
    /************************************************************************************
     *   Constructors and Getters
     ***********************************************************************************/
    // Constructor
    FirebaseManager(MyWiFiManager& wifiManager, LittleFSManager& spiffsManager);

    // Getters for WiFi and LittleFS managers
    LittleFSManager& getLittleFSManager();
    MyWiFiManager& getWiFiManager();

    /************************************************************************************
     *   Initialization and Setup
     ***********************************************************************************/
    // Initialize Firebase 
    void begin();
    void firebase_init();

    MyWiFiManager& _wifiManager;
    // Data queue and semaphore for Firebase operations
    QueueHandle_t firebaseDataQueue;
    SemaphoreHandle_t firebaseSemaphore;

    // Task handles for Firebase operations
    TaskHandle_t firebaseStreamTaskHandle;
    TaskHandle_t firebaseDownloadFWTaskHandle;
    TaskHandle_t firebaseUploadTaskHandle;

    FirebaseData fbdo;
    FirebaseData fbdo1;
    FirebaseData fbdo2;    

    // Header buffer for Firebase data
    char header[HEADER_SIZE];

    // Get Firebase download URL 
    String getDownloadURLFromFirebase();

    /************************************************************************************
     *   Firmware Update Functions
     ***********************************************************************************/
    // Check for available firmware updates
    bool checkForFirmwareUpdate();

    // Indicate download completion and readiness to flash
    bool setDownloadCompleteAndReadyToFlash(bool state);
    
    // Start monitoring variables on Firebase
    void startMonitoringVariable();

    /************************************************************************************
     *   Data Upload Functions
     ***********************************************************************************/
    // Upload sensor data to Firebase
    void uploadDataSensor(UploadDataType_t type, const String &path, const char *data,String &curTime , FirebaseData& fbdo);

    // Upload sensor data to Firebase by sensor number
    void uploadDataSensorToFB(const String &path, uint8_t *sensor_number, uint16_t *sensor_data);

    // Get header frame from Firebase 
    bool getHeaderFrame(FirebaseData& fbdo);

    // Get current time from NTP server
    String getCurrentTime();


    /************************************************************************************
     *   Firebase Setter Functions
     ***********************************************************************************/
    // Set a string value in Firebase
    bool setStringFB(FirebaseData& fbdo, const String& path, const String& value);

    // Set an integer value in Firebase
    bool setIntFB(FirebaseData& fbdo, const String& path, int value);

    // Set a boolean value in Firebase
    bool setBoolFB(FirebaseData& fbdo, const String& path, bool value);

    // Set a float value in Firebase
    bool setFloatFB(FirebaseData& fbdo, const String& path, float value);


private: 
    /************************************************************************************
     *   Private Members and Utility Functions
     ***********************************************************************************/
    // References to WiFi and LittleFS managers
    LittleFSManager& _spiffsManager;
    

    // Firebase configuration and authentication objects
    FirebaseConfig config;
    FirebaseAuth auth;


    bool downloadCompleteAndReadyToFlash = false; // Flag for download status

    // Task for handling Firebase stream
    static void streamTask(void* pvParameters);

    // Handle Firebase stream data
    void handleStreamData(FirebaseData& fbdo);

    // Task for handling firmware update
    static void updateFirmwareTask(void* pvParameters);

    // Handle upload data to Firebase
    void handleUploadData();

    // Static task for Firebase upload 
    static void firebaseUploadTask(void *pvParameters);


    


    // Utility functions for hex conversion
    uint8_t x2i(char *s); 
    String i2x(uint8_t num);
};

#endif // __FIREBASE_MANAGER_H
