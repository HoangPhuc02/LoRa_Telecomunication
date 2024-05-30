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
        firebaseSemaphore = xSemaphoreCreateBinary();
        xSemaphoreGive(firebaseSemaphore);

        firebaseStreamTaskHandle        = NULL; 
        firebaseDownloadFWTaskHandle    = NULL;  
        firebaseUploadTaskHandle = NULL; 
        firebaseDataQueue = xQueueCreate(5, sizeof(QueueUploadData_t));
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
        if (fbdo1.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK) {
            Serial.println("ERROR: Firebase get operation failed. Error reason: " + fbdo1.errorReason());
        } else {
            Serial.println("ERROR: HTTP error code: " + String(fbdo1.httpCode()));
        }
    }
    return "";
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
    fbdo2.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
    Firebase.begin(&config, &auth);
    // Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
    Serial.println("Connect firebase success");
}


SPIFFSManager& FirebaseManager::getSPIFFSManager() { return _spiffsManager; }

WiFiManager& FirebaseManager::getWiFiManager() {return _wifiManager;}


void FirebaseManager::startMonitoringVariable() {
    Serial.println("startMonitoringVariable");
    xTaskCreatePinnedToCore(streamTask, "streamTask", 8192, this, FB_MONITOR_VALUE_PRORITY, &firebaseStreamTaskHandle, FB_MONITOR_VALUE_CORE);
}

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
         // Suspend the task
        
        
        xTaskCreatePinnedToCore(
            updateFirmwareTask,
            "updateFirmwareTask",  
            8192,                  
            this,                 // Pass the FirebaseManager instance to the task
            FB_UPLOAD_FIRMWARE_PRIORITY,                     
            &firebaseDownloadFWTaskHandle,                  
            FB_UPLOAD_FIRMWARE_CORE);  

                        
    }
}

void FirebaseManager::handleUploadData()
{
    
    xTaskCreatePinnedToCore(
    firebaseUploadTask,     /* Task function. */
    "firebaseUploadTask",   /* String with name of task. */
    10000,                  /* Stack size in bytes. */
    this,                   /* Parameter passed as input of the task */
    FB_UPLOAD_DATA_PRIORITY ,                      /* Priority of the task. */
    &firebaseUploadTaskHandle, /* Task handle to keep track of created task */
    FB_UPLOAD_DATA_CORE);                     /* Pin task to core 1 */
}

void FirebaseManager::firebaseUploadTask(void *pvParameters) {
    FirebaseManager* manager = static_cast<FirebaseManager*>(pvParameters);
    uint32_t ulNotifiedValue;
    while (1) {
        // 1. Wait for Notification
        //ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Wait indefinitely for a notification
        if(xTaskNotifyWait(0, 0, &ulNotifiedValue, portMAX_DELAY) == pdPASS)
        { 
            // 2. Get Data from Queue (Optional)
            // You might have a queue to pass data from other tasks to this one.
            // Here's a basic example (you'll need to create and manage the queue):
            if (xSemaphoreTake(manager->firebaseSemaphore, portMAX_DELAY) == pdTRUE) {


                if (ulNotifiedValue == UpdateStatusDone) {
                    
                    if (Firebase.setString(manager->fbdo2, VARIABLE_PATH, "done")) {
                        Serial.println("Firebase update status to done successful!");
                        vTaskResume(manager->firebaseStreamTaskHandle);
                    } else {
                        Serial.printf("Firebase update failed: %s\n", (manager->fbdo2).errorReason().c_str());

                    }
                }
                else if (ulNotifiedValue == UpdateStatusFail) {
                    
                    if (Firebase.setString(manager->fbdo2, VARIABLE_PATH, "fail")) {
                        Serial.println("Firebase update status to fail successful!");
                        vTaskResume(manager->firebaseStreamTaskHandle);
                    } else {
                        Serial.printf("Firebase update failed: %s\n", (manager->fbdo2).errorReason().c_str());

                    }
                }
                // Handle UpdateStatus notification
                else if (ulNotifiedValue == SensorData) {
                    // Handle SensorData notification
                    QueueUploadData_t queueUpload;
                    if (xQueueReceive(manager->firebaseDataQueue, &queueUpload, portMAX_DELAY) == pdPASS) {
                        // Data received, process it...
                
                        uploadDataSensor(queueUpload.type, queueUpload.path,(const char*)queueUpload.data.c_str(),manager->fbdo2);
                        //Serial.println("Update success");
                    }
                }
                
                xSemaphoreGive(manager->firebaseSemaphore);
            }
        }
        //vTaskDelay(20 / portTICK_PERIOD_MS);

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
    vTaskSuspend(manager->firebaseStreamTaskHandle);  
    while(true)
    {
        if (xSemaphoreTake(manager->firebaseSemaphore, portMAX_DELAY) == pdTRUE)
        {
            manager->getHeaderFrame(manager->fbdo1);
            // Serial.println(spiffsManager.isInitialized());
            Serial.println("Start Update FW Task");
            String downloadURL = manager->getDownloadURLFromFirebase(); 
            if (!spiffsManager.isInitialized()) {
                Serial.println("SPIFFS is not initialized. Cannot download firmware.");
                vTaskDelete(NULL); // Exit task if SPIFFS failed
                return;
            }


            if (!downloadURL.isEmpty()) {
                int fileLength = 0; // Not used in this case, as downloadFile handles it internally
                Serial.println("Start Update FW Task");
                // Use SPIFFSManager to download and save the file
                if (spiffsManager.downloadFile(downloadURL, localFilePath)) {
                    Serial.println("Firmware update downloaded successfully!");
                    // transferFile(localFilePath); // Start firmware update process
                    // spiffsManager.readFile(localFilePath);
                    manager->downloadCompleteAndReadyToFlash = true;
                    Firebase.setString(manager->fbdo1, F("/Firmware/update_status"), F("updating"));
                    //write success
                } else {
                    Serial.println("Firmware download failed.");
                }
            } else {
                Serial.println("Error getting download URL.");
            }
            xSemaphoreGive(manager->firebaseSemaphore);
            vTaskDelete(NULL);
        }
    }
}


bool FirebaseManager::setDownloadCompleteAndReadyToFlash(bool state)
    {
        downloadCompleteAndReadyToFlash = state;
        return downloadCompleteAndReadyToFlash;
    }
    
    


bool FirebaseManager::getHeaderFrame(FirebaseData &fbdo)
{
    Serial.println("Start Get Header");
    memset(header, 0, sizeof(header)); 
    // TickType_t ticks = xTaskGetTickCount();
    Firebase.getString(fbdo, "/Firmware/node_id") ;
    char byteString[2] ;
    uint8_t cnt = 0;
    uint32_t codesize = 0;
    for(uint8_t i = 0; i < 4; i++)
    {
        strcpy(byteString,(char*)fbdo.stringData().substring(i*2, i*2+2).c_str());
        header[cnt++] = x2i(byteString);
    }
    header[cnt++] = ESP_SEND_HEADER_FLAG;
    Firebase.getInt(fbdo, "/Firmware/Codesize") ;
    codesize = fbdo.intData();
    for(uint8_t i = 0; i < 4; i++)
    {
        header[cnt++] = codesize%100;
        codesize /=100;
    }
        
    Firebase.getInt(fbdo, "/Firmware/Appvermain") ;
    header[cnt++] = fbdo.intData();
    Firebase.getInt(fbdo, "/Firmware/Appversub") ;
    header[cnt++] = fbdo.intData();

    Firebase.getInt(fbdo, "/Firmware/BW") ;
    header[cnt++] = fbdo.intData();
    Firebase.getInt(fbdo, "/Firmware/SF") ;
    header[cnt++] = fbdo.intData();
    Firebase.getInt(fbdo, "/Firmware/CR") ;
    header[cnt++] = fbdo.intData();
    Serial.println(header);
    // Serial.print("Get header 2: ");
    // Serial.println(ticks - xTaskGetTickCount());
    return true;
}


// void FirebaseManager::updateStringFB(const char *data,const String &path)
// {
//     while((Firebase.setString(fbdo1, path, data)) == false)
//     {
//         vTaskDelay(100);
//     }
// }
bool FirebaseManager::updateStringFB(FirebaseData& fbdo,const char *data, const String &path) {
  if (Firebase.setString(fbdo, path, data)) {
    Serial.println("Firebase update successful!");
    return true;
  } else {
    Serial.printf("Firebase update failed: %s\n", fbdo.errorReason().c_str());
    return false; 
  }
}

void FirebaseManager::uploadDataSensor(UploadDataType_t type, const String &path, const char *data, FirebaseData& fbdo) {

    
    if(type == SensorData)
    {
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
        String new_path = String(path) + String(device_id) + "/value"; 
        // Serial.println(device_id);
        Serial.println(device_id);
        uint8_t sensor_number = (data_str.length() - DEVICE_ID_SIZE) / DEVICE_SENSOR_DATA_FRAME_SIZE;
        Serial.println(sensor_number);
        char sensor_data_value[5]; // Buffer for formatted data

        // 2. Upload to Firebase
        
        for (int i = 0; i < DEVICE_SENSOR_NUMBER_DEFAULT; i++) {
            String valuePath = new_path + String(i + 1); // Create path for each value
            Serial.println(valuePath);
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
                Serial.println("Wait for update");
                if(Firebase.setString(fbdo, valuePath, sensor_data_value))
                    Serial.println("Success");
                else 
                    Serial.println("Fail");
            } else {
                Firebase.setString(fbdo, valuePath, "NA"); // No data for this sensor
            }
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
    // for(uint8_t i = 0; i < *sensor_number; i++)
    // {
    //     Firebase.setString(fbdo, valuePath, "NA");
    //     json.set("value" + String(i+1),VARIABLE_PATH, String(sensor_data[i]));
    // }
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
