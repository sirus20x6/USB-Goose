cmake_minimum_required(VERSION 3.16)
project(CH569Firmware VERSION 0.1.0 LANGUAGES C CXX ASM)

# Find common library
if(DEFINED PROJECT_ROOT)
    # When building from superbuild, include from project root
    set(COMMON_DIR "${PROJECT_ROOT}/common")
    include_directories(${COMMON_DIR}/include)
else()
    # When building standalone, use find_package
    find_package(DuckyCommon REQUIRED)
endif()

# Define source files
set(CH569_SOURCES
    src/main.c
    src/startup.c
    src/system_ch569.c
    src/ch569_hid.c
    src/ch569_os_detect.c
    src/usb/usb_core.c
    src/usb/usb3_core.c
    src/usb/hid.c
    src/usb/msc.c
    src/usb/u2f.c
    src/storage/sd_card.c
    src/storage/encryption.c
    src/payload/loader.c
    src/payload/executor.c
    src/comm/spi_slave.c
    src/hardware/leds.c
    src/hardware/rtc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/comm/spi_slave.c
)

# Include vendor-specific files if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/ch569/peripheral")
    file(GLOB CH569_VENDOR_SOURCES
        "vendor/ch569/peripheral/*.c"
    )
    list(APPEND CH569_SOURCES ${CH569_VENDOR_SOURCES})
endif()

# Create executable
add_executable(ch569_firmware ${CH569_SOURCES})

# Include directories
target_include_directories(ch569_firmware PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/ch569/include
)

# CH569 specific definitions
target_compile_definitions(ch569_firmware PRIVATE
    CH569
    RISCV
    USE_USB3
)

# If building from superbuild, link against the common library in its build dir
if(DEFINED PROJECT_ROOT)
    if(EXISTS "${PROJECT_ROOT}/common/build/libducky_common.a")
        target_link_libraries(ch569_firmware PRIVATE "${PROJECT_ROOT}/common/build/libducky_common.a")
    else()
        message(WARNING "Common library not found at ${PROJECT_ROOT}/common/build/libducky_common.a")
        # Create a stub for build to proceed
        add_library(ducky_common STATIC ${COMMON_DIR}/src/protocol.c)
        target_include_directories(ducky_common PUBLIC ${COMMON_DIR}/include)
        target_link_libraries(ch569_firmware PRIVATE ducky_common)
    endif()
else()
    # When building standalone, link against the found package
    target_link_libraries(ch569_firmware PRIVATE DuckyCommon::ducky_common)
endif()

# Linker script
set(CH569_LINK_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/ch569_link.ld")
target_link_options(ch569_firmware PRIVATE
    -T${CH569_LINK_SCRIPT}
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map
)

# Generate additional binary formats (hex, bin)
add_custom_command(TARGET ch569_firmware POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:ch569_firmware> ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:ch569_firmware> ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMENT "Creating HEX and BIN files"
)

# Add flash target
find_program(WCH_FLASH_TOOL wch-flasher.py PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../tools" NO_DEFAULT_PATH)
if(NOT WCH_FLASH_TOOL)
    set(WCH_FLASH_TOOL "${CMAKE_CURRENT_SOURCE_DIR}/../tools/wch-flasher.py")
endif()

# Set default port for flashing if not specified
if(NOT DEFINED CH569_PORT)
    if(WIN32)
        set(CH569_PORT "COM3" CACHE STRING "Serial port for CH569 flashing")
    else()
        set(CH569_PORT "/dev/ttyUSB0" CACHE STRING "Serial port for CH569 flashing") 
    endif()
endif()

add_custom_target(flash_ch569
    DEPENDS ch569_firmware
    COMMAND python ${WCH_FLASH_TOOL} -p ${CH569_PORT} -f ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMENT "Flashing CH569 firmware"
)

# Add unit tests if enabled
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# Generate compile_commands.json for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)