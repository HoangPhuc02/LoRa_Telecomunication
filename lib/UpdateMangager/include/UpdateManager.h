#ifndef __UPDATE_MANAGER_H
#define __UPDATE_MANAGER_H

#include<ProjectConfig.h>
#include "UARTManager.h" // Use double quotes for local files
#include "FirebaseManager.h"
#include "SPIFFS_FB.h" 


class UpdateManager{
public:
    UpdateManager(UARTManager& uartManager, const char* SPIFFSFilePath, FirebaseManager& firebaseManager, SPIFFSManager& spiffsManager);

    void init();

    void updateDataFromUart(char* buffer, uint16_t size, char* command);
    void loop();
    File file;
private:
    // enum UpdateMode {
    //     STATE_IDLE,
    //     /*FIRMWARE UPDATE*/
    //     NEW_UPDATE_REQUEST_MODE,
    //     NEW_UPDATE_REQUEST_ACCEPT_MODE,
    //     ESP_SEND_NEXT_PACKET_MODE,

    //     /*DATA UPDATE*/
    //     // ... (add more modes if needed)
    // };

    UARTManager& uartManager;
    FirebaseManager& firebaseManager;
    SPIFFSManager& spiffsManager;
    const char* SPIFFSFilePath;
    uint8_t mode = STATE_IDLE;
    //uint8_t mode = NEW_UPDATE_REQUEST_MODE;
    //uint8_t mode = DONE_OTA_MODE;
    bool ACP_KEY_RX = false;

    static void updateTask(void* pvParameters);

    void waitStmAcpUpdateReq();
    void startFlashSw();
    void sendHeaderFile();
    void waitStmAcpHeader();
    void startSendFw();
    void waitStmOTADone();
    void transferFile(const char* filename);
};

#endif // UPDATE_MANAGER_H
