#include <UARTManager.h>


QueueHandle_t UARTHandler::uart_queue = xQueueCreate(10, sizeof(uart_event_t));


UARTHandler::UARTHandler() {
    // Constructor code if needed
}

void UARTHandler::init() {
    uart_config_t config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(UART, &config);
    uart_set_pin(UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart_queue, 0);

    // Create a task for handling UART events
    xTaskCreatePinnedToCore(uartTask, "uartTask", 2048, NULL, 5, NULL, 0);
}

void UARTHandler::uartTask(void *pvParameters) {
    uart_event_t event;
    size_t len;
    uint8_t* data = (uint8_t*) malloc(BUF_SIZE);

    for (;;) {
        if (xQueueReceive(uart_queue, (void*)&event, (portTickType)portMAX_DELAY)) {
            switch(event.type) {
                case UART_DATA:
                    len = uart_read_bytes(UART, data, event.size, portMAX_DELAY);
                    Serial.write(data,len);
                    // Handle received data here
                    break;

                case UART_DATA_BREAK:
                    Serial.println("Transmit done");
                    break;

                default:
                    break;
            }
        }
    }

    free(data);
    vTaskDelete(NULL);
}

void UARTHandler::transmit(const uint8_t* data, size_t len) {
    uart_write_bytes(UART_NUM_1, (const char*)data, len);
}



// ... (Include UART.h and other necessary headers)

UARTManager::UARTManager(uart_port_t uart_num, int baud_rate, int rx_buffer_size, int tx_buffer_size)
    : uart_num(uart_num)
    , uart_queue(nullptr)
    , tx_queue(nullptr) {
    const uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_APB,
    };
    uart_driver_install(uart_num, rx_buffer_size, tx_buffer_size, 20, &uart_queue, 0);
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    #ifdef UART_REPLACE
    uart_param_config(UART_REPLACE, &uart_config);
    uart_set_pin(UART_REPLACE, TXD0_PIN, RXD0_PIN,  UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    #endif
}

UARTManager::~UARTManager() {
    // Cleanup (if needed, e.g., delete queues)
}

void UARTManager::init() {
    // uart_enable_pattern_det_baud_intr(uart_num, '+', PATTERN_CHR_NUM, 9, 0, 0);
    uart_pattern_queue_reset(uart_num, 20);
    tx_queue = xQueueCreate(5, sizeof(QueueBufferItem_t)); // Assuming QueueBufferItem_t is defined
    
    xTaskCreatePinnedToCore(rx_task, "uart_rx_task", 4096, this, configMAX_PRIORITIES - 2, &rx_task_handle, 0);
    xTaskCreatePinnedToCore(tx_task, "uart_tx_task", 4096, this, configMAX_PRIORITIES - 3, &tx_task_handle, 0);
}
// ...rest of the code
// ... (Previous code from UARTManager constructor and init)

void UARTManager::tx_task(void* pvParameters) {
    UARTManager* uart_manager = static_cast<UARTManager*>(pvParameters); // Get a reference to the object
    QueueBufferItem_t receivedItem;

    for (;;) {
        // Serial.println("TX TASK");
        if (xQueueReceive(uart_manager->tx_queue, &receivedItem, portMAX_DELAY)) {
            // Serial.println("Transfer command");
            uart_manager->sendData(receivedItem.data, receivedItem.dataLength);
            vTaskDelay(2000 / portTICK_PERIOD_MS); // Adjust delay as needed
        }
    }
    vTaskDelete(NULL);
}

void UARTManager::rx_task(void* pvParameters) {
    UARTManager* uart_manager = static_cast<UARTManager*>(pvParameters); // Get a reference to the object
    uart_event_t event;
    

    for (;;) {
        // Serial.println("RX TASK");
        if (xQueueReceive(uart_manager->uart_queue, (void*)&event, (TickType_t)portMAX_DELAY)) {
            bzero(uart_manager->rxBuffer, RX_BUF_SIZE);
            Serial.printf("uart[%d] event:", uart_manager->uart_num);
            if (event.type == UART_DATA) {
                Serial.printf("[UART DATA]: %d", event.size);
                uart_read_bytes(uart_manager->uart_num, (uint8_t*)uart_manager->rxBuffer, event.size, portMAX_DELAY);
                uart_manager->rxBufferSize = event.size;
                Serial.printf("[DATA EVT]: %s", uart_manager->rxBuffer);
                uart_manager->checkDataFromUart(uart_manager->rxBuffer, event.size); // Assuming commandRX is defined
            }
        }
    }
    vTaskDelete(NULL);
}

int UARTManager::sendData(const uint8_t* data, size_t len) {
    int txBytes = uart_write_bytes(uart_num, (const char*)data, len);
    // Serial.printf("Wrote %d bytes: %s", txBytes, data);
    return txBytes;
}
bool UARTManager::waitForCommandHex(uint8_t hexCommand,  uint8_t time_out_enable, uint32_t time_out)
{
    command[0] = hexCommand;
    //Serial.println(command);
    isReceveWrongCommand = false;
    if(time_out_enable = ENABLE_TIME_OUT)
    {
        TickType_t startTime, endTime;
        startTime = xTaskGetTickCount();

        while (isReceveWrongCommand == false ) {
            endTime = xTaskGetTickCount();
            if(endTime - startTime >= time_out)
                return RECEIVE_TIME_OUT;
            vTaskDelay(10);
        }
        return RECEIVE_SUCCESS;
    }
    while (isReceveWrongCommand == false ) {
        vTaskDelay(10);
    }
    return RECEIVE_SUCCESS;


}

bool UARTManager::updateDataFromUart(char* buffer, uint16_t size, uint8_t* command_rx) {
    // Implementation for handling received data
    // ... Parse and process the data based on the command
    // Return true if data was successfully processed, false otherwise
    return true; // Replace with your actual implementation
}

/*
if check is equal to string need to modify this function
*/
bool UARTManager::checkDataFromUart(char* buffer, uint16_t size) {

    if(size == 1)
    {
        if( buffer[0] == command[0])
            isReceveWrongCommand = true;
        else 
            isReceveWrongCommand = false;

        return CHECK_HEX;
    }
    updateDatatoFB = true;
    return DATA_SENSOR_RECEIVE;
    
}