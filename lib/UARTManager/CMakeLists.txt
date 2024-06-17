

# if (CONFIG_I2C_MANAGER_I2CDEV)
#     list(APPEND SOURCES "i2cdev/i2cdev.c")
#     list(APPEND INCLUDES "i2cdev")
# endif()

idf_component_register(
    SRCS "UARTManager.cpp" "UARTManagerv2.cpp" "UART.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver
)