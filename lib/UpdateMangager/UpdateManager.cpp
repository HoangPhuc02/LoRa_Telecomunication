#include "UpdateManager.h"


UpdateManager::UpdateManager(UARTManager& uartManager, const char* spiffsFilePath, FirebaseManager& firebaseManager, LittleFSManager& spiffsManager)
    : uartManager(uartManager), LittleFSFilePath(spiffsFilePath), firebaseManager(firebaseManager), spiffsManager(spiffsManager) {}

void UpdateManager::init() {
    xTaskCreate(updateTask, "update_task", 8192, this, UPDATE_TASK_LOOP_PRIORITY , UPDATE_TASK_LOOP_CORE);
}

void UpdateManager::updateTask(void* pvParameters) {
    UpdateManager* updateManager = static_cast<UpdateManager*>(pvParameters);

    while (true) {
        updateManager->loop();
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void UpdateManager::loop() {
    switch (state) {
        case STATE_IDLE:
            if (firebaseManager.checkForFirmwareUpdate()) {
                startFlashSw();
            }
            if(firebaseManager._wifiManager.wifiState == SYSTEM_EVENT_STA_GOT_IP)
            {
                if(firebaseManager._wifiManager.dataToSend[0] != WiFI_CONNECT_SUCCESS )
                {
                    firebaseManager._wifiManager.dataToSend[0] = WiFI_CONNECT_SUCCESS ;
                    uartManager.sendDataToUARTTx(firebaseManager._wifiManager.dataToSend, 1);
                }
            } 
            else if(firebaseManager._wifiManager.wifiState == SYSTEM_EVENT_STA_DISCONNECTED)
            {
                if(firebaseManager._wifiManager.dataToSend[0] != WiFI_CONNECT_LOSS )
                {
                    firebaseManager._wifiManager.dataToSend[0] = WiFI_CONNECT_LOSS ;
                    uartManager.sendDataToUARTTx(firebaseManager._wifiManager.dataToSend, 1);
                }
            } 
            break;
        case NEW_UPDATE_REQUEST_MODE:
            Serial.println("NEW_UPDATE_REQUEST_MODE");
            waitStmAcpUpdateReq();
            break;

        case ESP_SEND_HEADER_FLAG:
            Serial.println("ESP_SEND_HEADER_FLAG");
            sendHeaderFile();
            break;
        case HEADER_FLAG_RECEIVED_MODE:
            Serial.println("HEADER_FLAG_RECEIVED_MODE");
            waitStmAcpHeader();
            break;

        case ESP_SEND_NEXT_PACKET_MODE:
            Serial.println("ESP_SEND_NEXT_PACKET_MODE");
            startSendFw();
            break;
        case SET_DONE_OTA_MODE:
            setStmOTADone();
            break;
        case DONE_OTA_MODE:
            //Serial.println("DONE_OTA_MODE");
            waitStmOTADone();
            break;
        default:
            break;
    }
}
void UpdateManager::storeStateToLittleFS(uint8_t stateSet)
{
    while(spiffsManager.writeUint8(storeStateFilePath,stateSet) == false);
}

void UpdateManager::setState(uint8_t stateSet){
    state = stateSet;
    if(state == NEW_UPDATE_REQUEST_MODE)
        Serial.println("Current state : " + String(NEW_UPDATE_REQUEST));
    else if(state == STATE_IDLE)
        Serial.println("Current state : STATE_IDLE" );
    else if (state == SET_DONE_OTA_MODE)
        Serial.println("Current state : WAIT_OTA_DONE" );
}

uint8_t UpdateManager::readState(){
    uint8_t oldState;
    while(spiffsManager.readUint8(storeStateFilePath,oldState) == false);
    if(state == NEW_UPDATE_REQUEST_MODE)
        Serial.println("Last state : " + String(NEW_UPDATE_REQUEST));
    else if(state == STATE_IDLE)
        Serial.println("Last state : STATE_IDLE" );
    else if (state == SET_DONE_OTA_MODE)
        Serial.println("Last state : WAIT_OTA_DONE" );
    return oldState;
}
// ... (Implement other UpdateManager methods: waitStmAcpUpdateReq, startFlashSw, 
// sendHeaderFile, startSendFw, transferFile as before, but using spiffsManager and uartManager)
void UpdateManager::waitStmAcpHeader()
{
    // strcpy(uartManager.command , "");
    // strcpy(uartManager.command, (char*) HEADER_FLAG_RECEIVED_MODE);

    if(uartManager.waitForCommandHex( HEADER_FLAG_RECEIVED_MODE,0))
    {
        Serial.println("HEADER_FLAG_RECEIVED_SUCCESS");

        state = ESP_SEND_NEXT_PACKET_MODE;
        //storeStateToLittleFS(state);
    }
    else 
        startFlashSw();

    
}

void UpdateManager::updateDataFromUart(char* buffer, uint16_t size, char* command) {
    if (buffer && *buffer) {
        if (!strcmp(buffer, command)) {
            Serial.print("ACCEPT KEY");
            ACP_KEY_RX = true;
            return;
        }
        Serial.print("WRONG KEY");
    }
}
// ... (Other UpdateManager methods)

void UpdateManager::waitStmAcpUpdateReq() {
    
    // strcpy(uartManager.command , "");
    // strcpy(uartManager.command, NEW_UPDATE_REQUEST_ACCEPT);

    if(uartManager.waitForCommandHex( NEW_UPDATE_REQUEST_ACCEPT_MODE,0) == RECEIVE_SUCCESS)
    {
        Serial.println("NEW_UPDATE_REQUEST_ACCEPT_MODE");

        state = ESP_SEND_HEADER_FLAG;
        // storeStateToLittleFS(state);
    }
    else startFlashSw();

    
}

void UpdateManager::startFlashSw() {
    
    file = spiffsManager.openFile(localFilePath,"r");
    Serial.printf("Size of firmware: %d\n",file.size());
    QueueBufferItem_t queueItem;
    // memcpy(queueItem.data, (char*)NEW_UPDATE_REQUEST, sizeof(NEW_UPDATE_REQUEST));
    // queueItem.dataLength = sizeof(NEW_UPDATE_REQUEST);
    /* for hex*/
    queueItem.data[0] = NEW_UPDATE_REQUEST_MODE;
    queueItem.dataLength = 1;

    while (true) {
        if (xQueueSend(uartManager.tx_queue, &queueItem, portMAX_DELAY) != pdPASS) {
            Serial.println("Failed to send buffer to queue!");
        } else {
            break;
        }
    }
    Serial.println("Update requested");
    
    state = NEW_UPDATE_REQUEST_MODE;
    storeStateToLittleFS(state);
}

void UpdateManager::sendHeaderFile() {
    //uint8_t header[HEADER_SIZE];  // Assuming HEADER_SIZE is defined appropriately
    // Fill header with header data... 
    uint8_t header[HEADER_SIZE];
    File headerFile = spiffsManager.openFile(headerFilePath,"r");
    headerFile.read(header,HEADER_SIZE);
    uartManager.sendDataToUARTTx((char*)header, HEADER_SIZE );
    // QueueBufferItem_t queueItem;
    // memcpy(queueItem.data, firebaseManager.header, HEADER_SIZE);
    // queueItem.dataLength = HEADER_SIZE;
    // while (true) {
    //     if (xQueueSend(uartManager.tx_queue, &queueItem, portMAX_DELAY) != pdPASS) {
    //         Serial.println("Failed to send buffer to queue!");
    //     } else {
    //         break;
    //     }
    // }
    state = HEADER_FLAG_RECEIVED_MODE;
    // storeStateToLittleFS(state);
}

void UpdateManager::startSendFw() {

    
    
    transferFile(localFilePath);
    Serial.println("Download firmware complete");
}

void UpdateManager::transferFile(const char* filename) {

    QueueBufferItem_t queueItem;
    // Test specific will be delete
    // digitalWrite(PIN_CHECK_STATE,HIGH);
    // digitalWrite(PIN_CHECK_TIME,HIGH);
    //====================================
    uint8_t buffer[BUFFER_SIZE] = {0};

    // strcpy(uartManager.command , "");
    // strcpy(uartManager.command, (char*)MASTER_ACCEPT_PACKET_MODE);
    uartManager.waitForCommandHex(ESP_SEND_NEXT_PACKET_MODE,ESP_SEND_NEXT_PACKET_MODE,ENABLE_TIME_OUT,10000);

    //startFlashSw();
    int filesize = file.size();
    uint8_t cnt = 0;
    Serial.printf("Size of packet: %d\n",filesize);

    // while (file.read(buffer, BUFFER_SIZE) > 0 ) {
    while (true){
        // Test specific will be delete
        // digitalWrite(PIN_CHECK_STATE,LOW);
        // digitalWrite(PIN_CHECK_TIME,LOW);
        //=====================================
        size_t bytesRead = file.read(buffer, BUFFER_SIZE) ;
        if(bytesRead <= 0) break;
        //===============================
        filesize-=BUFFER_SIZE;
        Serial.printf("Sending packet number: %d\n",cnt++);  
        memcpy(queueItem.data, buffer, BUFFER_SIZE);
        queueItem.dataLength = BUFFER_SIZE;
          // Test specific will be delete

        // digitalWrite(PIN_CHECK_TIME,HIGH);
        /*====================================*/
        if (xQueueSend(uartManager.tx_queue, &queueItem, pdMS_TO_TICKS(10)) != pdPASS) {
            Serial.println("Failed to send buffer to queue!");
        }
        // // Test specific will be delete

        // digitalWrite(PIN_CHECK_TIME,LOW);

        // //===============================
        if(filesize <= 0) 
        {
            // strcpy(uartManager.command , "");
            // strcpy(uartManager.command, (char*)MASTER_RECEIVE_ALL_MODE);
            Serial.println("Received all packets");
            uartManager.waitForCommandHex( MASTER_RECEIVE_ALL_MODE,0,DISABLE_TIME_OUT);

            break;
        }
        else 
        {
            while(uartManager.waitForCommandHex(MASTER_ACCEPT_PACKET_MODE,0) == RECEIVE_TIME_OUT)
            {
                xQueueSend(uartManager.tx_queue, &queueItem, portMAX_DELAY) != pdPASS ;        
            }
            /*Test specific will be delete*/
            // digitalWrite(PIN_CHECK_STATE,HIGH);
            // digitalWrite(PIN_CHECK_TIME,HIGH);
            /*====================================*/
        }
         
    }
    
    firebaseManager.setDownloadCompleteAndReadyToFlash( false);
    file.close();
    // Serial.println("\nEnd of file.");
    uartManager.isReceveWrongCommand = false;

    state = SET_DONE_OTA_MODE;
    storeStateToLittleFS(state);
}
void UpdateManager::setStmOTADone()
{
    uartManager.command[0] = DONE_OTA_MODE;
    uartManager.command[1] = FAIL_OTA_MODE;
    state = DONE_OTA_MODE;
}
void UpdateManager::waitStmOTADone()
{
    //Serial.println("wait ota hex code");
    //uartManager.isReceveWrongCommand = false;  
    
    //uartManager.waitForCommandHex(DONE_OTA_MODE,FAIL_OTA_MODE,DISABLE_TIME_OUT,2000) ;
    // uartManager.command[0] = DONE_OTA_MODE;
    // uartManager.command[1] = FAIL_OTA_MODE;
    
        if(uartManager.isReceveWrongCommand == true)
        {
            
            if(uartManager.command[0] ==  0)
            {
                Serial.println("OTA done" );
                xTaskNotify(firebaseManager.firebaseUploadTaskHandle,UpdateStatusDone,eSetValueWithOverwrite); 
            }
            else if(uartManager.command[1] == 0)
            {
                Serial.println("OTA fail" );
                xTaskNotify(firebaseManager.firebaseUploadTaskHandle,UpdateStatusFail,eSetValueWithOverwrite); 
                
            }
                
            uartManager.isReceveWrongCommand = false;
            state = STATE_IDLE;
            storeStateToLittleFS(state);
        }
        if(firebaseManager._wifiManager.wifiState == SYSTEM_EVENT_STA_GOT_IP)
        {
            if(firebaseManager._wifiManager.dataToSend[0] != WiFI_CONNECT_SUCCESS )
            {
                firebaseManager._wifiManager.dataToSend[0] = WiFI_CONNECT_SUCCESS ;
                uartManager.sendDataToUARTTx(firebaseManager._wifiManager.dataToSend, 1);
            }
        } 
        else if(firebaseManager._wifiManager.wifiState == SYSTEM_EVENT_STA_DISCONNECTED)
        {
            if(firebaseManager._wifiManager.dataToSend[0] != WiFI_CONNECT_LOSS )
            {
                firebaseManager._wifiManager.dataToSend[0] = WiFI_CONNECT_LOSS ;
                uartManager.sendDataToUARTTx(firebaseManager._wifiManager.dataToSend, 1);
            }
        }
    
}
