/**
 * SYNTAX:
 *
 * FileConfig::FileConfig(<file_name>, <file_callback>);
 *
 * <file_name> - The filename included path of file that will be used.
 * <file_callback> - The callback function that provides file operation.
 *
 * The file_callback function parameters included the File reference returned from file operation, filename for file operation and file_operating_mode.
 * The file_operating_mode included file_mode_open_read, file_mode_open_write, file_mode_open_append and file_mode_open_remove.
 *
 * The file name can be a name of source (input) and target (output) file that used in upload and download.
 *
 * SYNTAX:
 *
 * RealtimeDatabase::set(<AsyncClient>, <path>, <file_config_data>, <AsyncResultCallback>, <uid>);
 *
 * RealtimeDatabase::get(<AsyncClient>, <path>, <file_config_data>, <AsyncResultCallback>, <uid>);
 *
 * <AsyncClient> - The async client.
 * <path> - The node path to set/get the file data.
 * <file_config_data> - The file config data which in case of filesystem data, it will be obtained from FileConfig via getFile.
 * <AsyncResultCallback> - The async result callback (AsyncResultCallback).
 * <uid> - The user specified UID of async result (optional).
 *
 * The complete usage guidelines, please visit https://github.com/mobizt/FirebaseClient
 */

#include <Arduino.h>
#include <Wire.h>

#include <String>
#include "stdint.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

//#include "UART.h"
// #include <FirebaseLib.h>
// #include <SPIFFS_FB.h>
// // #include <FirebaseLib.h>

#include "UpdateManager.h"
// #include "FirebaseManager.h"

#include "UARTManager.h"


#include <SPI.h>



#include<ProjectConfig.h>






// QueueHandle_t uart_queue = NULL;
// QueueHandle_t TransferFWFile = NULL;
// char commandRX[30] = {0};
// typedef struct 
// {
//   FILE file;
//   const char* fileLocalPath;
//   const char* fileFireBasePath;
//   uint32_t fileSize;
//   uint32_t fileCursor;
// }SPIFFSFile;







SPIFFSManager spiffs_manager;
WiFiManager wifi_manager(WIFI_SSID,WIFI_PASSWORD);
FirebaseManager fb_manager(wifi_manager,spiffs_manager);
UARTManager uart_manager(UART);
UpdateManager update_manager(uart_manager, "/firmware.bin", fb_manager, spiffs_manager);
// firebaseLib fblib;
// UARTInterrupt uart(UART_NUM_1, 16, 17);

// // User-defined callback function for TX done
// void myTxDoneCallback() {
//     Serial.println("Transmission complete!");
//}
void setup() {

  Serial.begin(115200);
  // uart.begin(115200); // Start UART communication with the desired baud rate
  //   uart.onTxDone(myTxDoneCallback); // Register the TX done callback
  // UARTHandler uartHandler;
  // uartHandler.init();

  // // Example transmission
  // const char* message = "Hello, UART!";
  // uartHandler.transmit((const uint8_t*)message, strlen(message));
  uart_manager.init(); 
  spiffs_manager.begin();
  wifi_manager.begin();
  while(!wifi_manager.isConnected());

 // FirebaseManager.downloadVersion()
  QueueBufferItem_t queueItem;
    memcpy(queueItem.data, "WIFI CONNECTED", 15);
    queueItem.dataLength = 15;



    if (xQueueSend(uart_manager.tx_queue, &queueItem, portMAX_DELAY) != pdPASS) {
        Serial.println("Failed to send buffer to queue!");
    }
  
  fb_manager.begin();
  update_manager.init();
  fb_manager.startMonitoringVariable();


  
}
void loop()
{}



