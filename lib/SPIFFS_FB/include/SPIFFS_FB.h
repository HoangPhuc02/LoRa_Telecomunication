#ifndef __LittleFS_FB_H
#define __LittleFS_FB_H
/** UART config*/
/*Pin */
#include <ProjectConfig.h>

#include <FS.h>
#include <WiFi.h>
// #include <FirebaseESP32.h>
// #include "SPIFFS.h"
#include "LittleFS.h"
#include <HTTPClient.h>
#include <WiFiClient.h>




class LittleFSManager {
public:
    //SPIFFSManager() ;
    void begin();
    bool isInitialized() ;

    bool writeFile(const String &filePath, const uint8_t *data, size_t length);
    void removeFile(const String &filePath);
    String readFile(const String &filePath) ;


    bool readUint8(const String &filePath, uint8_t &value);
    bool writeUint8(const String &filePath, uint8_t value);
    bool downloadFile(const String &downloadURL, const char *filename) ;

    bool fileExists(const char* filePath) ;

    void listFiles() ;
    File openFile(const char* filename, const char* mode) ;
    //void closeFile();

private:
    bool _initialized; 
  
};




#endif