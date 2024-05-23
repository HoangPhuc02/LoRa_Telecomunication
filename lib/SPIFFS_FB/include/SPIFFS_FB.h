#ifndef __SPIFFS_FB_H
#define __SPIFFS_FB_H
/** UART config*/
/*Pin */
#include <Arduino.h>
#include <FS.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "SPIFFS.h"
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <vector> 
#include <string.h>

#include <ProjectConfig.h>


bool setupSPIFFS();

bool downloadFile(const String &downloadURL, const char *filename, int &len) ;


class SPIFFSManager {
public:
    //SPIFFSManager() ;
    void begin();
    bool isInitialized() ;

    bool writeFile(const String &filePath, const uint8_t *data, size_t length);

    String readFile(const String &filePath) ;



    bool downloadFile(const String &downloadURL, const char *filename) ;

    bool fileExists(const char* filePath) ;

    void listFiles() ;
    File openFile(const char* filename, const char* mode) ;
    //void closeFile();

private:
    bool _initialized; 
  
};




#endif