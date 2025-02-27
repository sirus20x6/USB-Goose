cmake_minimum_required(VERSION 3.16)
project(ESP32C6Firmware VERSION 0.1.0 LANGUAGES C CXX ASM)

# Find common library
if(DEFINED PROJECT_ROOT)
    # When building from superbuild, include from project root
    set(COMMON_DIR "${PROJECT_ROOT}/common")
    include_directories(${COMMON_DIR}/include)
else()
    # When building standalone, use find_package
    find_package(DuckyCommon REQUIRED)
endif()

# ESP-IDF specific settings
set(SDKCONFIG_DEFAULTS "${CMAKE_CURRENT_SOURCE_DIR}/sdkconfig.defaults")
set(SDKCONFIG "${CMAKE_CURRENT_BINARY_DIR}/sdkconfig")

# Include ESP-IDF build system - integrate with ESP-IDF's own build system
include("$ENV{IDF_PATH}/tools/cmake/project.cmake")

# Define source files for our application component
set(ESP32C6_SOURCES
    src/main.c
    src/wireless/wifi.c
    src/wireless/ble.c
    src/wireless/thread.c
    src/comm/spi_master.c
    src/web/server.c
    src/web/api.c
    src/recon/scanner.c
    src/recon/analyzer.c
    src/security/crypto.c
    src/security/secure_storage.c
    src/esp32c6_os_detect.c
)

# Create the main component
idf_component_register(
    SRCS ${ESP32C6_SOURCES}
    INCLUDE_DIRS 
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/config"
    REQUIRES 
        driver 
        esp_wifi 
        esp_system 
        bt 
        nvs_flash 
        esp_http_server
        mbedtls
        spi_flash
)

# If building standalone (not from superbuild), we need to create the common library
if(NOT DEFINED PROJECT_ROOT)
    # Create a static library from the common code
    add_library(esp32c6_common STATIC
        ${DuckyCommon_SOURCE_DIR}/src/protocol.c
        ${DuckyCommon_SOURCE_DIR}/src/security.c
        ${DuckyCommon_SOURCE_DIR}/src/payload_framework.c
    )
    
    target_include_directories(esp32c6_common PUBLIC
        ${DuckyCommon_INCLUDE_DIRS}
    )
    
    # Link against the main component
    target_link_libraries(${COMPONENT_LIB} PRIVATE esp32c6_common)
else()
    # When building from superbuild, link against the already built common library
    if(EXISTS "${PROJECT_ROOT}/common/build/libducky_common.a")
        target_link_libraries(${COMPONENT_LIB} PRIVATE "${PROJECT_ROOT}/common/build/libducky_common.a")
    else()
        message(WARNING "Common library not found at ${PROJECT_ROOT}/common/build/libducky_common.a")
        # Create a stub for build to proceed
        add_library(ducky_common STATIC ${COMMON_DIR}/src/protocol.c)
        target_link_libraries(${COMPONENT_LIB} PRIVATE ducky_common)
    endif()
endif()

# ESP32-C6 specific definitions
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    ESP32C6
    ESP_PLATFORM
)

# Add flash target that leverages ESP-IDF's flash command
add_custom_target(flash_esp32c6
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target flash
    COMMENT "Flashing ESP32-C6 firmware"
)

# Set default port for flashing if not specified
if(NOT DEFINED ESP32C6_PORT)
    if(WIN32)
        set(ESP32C6_PORT "COM4" CACHE STRING "Serial port for ESP32-C6 flashing")
    else()
        set(ESP32C6_PORT "/dev/ttyUSB1" CACHE STRING "Serial port for ESP32-C6 flashing") 
    endif()
endif()

# Set port for ESP-IDF
set(ESPPORT ${ESP32C6_PORT})
set(ESPBAUD 921600)

# Generate compile_commands.json for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)