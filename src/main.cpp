
#include<ProjectConfig.h>
#include <Arduino.h>
#include <Wire.h>

#include <String>
#include "stdint.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

//#include <MyWiFiManager.h>

#include "UpdateManager.h"
#include "FirebaseManager.h"

#include "UARTManager.h"
#include <SPI.h>



LittleFSManager spiffs_manager;
MyWiFiManager wifi_manager(WIFI_SSID,WIFI_PASSWORD);
FirebaseManager fb_manager(wifi_manager,spiffs_manager);
UARTManager uart_manager(fb_manager,UART);
UpdateManager update_manager(uart_manager, "/firmware.bin", fb_manager, spiffs_manager);

void setup() {

  Serial.begin(115200);

  uart_manager.init(); 
  spiffs_manager.begin();
  //wifi_manager.setOnDisconnectCallback(&UARTManager::sendDataToUARTTx);
  wifi_manager.dataToSend[0] = WiFI_CONNECT_LOSS ;
  uart_manager.sendDataToUARTTx(wifi_manager.dataToSend, 1);
  wifi_manager.begin();
             
 
  while(!wifi_manager.isConnected());
  wifi_manager.dataToSend[0] = WiFI_CONNECT_SUCCESS ;
  uart_manager.sendDataToUARTTx(wifi_manager.dataToSend, 1);

  fb_manager.begin();
  if(spiffs_manager.fileExists(storeStateFilePath))
  {
    Serial.println("State file exit");
      uint8_t state = update_manager.readState();
      update_manager.setState(state);
  }
  else 
  {
    Serial.println("State file doesnt exit");
    update_manager.setState(STATE_IDLE);
    update_manager.storeStateToLittleFS(STATE_IDLE);
  }

  update_manager.init();
  fb_manager.startMonitoringVariable();
  //vTaskSuspend(fb_manager.firebaseStreamTaskHandle);  


  
}
void loop()
{}


