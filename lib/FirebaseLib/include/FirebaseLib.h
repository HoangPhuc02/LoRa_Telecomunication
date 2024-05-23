#ifndef __FIREBASE_LIB_H
#define __FIREBASE_LIB_H
/** UART config*/
/*Pin */
#include <Arduino.h>
#include <WiFi.h>
#include <String.h>
#include <SPIFFS_FB.h>
// #include <FirebaseClient.h>

#include <FirebaseESP32.h>
#include <HTTPClient.h>




#define WIFI_SSID "PP"
#define WIFI_PASSWORD "phucphuc"

// The API key can be obtained from Firebase console > Project Overview > Project settings.
#define API_KEY "AIzaSyDgReLUPXPzDwMjOO5U5Y66RuTCFwUBJR0"

// User Email and password that already registerd or added in your project.
#define USER_EMAIL "phuchocnhung@gmail.com"
#define USER_PASSWORD "phucphuc"
#define DATABASE_URL "https://aquasys-e55d4-default-rtdb.firebaseio.com/"

// File path on the firebase
const char* firebaseFilePath = "/Firmware/BlinkLed/URL";

// File path on the spiffs
const char* localFilePath = "/firmware.bin";

class firebaseLib{
    private :
        FirebaseAuth auth;
        FirebaseConfig config;

        // Firebase Data object
        FirebaseData fbdo;

         /* callback function*/
        // Check wifi is still connecting
        // void WiFiEvent(WiFiEvent_t event) ;

        // Check when data is update
        void streamCallback(StreamData data) ;
    public:
        /* flag*/
        bool wifiConnected ;
        bool listenerAvailable;

        /* constructor*/
    
        /* Setup */
        void setupWiFi() ;
        void setupFirebase();

        String getDownloadURLFromFirebase(const char * filePath);
        /*Read and transfer to FB*/

       
        /* RTOS task*/
        void FirebaseLoopTask();
        
    

        

};





#endif