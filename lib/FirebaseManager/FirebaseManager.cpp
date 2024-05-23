#include "FirebaseManager.h"

#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
                                                                                                                                                                                                                                                                                                                                                                                                                                                                    


/* Public function */
    FirebaseManager::FirebaseManager(WiFiManager& wifiManager, SPIFFSManager& spiffsManager) : _wifiManager(wifiManager), _spiffsManager(spiffsManager) {
        //fbdo.setBSSLBufferSize(1024, 1024); // Adjust buffer sizes if needed
    }

    void FirebaseManager::begin() {
        // Serial.println("begin success");
        firebase_init();

        firebaseDataQueue = xQueueCreate(5, sizeof(String));
        handleUploadData();
        // startMonitoringVariable();
    }

    String FirebaseManager::getDownloadURLFromFirebase() {
    if (Firebase.get(fbdo1, firebaseFilePath)) { 
        Serial.println(fbdo1.dataType());
        if (fbdo1.dataType() == "string") {
            Serial.println(fbdo1.stringData() );
            return fbdo1.stringData();
        } else {
            Serial.println("ERROR: Data at Firebase path is not a string.");
        }
    } else {
        // Check for specific error reasons
        if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK) {
            Serial.println("ERROR: Firebase get operation failed. Error reason: " + fbdo.errorReason());
        } else {
            Serial.println("ERROR: HTTP error code: " + String(fbdo.httpCode()));
        }
    }
    return "";
}

    void FirebaseManager::startMonitoringVariable() {
        Serial.println("startMonitoringVariable");
        xTaskCreatePinnedToCore(streamTask, "streamTask", 8192, this, 1, NULL, 1);
    }


void FirebaseManager::firebase_init()
{
     // Initialize Firebase
    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
    Firebase.reconnectNetwork(true);

    // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
    // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
    fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
    fbdo1.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

    Firebase.begin(&config, &auth);
    // Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
    Serial.println("Connect firebase success");
}


SPIFFSManager& FirebaseManager::getSPIFFSManager() { return _spiffsManager; }

WiFiManager& FirebaseManager::getWiFiManager() {return _wifiManager;}


bool FirebaseManager::checkForFirmwareUpdate() {return downloadCompleteAndReadyToFlash;}
/* Private fucntion*/
void FirebaseManager::streamTask(void* pvParameters) {
    FirebaseManager* manager = static_cast<FirebaseManager*>(pvParameters);
    while (true) {
        while (!manager->_wifiManager.isConnected()) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        if (!Firebase.beginStream(manager->fbdo, VARIABLE_PATH)) {
            Serial.println("REASON: " + manager->fbdo.errorReason());
            vTaskDelay(5000 / portTICK_PERIOD_MS); 
            continue;
        }


        while (manager->_wifiManager.isConnected() && Firebase.readStream(manager->fbdo)) {
            if (manager->fbdo.streamTimeout()) {
                Serial.println("Stream timeout, resuming...");
                Firebase.readStream(manager->fbdo);
            } else if (manager->fbdo.streamAvailable()) {
                manager->handleStreamData(manager->fbdo);
            }
        }
        Serial.println("Wi-Fi disconnected or stream error. Restarting stream...");
        Firebase.endStream(manager->fbdo); 
    }
    vTaskDelete(NULL);  
}

void FirebaseManager::handleStreamData(FirebaseData& fbdo) {
    if (fbdo.dataType() == "string" && fbdo.stringData() == START_UPDATE_FB_STATUS) {
        Serial.println("Variable changed to true. Starting firmware update...");
        getHeaderFrame();
        xTaskCreatePinnedToCore(
            updateFirmwareTask,
            "updateFirmwareTask",  
            8192,                  
            this,                 // Pass the FirebaseManager instance to the task
            1,                     
            NULL,                  
            1);                    
    }
}

void FirebaseManager::handleUploadData()
{
    firebaseUploadTaskHandle = NULL; 
    xTaskCreatePinnedToCore(
    firebaseUploadTask,     /* Task function. */
    "firebaseUploadTask",   /* String with name of task. */
    10000,                  /* Stack size in bytes. */
    this,                   /* Parameter passed as input of the task */
    configMAX_PRIORITIES - 1,                      /* Priority of the task. */
    &firebaseUploadTaskHandle, /* Task handle to keep track of created task */
    1);                     /* Pin task to core 1 */
}

void FirebaseManager::firebaseUploadTask(void *pvParameters) {
    FirebaseManager* manager = static_cast<FirebaseManager*>(pvParameters);
    while (1) {
        // 1. Wait for Notification
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Wait indefinitely for a notification

        // 2. Get Data from Queue (Optional)
        // You might have a queue to pass data from other tasks to this one.
        // Here's a basic example (you'll need to create and manage the queue):

        String data;
        if (xQueueReceive(manager->firebaseDataQueue, &data, portMAX_DELAY) == pdPASS) {
            // Data received, process it...
            uploadDataSensor((const char*)data.c_str(),manager->fbdo1);
        }

        // 3. Perform Firebase Upload
        // Call your uploadDataSensor function here with the received data.

        // 4. Optional: Signal Completion
        // You can notify other tasks that the upload is finished if needed.
    }
}
    // Task function to handle firmware update
void FirebaseManager::updateFirmwareTask(void* pvParameters) {
        FirebaseManager* manager = static_cast<FirebaseManager*>(pvParameters);
        SPIFFSManager& spiffsManager = manager->getSPIFFSManager();
        // Serial.println(spiffsManager.isInitialized());
        String downloadURL = manager->getDownloadURLFromFirebase(); 
        if (!spiffsManager.isInitialized()) {
            Serial.println("SPIFFS is not initialized. Cannot download firmware.");
            vTaskDelete(NULL); // Exit task if SPIFFS failed
            return;
        }


        if (!downloadURL.isEmpty()) {
            int fileLength = 0; // Not used in this case, as downloadFile handles it internally

            // Use SPIFFSManager to download and save the file
            if (spiffsManager.downloadFile(downloadURL, localFilePath)) {
                Serial.println("Firmware update downloaded successfully!");
                // transferFile(localFilePath); // Start firmware update process
                // spiffsManager.readFile(localFilePath);
                manager->downloadCompleteAndReadyToFlash = true;
                Firebase.setString(manager->fbdo1, F("/Firmware/update_status"), F("false"));
                //write success
            } else {
                Serial.println("Firmware download failed.");
            }
        } else {
            Serial.println("Error getting download URL.");
        }
        vTaskDelete(NULL);
    }

void FirebaseManager::updateFirmwareStatus(const char* status)
    {
        Serial.println(status);
    }
bool FirebaseManager::setDownloadCompleteAndReadyToFlash(bool state)
    {
        downloadCompleteAndReadyToFlash = state;
        return downloadCompleteAndReadyToFlash;
    }
    
    
String FirebaseManager::parseStringValue(FirebaseJson &json, const String &path, const String &targetType ) 
    {
        FirebaseJsonData result;
        if (json.get(result, path) && result.success) {
            
            if (targetType == "string" && result.type == "string") {
            return result.to<String>();
            } else if (targetType == "json") {
            return result.to<String>(); // Return the JSON object as a string
            }
        }
        return "null"; // Or another error indicator value if parsing fails
    }

int FirebaseManager::parseIntValue(FirebaseJson &json, const String &path) 
    {
        FirebaseJsonData result;
        // if (json.get(result, path) && result.success) {
        //     return result.to<int>();
        // }
        // return ; // Or another error indicator value if parsing fails
        json.get(result, path);
        return result.to<int>();
    }

void FirebaseManager::downloadJson(const String &path)
    {
        memset(header, 0, sizeof(header)); 
        FirebaseJson json;
        // FirebaseJsonData result;
        TickType_t ticks = xTaskGetTickCount();
        if(Firebase.getJSON(fbdo1,headerFirebaseFilePath,&json))
        {
            
            char byteString[2] ;
            uint8_t cnt = 0;
            uint32_t codesize = 0;
            String NodeID = parseStringValue(json,"/node_id");
            for(uint8_t i = 0; i < 4; i++)
            {
                strcpy(byteString,(char*)NodeID.substring(i*2, i*2+2).c_str());
                header[cnt++] = x2i(byteString);
            }
            header[cnt++] = ESP_SEND_HEADER_FLAG;
            codesize = parseIntValue(json,"/Codesize");
            for(uint8_t i = 0; i < 4; i++)
            {
                header[cnt++] = codesize%100;
                codesize /=100;
            }
            header[cnt++] = parseIntValue(json,"/Appvermain");
            header[cnt++] = parseIntValue(json,"/Appversub") ;
            header[cnt++] = parseIntValue(json,"/BW") ;
            header[cnt++] = parseIntValue(json,"/SF") ;
            header[cnt++] = parseIntValue(json,"/CR") ;
            // Serial.println(header);
        }
        else
        {
            // Check for specific error reasons
            if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
            {
                Serial.println("ERROR: Firebase get operation failed. Error reason: " + fbdo.errorReason());
            }
            else
            {
                Serial.println("ERROR: HTTP error code: " + String(fbdo.httpCode()));
            }
        }
        Serial.print("Get header 1: ");
        Serial.println(ticks - xTaskGetTickCount());
    }

bool FirebaseManager::getHeaderFrame()
{
    memset(header, 0, sizeof(header)); 
    TickType_t ticks = xTaskGetTickCount();
    Firebase.getString(fbdo1, "/Firmware/node_id") ;
    char byteString[2] ;
    uint8_t cnt = 0;
    uint32_t codesize = 0;
    for(uint8_t i = 0; i < 4; i++)
    {
        strcpy(byteString,(char*)fbdo1.stringData().substring(i*2, i*2+2).c_str());
        header[cnt++] = x2i(byteString);
    }
    header[cnt++] = ESP_SEND_HEADER_FLAG;
    Firebase.getInt(fbdo1, "/Firmware/Codesize") ;
    codesize = fbdo1.intData();
    for(uint8_t i = 0; i < 4; i++)
    {
        header[cnt++] = codesize%100;
        codesize /=100;
    }
        
    Firebase.getInt(fbdo1, "/Firmware/Appvermain") ;
    header[cnt++] = fbdo1.intData();
     Firebase.getInt(fbdo1, "/Firmware/Appversub") ;
    header[cnt++] = fbdo1.intData();

    Firebase.getInt(fbdo1, "/Firmware/BW") ;
    header[cnt++] = fbdo1.intData();
     Firebase.getInt(fbdo1, "/Firmware/SF") ;
    header[cnt++] = fbdo1.intData();
     Firebase.getInt(fbdo1, "/Firmware/CR") ;
    header[cnt++] = fbdo1.intData();
    Serial.println(header);
    Serial.print("Get header 2: ");
    Serial.println(ticks - xTaskGetTickCount());
    return true;
}





void FirebaseManager::uploadDataSensor(const char *data, FirebaseData& fbdo1) {
    
    Serial.println("Starting Data Upload to Firebase");
    
    Serial.println(data);
    // 1. Prepare Data
    String data_str(data);
    String data_deviece_id = data_str.substring(0, 4);
    char device_id[9]; 
    for (int i = 0; i < 4; i++) {
        sprintf(&device_id[i * 2], "%02X", data_deviece_id[i]);
    }
    
    //String path = sensorDataFirebaseFilePath + "/" + device_id + "/value";
    String path = String(sensorDataFirebaseFilePath) + String(device_id) + "/value"; 
    // Serial.println(device_id);
    Serial.println(device_id);
    uint8_t sensor_number = (data_str.length() - DEVICE_ID_SIZE) / DEVICE_SENSOR_DATA_FRAME_SIZE;
    Serial.println(sensor_number);
    char sensor_data_value[5]; // Buffer for formatted data

    // 2. Upload to Firebase
    
    for (int i = 0; i < DEVICE_SENSOR_NUMBER_DEFAULT; i++) {
        String valuePath = path + String(i + 1); // Create path for each value
        // Serial.println(valuePath);
        if (i + 1 <= sensor_number) {
            // Extract and format the sensor value
            String buffer_tmp = data_str.substring(DEVICE_SENSOR_DATA_FRAME_SIZE * i + DEVICE_ID_SIZE, DEVICE_SENSOR_DATA_FRAME_SIZE * i + DEVICE_ID_SIZE * DEVICE_SENSOR_DATA_FRAME_SIZE).c_str(); // Get 3 characters

            if (buffer_tmp[0] == '1'-'0') { 
                // Floating point value
                sprintf(sensor_data_value, "%d.%02d", buffer_tmp[1], buffer_tmp[2]);
            } else {
                // ADC value
                sprintf(sensor_data_value, "%02d%02d", buffer_tmp[1], buffer_tmp[2]);
            }
            
            if(Firebase.setString(fbdo1, valuePath, sensor_data_value))
                Serial.println("Success");
            else 
                Serial.println("Fail");
        } else {
            Firebase.setString(fbdo1, valuePath, "NA"); // No data for this sensor
        }
    }


}
void FirebaseManager::uploadDataSensorToFB(const String &path,uint8_t *sensor_number, uint16_t *sensor_data)    
{
    FirebaseJson json;

    // 4. Fetch Existing Data from Firebase
    Serial.println("Fetching existing data from Firebase...");
    if (Firebase.getJSON(fbdo1, path, &json)) {
        if (fbdo1.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK) {
            Serial.println("Updating existing Firebase data...");

            // 5. Update JSON Data
            updateSensorValues(json, sensor_number, sensor_data); 

            // 6. Upload Updated Data to Firebase
            if (Firebase.set(fbdo1, path, &json)) {
                Serial.println("Data uploaded successfully!");
            } else {
                Serial.printf("Firebase set failed: %s\n", fbdo1.errorReason().c_str());
            }
        } else {
            Serial.printf("Firebase getJSON failed: %s\n", fbdo1.errorReason().c_str());
        }
    } else {
        Serial.println("No existing data found. Creating new entry...");
        
        // If no existing data, create new JSON
        FirebaseJson json;
        for (uint8_t i = 0; i < *sensor_number; i++) {
            json.set("value" + String(i + 1), String(sensor_data[i]));
        }

        if (Firebase.set(fbdo1, path, json)) {
            Serial.println("Data uploaded successfully!");
        } else {
            Serial.printf("Firebase set failed: %s\n", fbdo1.errorReason().c_str());
        }
    }
}


String FirebaseManager::parseSensorData(const String &data, uint8_t *sensor_number, uint16_t *sensor_data) {
    *sensor_number = (data.length() - DEVICE_ID_SIZE) / DEVICE_SENSOR_DATA_FRAME_SIZE;
    String device_id = data.substring(0, DEVICE_ID_SIZE);
    for(uint8_t i = 0; i < *sensor_number ; i++)
    {
        sensor_data[i] = (uint16_t)x2i((char*)data.substring(3 + i*3 + 1, 3 + i*3 + 3 - 1).c_str());
        sensor_data[i] = 100*sensor_data[i] + x2i((char*)data.substring(3 + i*3 + 2, 3 + i*3 + 3).c_str());
    }
    return device_id;
}

void FirebaseManager::updateSensorValues(FirebaseJson &json, uint8_t *sensor_number, const uint16_t *sensor_data) {
    for(uint8_t i = 0; i < *sensor_number; i++)
    {
        json.set("value" + String(i+1), String(sensor_data[i]));
    }
}



uint8_t FirebaseManager::x2i(char *s) 
{
  uint8_t x = 0;
  for(;;) {
    char c = *s;
    if (c >= '0' && c <= '9') {
      x *= 16;
      x += c - '0'; 
    }
    else if (c >= 'A' && c <= 'F') {
      x *= 16;
      x += (c - 'A') + 10; 
    }
    else if (c >= 'a' && c <= 'f') {
      x *= 16;
      x += (c - 'a') + 10; 
    }
    else break;
    s++;
  }
  return x;
}

String FirebaseManager::i2x(uint8_t num) {
    const char hexDigits[] = "0123456789ABCDEF";
    String hexString;
    hexString += hexDigits[num >> 4];
    hexString += hexDigits[num & 0x0F];
    return hexString;
}
