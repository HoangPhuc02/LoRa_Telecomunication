#ifndef __UPDATE_MANAGER_H
#define __UPDATE_MANAGER_H

#include<ProjectConfig.h>
#include "UARTManager.h" // Use double quotes for local files
#include "FirebaseManager.h"
#include "SPIFFS_FB.h" 


class UpdateManager{
public:
    UpdateManager(UARTManager& uartManager, const char* LittleFSFilePath, FirebaseManager& firebaseManager, LittleFSManager& spiffsManager);

    void init();

    void updateDataFromUart(char* buffer, uint16_t size, char* command);
    void loop();
    void setState(uint8_t stateSet);
    uint8_t readState();

    void storeStateToLittleFS(uint8_t stateSet);
    File file;

private:


    UARTManager& uartManager;
    FirebaseManager& firebaseManager;
    LittleFSManager& spiffsManager;
    const char* LittleFSFilePath;
    uint8_t state = STATE_IDLE;
    //uint8_t mode =  ESP_SEND_NEXT_PACKET_MODE;
    //uint8_t mode = DONE_OTA_MODE;
    //uint8_t mode = SET_DONE_OTA_MODE;
    bool ACP_KEY_RX = false;
    
    static void updateTask(void* pvParameters);
    void waitStmAcpUpdateReq();
    void startFlashSw();
    void sendHeaderFile();
    void waitStmAcpHeader();
    void startSendFw();
    void setStmOTADone();
    void waitStmOTADone();
    void transferFile(const char* filename);
};

#endif // UPDATE_MANAGER_H
