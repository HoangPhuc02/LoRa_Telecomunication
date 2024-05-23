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

#include "FirebaseLib.h"

#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
// void firebaseLib::WiFiEvent(WiFiEvent_t event)
// {

// }


void firebaseLib::streamCallback(StreamData data) {
  if (data.dataType() == "int") {
    Serial.println("Variable updated! New value: " + String(data.intData()));
    // Take action based on the new value
  } else if (data.dataType() == "string") {
    Serial.println("Variable updated! New value: " + data.stringData());
    // Take action based on the new value
  }
  // Handle other data types if needed
}

// // void startListener(const char)
// {
//   if (!Firebase.RTDB.beginStream(fbdo, "/path/to/your/variable")) {
//     Serial.println("Could not begin stream");
//     Serial.println("REASON: " + fbdo.errorReason());
// } else {
//     Serial.println("Stream started!");
//     Firebase.RTDB.setStreamCallback(fbdo, streamCallback);
// }
// // }
void firebaseLib::setupWiFi() {
 

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}
void firebaseLib::setupFirebase(){
    // Initialize Firebase
   /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // The WiFi credentials are required for Pico W
  // due to it does not have reconnect feature.


  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  Firebase.begin(&config, &auth);
  // Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

String firebaseLib::getDownloadURLFromFirebase(const char * filePath) {
  if (Firebase.getString(fbdo, filePath)) {
    return fbdo.stringData();
  } else {
    Serial.println("Error getting download URL from Firebase.");
    return "";
  }
}

// void downloadHeader()
// {

// }

// void 




