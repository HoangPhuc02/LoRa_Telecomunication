#ifndef __UART_MANAGER_H
#define __UART_MANAGER_H

#include <ProjectConfig.h>
#include <Arduino.h>
#include "driver/uart.h"



#include <FirebaseManager.h>



class UARTHandler {
public:
    UARTHandler();
    void init();
    void transmit(const uint8_t* data, size_t len);

private:
    static void uartTask(void *pvParameters);
    static QueueHandle_t uart_queue;
    static const int BUF_SIZE = 1024;
    // static const int TX_PIN = 2; // Example TX pin
    // static const int RX_PIN = 4; // Example RX pin
};


// ... (Include UART.h and other necessary headers)
typedef struct {
    uint8_t data[TX_BUF_SIZE];
    uint16_t dataLength;
}
QueueBufferItem_t;

class UARTManager {
public:
    UARTManager( FirebaseManager& firebaseManager, uart_port_t uart_num = UART,int baud_rate = 115200, int rx_buffer_size = RX_BUF_SIZE * 2, int tx_buffer_size = TX_BUF_SIZE * 2);
    ~UARTManager();

    void init();

    int sendData(const uint8_t* data, size_t len);
    bool updateDataFromUart(char* buffer, uint16_t size,  uint8_t* command_rx);
    bool checkDataFromUart(char* buffer, uint16_t size);
    bool waitForCommandHex(uint8_t hexCommand,uint8_t hexCommand2, uint8_t time_out_enable = ENABLE_TIME_OUT , uint32_t time_out = TIME_WAIT_RECEIVE_ACCPET_UART);
    static void sendDataToUARTTx(char* data, uint32_t size);
    bool isReceveWrongCommand = false;
    char command[16];
    uint8_t hex_command;
    bool updateDatatoFB = false;

    uart_port_t uart_num;
    static QueueHandle_t uart_queue ;
    static QueueHandle_t tx_queue;

    char rxBuffer[RX_BUF_SIZE];
    int32_t rxBufferSize;
private:
    FirebaseManager &firebaseManager;
    

    static void tx_task(void* pvParameters);
    static void rx_task(void* pvParameters);

    // Task handles (for potential future task management)
    TaskHandle_t rx_task_handle;
    TaskHandle_t tx_task_handle;


};





#endif