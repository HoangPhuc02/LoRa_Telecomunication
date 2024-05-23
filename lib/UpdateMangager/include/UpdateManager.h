#ifndef __UPDATE_MANAGER_H
#define __UPDATE_MANAGER_H

#include "UARTManager.h" // Use double quotes for local files
#include "FirebaseManager.h"
// #include "SPIFFS_FB.h" 
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"



#include<ProjectConfig.h>
/*ESP To GATEWAY NOTIFY NEW FIRMWARE */







// byte mode = 0;
// bool ACP_KEY_RX = false;





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
