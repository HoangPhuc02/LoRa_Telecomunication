#include "UpdateManager.h"

// #define MASTER_ACCEPT_PACKET 0xA5
// #define NEW_UPDATE_REQUEST_ACCEPT 0xA1
// #define NEW_UPDATE_REQUEST 0xA0

// ... (Include necessary headers)

// static byte Node_Add = 1;
// static byte Flag = HEADER_FLAG_FW_INFO;
// static byte Size_SW[4] = {0x80,0x13,0x00,0x00};
// static byte App_Main_Ver = 0x01;
// static byte App_Sub_Ver = 0x00;
// static byte Bandwidth = 0;
// static byte SF = 0;
// static byte CR = 0;
// static byte reserve[5];
// static char header[HEADER_SIZE] = {Node_Add,Flag,Size_SW[0],Size_SW[1],Size_SW[2],Size_SW[3],App_Main_Ver,App_Sub_Ver,Bandwidth,SF,CR};

UpdateManager::UpdateManager(UARTManager& uartManager, const char* spiffsFilePath, FirebaseManager& firebaseManager, SPIFFSManager& spiffsManager)
    : uartManager(uartManager), SPIFFSFilePath(spiffsFilePath), firebaseManager(firebaseManager), spiffsManager(spiffsManager) {}

void UpdateManager::init() {
    xTaskCreate(updateTask, "update_task", 8192, this, configMAX_PRIORITIES, 0);
}

void UpdateManager::updateTask(void* pvParameters) {
    UpdateManager* updateManager = static_cast<UpdateManager*>(pvParameters);

    while (true) {
        updateManager->loop();
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void UpdateManager::loop() {
    
    
    switch (mode) {
        case STATE_IDLE:
            if (firebaseManager.checkForFirmwareUpdate()) {
                startFlashSw();
            }
            else if (uartManager.updateDatatoFB == true)
            {
                
                Serial.println("go to function");
                String data = (String(uartManager.rxBuffer)).substring(0,uartManager.rxBufferSize);
                
                Serial.println(data);

                xTaskNotifyGive(firebaseManager.firebaseUploadTaskHandle); 
                while (true) {
                    if (xQueueSend(firebaseManager.firebaseDataQueue, &data, portMAX_DELAY) != pdPASS) {
                        Serial.println("Failed to send buffer to queue!");
                    } else {
                        break;
                    }
                } 

                uartManager.updateDatatoFB = false;

            }
            break;
        case NEW_UPDATE_REQUEST_MODE:
            Serial.println("NEW_UPDATE_REQUEST_MODE");
            waitStmAcpUpdateReq();
            break;

        case ESP_SEND_HEADER_FLAG:
            sendHeaderFile();
            break;
        case HEADER_FLAG_RECEIVED_MODE:
            waitStmAcpHeader();
            break;

        case ESP_SEND_NEXT_PACKET_MODE:
            Serial.println("ESP_SEND_NEXT_PACKET_MODE");
            startSendFw();
            break;
        case DONE_OTA_MODE:
            waitStmOTADone();
            break;
        default:
            break;
    }
}

// ... (Implement other UpdateManager methods: waitStmAcpUpdateReq, startFlashSw, 
// sendHeaderFile, startSendFw, transferFile as before, but using spiffsManager and uartManager)
void UpdateManager::waitStmAcpHeader()
{
    // strcpy(uartManager.command , "");
    // strcpy(uartManager.command, (char*) HEADER_FLAG_RECEIVED_MODE);

    if(uartManager.waitForCommandHex( HEADER_FLAG_RECEIVED_MODE))
    {
        Serial.println("HEADER_FLAG_RECEIVED_SUCCESS");
        mode = ESP_SEND_NEXT_PACKET_MODE;
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

    if(uartManager.waitForCommandHex( NEW_UPDATE_REQUEST_ACCEPT_MODE))
    {
        Serial.println("NEW_UPDATE_REQUEST_ACCEPT_MODE");
        mode = ESP_SEND_HEADER_FLAG;
    }
    else startFlashSw();

    
}

void UpdateManager::startFlashSw() {
    
    file = spiffsManager.openFile(localFilePath,"r");
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
    firebaseManager.updateFirmwareStatus("FW Update requested");
    mode = NEW_UPDATE_REQUEST_MODE;
}

void UpdateManager::sendHeaderFile() {
    //uint8_t header[HEADER_SIZE];  // Assuming HEADER_SIZE is defined appropriately
    // Fill header with header data... 

    QueueBufferItem_t queueItem;
    memcpy(queueItem.data, firebaseManager.header, HEADER_SIZE);
    queueItem.dataLength = HEADER_SIZE;
    while (true) {
        if (xQueueSend(uartManager.tx_queue, &queueItem, portMAX_DELAY) != pdPASS) {
            Serial.println("Failed to send buffer to queue!");
        } else {
            break;
        }
    }
    mode = HEADER_FLAG_RECEIVED_MODE;
}

void UpdateManager::startSendFw() {

    
    
    transferFile(localFilePath);
    firebaseManager.updateFirmwareStatus("Update complete");
}

void UpdateManager::transferFile(const char* filename) {
    
    QueueBufferItem_t queueItem;
    
    uint8_t buffer[BUFFER_SIZE] = {0};

    // strcpy(uartManager.command , "");
    // strcpy(uartManager.command, (char*)MASTER_ACCEPT_PACKET_MODE);
    if(uartManager.waitForCommandHex(ESP_SEND_NEXT_PACKET_MODE) == RECEIVE_TIME_OUT)
        startFlashSw();
    int filesize = file.size();
    uint8_t cnt = 0;
    while (file.read(buffer, BUFFER_SIZE) > 0) {
        filesize-=BUFFER_SIZE;
        
        memcpy(queueItem.data, buffer, BUFFER_SIZE);
        queueItem.dataLength = BUFFER_SIZE;

        if (xQueueSend(uartManager.tx_queue, &queueItem, portMAX_DELAY) != pdPASS) {
            Serial.println("Failed to send buffer to queue!");
        }
        if(filesize < 0) 
        {
            // strcpy(uartManager.command , "");
            // strcpy(uartManager.command, (char*)MASTER_RECEIVE_ALL_MODE);
            uartManager.waitForCommandHex( MASTER_RECEIVE_ALL_MODE,DISABLE_TIME_OUT);

        }
        else 
        {
            while(uartManager.waitForCommandHex(MASTER_ACCEPT_PACKET_MODE) == RECEIVE_TIME_OUT)
            {
                xQueueSend(uartManager.tx_queue, &queueItem, portMAX_DELAY) != pdPASS ;        
            }
        }
        Serial.println(cnt);   
    }
    firebaseManager.setDownloadCompleteAndReadyToFlash( false);
    file.close();
    Serial.println("\nEnd of file.");
    mode = DONE_OTA_MODE;
}

void UpdateManager::waitStmOTADone()
{
    uartManager.waitForCommandHex(DONE_OTA_MODE,DISABLE_TIME_OUT) ;
    mode = STATE_IDLE;
}
