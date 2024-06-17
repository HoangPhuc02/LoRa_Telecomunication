/*
 * Connect the SD card to the following pins:
 *
 * SD Card | ESP32
 *    D2       -
 *    D3       SS
 *    CMD      MOSI
 *    VSS      GND
 *    VDD      3.3V
 *    CLK      SCK
 *    VSS      GND
 *    D0       MISO
 *    D1       -
 */

#include "SPIFFS_FB.h"



    
    void LittleFSManager::begin()
    {
        if (!LittleFS.begin(true)) {
            Serial.println("ERROR: Failed to mount LittleFS");
            _initialized = false;
        } else {
            Serial.println("LittleFS mounted successfully.");
            _initialized = true;

            // Log LittleFS information

            // Serial.println("Total space:      " + String(LittleFS.totalBytes));
            // Serial.println("Used space:       " + String(LittleFS.usedBytes));
            // Serial.println("Block size:       " + String(LittleFS.blockSize));
            // Serial.println("Page size:        " + String(LittleFS.pageSize));
            // Serial.println("Maximum open files: " + String(LittleFS.maxOpenFiles));
            // Serial.println("Maximum path length: " + String(LittleFS.maxPathLength));
        }
    }
    bool LittleFSManager::isInitialized() {
        return _initialized;
    }

    bool LittleFSManager::writeFile(const String &filePath, const uint8_t *data, size_t length) {
        if (!_initialized) {
            Serial.println("ERROR: LittleFS not initialized.");
            return false;
        }

        File file = LittleFS.open(filePath, FILE_WRITE);
        if (!file) {
            Serial.println("ERROR: Failed to open file for writing: " + filePath);
            return false;
        }

        size_t bytesWritten = file.write(data, length);
        file.close();

        if (bytesWritten != length) {
            Serial.println("ERROR: File write incomplete! Wrote " + String(bytesWritten) + " of " + String(length) + " bytes.");
            return false;
        }

        Serial.println("File written successfully: " + filePath);
        return true;
    }

    String LittleFSManager::readFile(const String &filePath) {
        if (!_initialized) {
            Serial.println("ERROR: LittleFS not initialized.");
            return "";
        }

        if (!LittleFS.exists(filePath)) {
            Serial.println("ERROR: File does not exist: " + filePath);
            return "";
        }

        File file = LittleFS.open(filePath, FILE_READ);
        if (!file) {
            Serial.println("ERROR: Failed to open file for reading: " + filePath);
            return "";
        }

        String fileContent;
        while (file.available()) {
            // fileContent += (char)file.read();
            Serial.write(file.read());
        }
        file.close();
        return fileContent;
    }
    bool LittleFSManager::writeUint8(const String &filePath, uint8_t value) {
        if (!_initialized) {
            Serial.println("ERROR: LittleFS not initialized.");
            return false;
        }
        
        File file = LittleFS.open( filePath, FILE_WRITE);
        if (!file) {
            Serial.println("ERROR: Failed to open file for writing: " + filePath);
            return false;
        }
        file.write(value); 
        file.close();
        return true;
    }
     bool LittleFSManager::readUint8(const String &filePath, uint8_t &value) {
        if (!_initialized) {
            Serial.println("ERROR: LittleFS not initialized.");
            return false;
        }

        File file = LittleFS.open(filePath, FILE_READ);
        if (!file) {
            Serial.println("ERROR: Failed to open file for reading: " + filePath);
            return false;
        }
        if (file.available()) {
            value = file.read();
        } else {
            Serial.println("ERROR: File empty: " + filePath);
            return false; 
        }
        file.close();
        return true;
    }



  

bool LittleFSManager::downloadFile(const String &downloadURL, const char *filename) {
    if (!_initialized) {
        Serial.println("ERROR: LittleFS not initialized.");
        return false;
    }
    HTTPClient http;
    if (!http.begin(downloadURL)) {
        Serial.println("ERROR: Unable to begin HTTP request");
        return false;
    }

    int httpCode = http.GET();

    if (httpCode > 0 && (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)) { 
        // Handle redirects (301 Moved Permanently)
        if (httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String newLocation = http.getLocation();
            http.end();  // End the previous request
            http.begin(newLocation);  // Start a new request at the redirected URL
            httpCode = http.GET();
            if (httpCode != HTTP_CODE_OK) {
                Serial.println("HTTP GET failed after redirect!");
                http.end();
                return false;
            }
        }
        
        int fileSize = http.getSize(); // Get Content-Length if available
        Serial.printf("Downloading: %s", filename);
        if (fileSize > 0) {
            Serial.printf(" (%d bytes)\n", fileSize);
        } else {
            Serial.println(" (Unknown size)");
        }
        File file = LittleFS.open(filename, FILE_WRITE);
        if (!file) {
            Serial.println("Error opening file for writing");
            http.end();
            return false;
        }

        WiFiClient *stream = http.getStreamPtr();
        uint32_t downloadedBytes = 0;
        const size_t bufferSize = 512;
        uint8_t buffer[bufferSize];
        uint32_t len = 0;
        while (http.connected() && (len = stream->readBytes(buffer, bufferSize)) > 0) {
            if (file.write(buffer, len) != len) { // Check for write errors
                Serial.println("Error writing to file!");
                file.close();
                http.end();
                return false;
            }
            downloadedBytes += len;
            if (fileSize > 0) { // Only print progress if file size is known
                Serial.printf("Downloaded: %d%%\r", (downloadedBytes * 100) / fileSize);
            }
        }

        file.close();
        http.end();

        if (fileSize > 0 && downloadedBytes != fileSize) {
            Serial.println("\nWarning: Downloaded file size does not match expected size!");
            return false;
        } else {
            Serial.println("\nDownload complete.");
            return true;
        }

        
    } else {
        Serial.printf("HTTP GET failed, error code: %d\n", httpCode);
        http.end();
        return false;
    }
}

    bool LittleFSManager::fileExists(const char* filePath) {
        return LittleFS.exists(filePath);
    }

    void LittleFSManager::listFiles() {
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            Serial.println(file.name());
            file = root.openNextFile();
        }
    }

  File LittleFSManager::openFile(const char* filename, const char* mode) {
    File file = LittleFS.open(filename, mode);
    return file;
  }



