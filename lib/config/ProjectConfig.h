#ifndef __PROJECT_CONFIG_H
#define __PROJECT_CONFIG_H




#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include <cstdlib>
#include <string>
#include <vector> 
#include <cstring>
#include <time.h>
/* ================================Project specific===================================== */

// #define BINH_WIFI 
#define UART_REPLACE 

#define PIN_CHECK_STATE 16
#define PIN_CHECK_TIME  17
/* ================================Task RTOS Manager==================================== */



/* OLD DEFINE */
// #define RX_TASK_PRIORITY configMAX_PRIORITIES - 2
// #define TX_TASK_PRIORITY configMAX_PRIORITIES - 3
// #define UPDATE_TASK_LOOP_PRIORITY configMAX_PRIORITIES
// #define FB_MONITOR_VALUE_PRORITY  1
// #define FB_UPLOAD_FIRMWARE_PRIORITY 1
// #define FB_UPLOAD_DATA_PRIORITY configMAX_PRIORITIES - 1,

// /* core*/
// /* UPDATE MANAGER*/
// #define UPDATE_TASK_LOOP_CORE 0

// /*UART MANAGER*/
// #define RX_TASK_CORE 0
// #define TX_TASK_CORE 0

// /* FIREBASE MANAGER*/
// #define FB_MONITOR_VALUE_CORE 1
// #define FB_UPLOAD_FIRMWARE_CORE 1
// #define FB_UPLOAD_DATA_CORE 1

/* NEW DEFINE */

#define RX_TASK_PRIORITY configMAX_PRIORITIES 
#define TX_TASK_PRIORITY configMAX_PRIORITIES - 5
#define UPDATE_TASK_LOOP_PRIORITY configMAX_PRIORITIES - 1
#define FB_MONITOR_VALUE_PRORITY configMAX_PRIORITIES - 4
#define FB_UPLOAD_FIRMWARE_PRIORITY configMAX_PRIORITIES -3
#define FB_UPLOAD_DATA_PRIORITY configMAX_PRIORITIES - 2

/* core*/
/* UPDATE MANAGER*/
#define UPDATE_TASK_LOOP_CORE 0

/*UART MANAGER*/
#define RX_TASK_CORE 0
#define TX_TASK_CORE 0

/* FIREBASE MANAGER*/
#define FB_MONITOR_VALUE_CORE 1
#define FB_UPLOAD_FIRMWARE_CORE 1
#define FB_UPLOAD_DATA_CORE 1
/* =================================UART Manager config================================= */


#ifdef UART_REPLACE
#define UART UART_NUM_1
#define UART_REPLACE UART_NUM_0
#define TXD_PIN 1
#define RXD_PIN 3

#define TXD0_PIN 5
#define RXD0_PIN 18


#else 
#define UART UART_NUM_1
#define TXD_PIN 4
#define RXD_PIN 2

#endif


#define RX_BUF_SIZE 1024
#define TX_BUF_SIZE 1024



//     #define UART UART_NUM_0

// #elif UART_TRANSFER_1
//     #define UART UART_NUM_1

// #elif UART_TRANSFER_2
//     #define UART UART_NUM_2

// #else 
//     #define UART UART_NUM_1

#define CHECK_HEX 1
#define DATA_SENSOR_RECEIVE 0


#define TIME_WAIT_RECEIVE_ACCPET_UART 5000 //ms


#define ENABLE_TIME_OUT 1
#define DISABLE_TIME_OUT 0

#define RECEIVE_TIME_OUT 0
#define RECEIVE_SUCCESS 1

/* ===================================================================================== */


/* =================================Wifi Manager config================================= */


#ifdef BINH_WIFI
    #define WIFI_SSID "MI 8 Pro"
    #define WIFI_PASSWORD "123456789"
#else
    #define WIFI_SSID "PP"
    #define WIFI_PASSWORD "phucphuc"

    // #define WIFI_SSID "SIX TRET"
    // #define WIFI_PASSWORD "chaokhub"
#endif

#define MAX_CONNECT_TIME 5

#define PARAM_INPUT_1  "ssid"
#define PARAM_INPUT_2  "pass"
#define PARAM_INPUT_3  "ip"
#define PARAM_INPUT_4  "gateway"

/* ===================================================================================== */


/* ===============================SPIFFS Mangager config================================ */

#define localFilePath       "/firmware.bin"
#define headerFilePath      "/header.txt"
#define storeStateFilePath  "/state.txt"
#define ssidPath            "/ssid.txt"
#define passPath            "/pass.txt"
#define ipPath              "/ip.txt"
#define gatewayPath         "/gateway.txt"


/* ===============================Firtbase Mangager config============================== */


// The API key can be obtained from Firebase console > Project Overview > Project settings.
#define API_KEY "AIzaSyDgReLUPXPzDwMjOO5U5Y66RuTCFwUBJR0"

// User Email and password that already registerd or added in your project.
#define USER_EMAIL "phuchocnhung@gmail.com"
#define USER_PASSWORD "phucphuc"
#define DATABASE_URL "https://aquasys-e55d4-default-rtdb.firebaseio.com/"



#define VARIABLE_PATH "/Firmware/update_status"
// File path on the firebase
#define firebaseFilePath "/Firmware/URL"
#define headerFirebaseFilePath "/Firmware"
#define sensorDataFirebaseFilePath "/DataSensor/"

#define FB_TIME_BEGIN_OTA "/Firmware/TimeBeginOTA"
#define FB_TIME_FINISH_OTA "/Firmware/TimeFinishOTA"
#define WIFI_TIME_CHECK "/DataSensor/TimeUpdate"
#define HEADER_SIZE 20


#define START_UPDATE_FB_STATUS "true"
#define NOT_UPDATE_FB_STATUS "false"
#define FINISH_UPDATE_FB_STATUS "done"


/*DEVICE_ID_SIZE | DEVICE_SENSOR_DATA_FRAME_SIZE | DEVICE_SENSOR_DATA_FRAME_SIZE| DEVICE_SENSOR_DATA_FRAME_SIZE*/
#define DEVICE_ID_SIZE 4
#define DEVICE_SENSOR_DATA_FRAME_SIZE 3
#define DEVICE_SENSOR_NUMBER_DEFAULT 4


#define NTP_SERVER              "pool.ntp.org"
#define GMT_OFFSET_SEC          25200 // + 7 (vn ) * 60 * 60 => SEC
#define DAY_LIGHT_OFFSET_SEC    0

#define MAX_RETRIES_DOWNLOAD_FW 3
/* ===================================================================================== */

/* ===============================Update Mangager config=============================== */
#define BUFFER_SIZE 1024

#define UART_BAUD_RATE 115200  // Baud rate for UART communication


#define STATE_IDLE                  0xFFU
#define NEW_UPDATE_REQUEST	  		"NEW_UPDATE_REQUEST	"
#define NEW_UPDATE_REQUEST_MODE	  		0x01U

/* ERROR HANDLE MESSEGE GATEWAY TO ESP*/
#define SYSTEM_STATE_UNDEFINE 		0x02U
#define GATEWAY_BUSY		  		0x03U
#define INVALID_REQUEST 	  		0x04U
/* Sequence GATEWAY with ESP*/
#define NEW_UPDATE_REQUEST_ACCEPT 	"NEW_UPDATE_REQUEST_ACCEPT"
#define NEW_UPDATE_REQUEST_ACCEPT_MODE 	0x05U

#define NEW_UPDATE_REQUEST_DENY		"NEW_UPDATE_REQUEST_DENY"
#define NEW_UPDATE_REQUEST_DENY_MODE		0x06U
#define ESP_SEND_HEADER_FLAG  		0x07U
#define HEADER_FLAG_RECEIVED        "HEADER_FLAG_RECEIVED"
#define HEADER_FLAG_RECEIVED_MODE  		0x08U
#define HEADER_FLAG_INVALID  		0x09U
#define HEADER_FLAG_FW_INFO			0x0CU

#define MASTER_ACCEPT_PACKET 		"MASTER_ACCEPT_PACKET"
#define MASTER_ACCEPT_PACKET_MODE 		0x0BU

#define MASTER_RECEIVE_ALL 		"MASTER_RECEIVE_ALL"
#define MASTER_RECEIVE_ALL_MODE        0X0CU

#define ESP_SEND_NEXT_PACKET  		"ESP_SEND_NEXT_PACKET"
#define ESP_SEND_NEXT_PACKET_MODE  		0x0AU


#define SET_DONE_OTA_MODE           0x7AU
#define DONE_OTA_MODE               0x7BU
#define FAIL_OTA_MODE               0xFBU
/*CONFIRM DOWNLOADING DONE*/
#define ESP_DOWNLOAD_DONE    		"ESP_DOWNLOAD_DONE"
#define PACKET_1024bytes	 		1024U
#define PACKET_255bytes				255U
#define PACKET_112bytes				112U
#define PACKET_64bytes				64U
#define HEADER_CONFIG_SIZE 	 		16U


/*WIFI */
#define WiFI_CONNECT_LOSS           0xF0
#define WiFI_CONNECT_SUCCESS        0xF1
/* ===================================================================================== */
#endif

