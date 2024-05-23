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


bool setupSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Error mounting SPIFFS");
    return false;
  }
  
  // Print SPIFFS info for debugging
  Serial.println("SPIFFS totalBytes: " + String(SPIFFS.totalBytes()));
  Serial.println("SPIFFS usedBytes: " + String(SPIFFS.usedBytes()));
  return true;
}



bool downloadFile(const String &downloadURL, const char *filename, int &len) {
  HTTPClient http;
  http.begin(downloadURL);
  int httpCode = http.GET();
  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);

  if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
      len = http.getSize();
      Serial.print("File Size: ");
      Serial.println(len);

      File file = SPIFFS.open(filename, FILE_WRITE);
      if (!file) {
        Serial.println("Error opening file for writing");
        return false;
      }

      
      WiFiClient* stream = http.getStreamPtr();
      while (http.connected() && !stream->available()) delay(1);

      while (stream->available()) {
        file.write(stream->read());
      }
      file.close();
      http.end();
      return true;
  } else {
      // Serial.print("Error in downloading file: ");
      Serial.println(httpCode);
      http.end();
      return false;
    }
    Serial.println("Error downloading file: " + http.errorToString(httpCode));
    return false;
  } 
  
    
    void SPIFFSManager::begin()
    {
        if (!SPIFFS.begin(true)) {
            Serial.println("ERROR: Failed to mount SPIFFS");
            _initialized = false;
        } else {
            Serial.println("SPIFFS mounted successfully.");
            _initialized = true;

            // Log SPIFFS information

            // Serial.println("Total space:      " + String(SPIFFS.totalBytes));
            // Serial.println("Used space:       " + String(SPIFFS.usedBytes));
            // Serial.println("Block size:       " + String(SPIFFS.blockSize));
            // Serial.println("Page size:        " + String(SPIFFS.pageSize));
            // Serial.println("Maximum open files: " + String(SPIFFS.maxOpenFiles));
            // Serial.println("Maximum path length: " + String(SPIFFS.maxPathLength));
        }
    }
    bool SPIFFSManager::isInitialized() {
        return _initialized;
    }

    bool SPIFFSManager::writeFile(const String &filePath, const uint8_t *data, size_t length) {
        if (!_initialized) {
            Serial.println("ERROR: SPIFFS not initialized.");
            return false;
        }

        File file = SPIFFS.open(filePath, FILE_WRITE);
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

    String SPIFFSManager::readFile(const String &filePath) {
        if (!_initialized) {
            Serial.println("ERROR: SPIFFS not initialized.");
            return "";
        }

        if (!SPIFFS.exists(filePath)) {
            Serial.println("ERROR: File does not exist: " + filePath);
            return "";
        }

        File file = SPIFFS.open(filePath, FILE_READ);
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
/*
    bool SPIFFSManager::downloadFile(const String &downloadURL, const char *filename) {
        if (!_initialized) {
            Serial.println("ERROR: SPIFFS not initialized.");
            return false;
        }

        HTTPClient http;
        if (!http.begin(downloadURL)) {
            Serial.println("ERROR: Unable to begin HTTP request");
            return false;
        }

        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            File file = SPIFFS.open(filename, FILE_WRITE);
            if (!file) {
                Serial.println("ERROR: Failed to open file for writing: " + String(filename));
                http.end();
                return false;
            }

            // Get file length from header (if available)
            int fileLength = http.getSize();
            Serial.println("Downloading file of size: " + String(fileLength) + " bytes");

            // Download and write to SPIFFS in chunks
            WiFiClient *stream = http.getStreamPtr();
            std::vector<uint8_t> buffer(128);  // Using std::vector for buffer
            int totalBytesRead = 0;
            while (http.connected() && (totalBytesRead < fileLength || fileLength == -1)) {
                size_t size = stream->available();
                if (size) {
                    int c = stream->readBytes(buffer.data(), min(size, buffer.size()));
                    totalBytesRead += c;
                    if (file.write(buffer.data(), c) != c) {
                        Serial.println("ERROR: Failed to write data to file");
                        file.close();
                        http.end();
                        return false;
                    }
                }
            }
            Serial.println("Downloaded and saved: " + String(filename) + " (" + String(totalBytesRead) + " bytes)");
            return true;
        } else {
            Serial.println("ERROR: HTTP GET failed, error code: " + String(httpCode));
        }
        http.end();
        return false; 
    }
*/
  bool SPIFFSManager::downloadFile(const String &downloadURL, const char *filename) {

    HTTPClient http;
    http.begin(downloadURL);
    int httpCode = http.GET();
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);

    if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
        // len = http.getSize();
        Serial.print("File Size: " + String(http.getSize()));
        // Serial.println(len);

        File file = SPIFFS.open(filename, FILE_WRITE);
        if (!file) {
          Serial.println("Error opening file for writing");
          return false;
        }

        
        WiFiClient* stream = http.getStreamPtr();
        while (http.connected() && !stream->available()) delay(1);

        while (stream->available()) {
          file.write(stream->read());
        }
        
        file.close();
        http.end();
        return true;
    } else {
        // Serial.print("Error in downloading file: ");
        Serial.println(httpCode);
        http.end();
        return false;
      }
      Serial.println("Error downloading file: " + http.errorToString(httpCode));
      return false;
    } 
    bool SPIFFSManager::fileExists(const char* filePath) {
        return SPIFFS.exists(filePath);
    }

    void SPIFFSManager::listFiles() {
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        while (file) {
            Serial.println(file.name());
            file = root.openNextFile();
        }
    }

  File SPIFFSManager::openFile(const char* filename, const char* mode) {
    File file = SPIFFS.open(filename, mode);
    return file;
  }
//    void SPIFFSManager::closeFile(File& file) {
//         file.close();



