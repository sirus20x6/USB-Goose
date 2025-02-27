# Toolchain file for ESP32-C6 (RISC-V)
# This file configures CMake to use the RISC-V LLVM toolchain for ESP32-C6

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv32)

# Check for ESP-IDF environment
if(DEFINED ENV{IDF_PATH})
    set(ESP_IDF_PATH $ENV{IDF_PATH})
else()
    message(FATAL_ERROR "ESP-IDF not found. Please set IDF_PATH environment variable.")
endif()

# Specify the LLVM toolchain executables via ESP-IDF
set(LLVM_RISCV_ROOT "${ESP_IDF_PATH}/tools/tools/riscv32-esp-elf-llvm/bin")

set(CMAKE_C_COMPILER "${LLVM_RISCV_ROOT}/clang")
set(CMAKE_CXX_COMPILER "${LLVM_RISCV_ROOT}/clang++")
set(CMAKE_ASM_COMPILER "${LLVM_RISCV_ROOT}/clang")
set(CMAKE_AR "${LLVM_RISCV_ROOT}/llvm-ar")
set(CMAKE_RANLIB "${LLVM_RISCV_ROOT}/llvm-ranlib")
set(CMAKE_OBJCOPY "${LLVM_RISCV_ROOT}/llvm-objcopy")
set(CMAKE_OBJDUMP "${LLVM_RISCV_ROOT}/llvm-objdump")
set(CMAKE_SIZE "${LLVM_RISCV_ROOT}/llvm-size")
set(CMAKE_STRIP "${LLVM_RISCV_ROOT}/llvm-strip")

# ESP32-C6 specific target triple and CPU
set(RISCV_TARGET_TRIPLE "riscv32-unknown-none-elf")
set(RISCV_CPU "rv32imc")
set(RISCV_ABI "ilp32")

# Include ESP-IDF specific paths
include_directories(
    ${ESP_IDF_PATH}/components/esp_common/include
    ${ESP_IDF_PATH}/components/esp_wifi/include
    ${ESP_IDF_PATH}/components/esp_event/include
    ${ESP_IDF_PATH}/components/esp_system/include
    ${ESP_IDF_PATH}/components/esp32c6/include
    ${ESP_IDF_PATH}/components/freertos/FreeRTOS-Kernel/include
    ${ESP_IDF_PATH}/components/driver/include
    ${ESP_IDF_PATH}/components/spi_flash/include
    ${ESP_IDF_PATH}/components/hal/include
    ${ESP_IDF_PATH}/components/log/include
)

# Compiler flags for RISC-V ESP32-C6
set(COMMON_FLAGS "-target ${RISCV_TARGET_TRIPLE} -mcpu=${RISCV_CPU} -mabi=${RISCV_ABI}")
set(COMMON_FLAGS "${COMMON_FLAGS} -march=rv32imc")
set(COMMON_FLAGS "${COMMON_FLAGS} -ffunction-sections -fdata-sections")
set(COMMON_FLAGS "${COMMON_FLAGS} -DESP32C6 -DIDF_VER=\"v5.0\"")

# ESP32-C6 specific optimizations and flags
set(CMAKE_C_FLAGS_INIT "${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${COMMON_FLAGS} -fno-rtti -fno-exceptions")
set(CMAKE_ASM_FLAGS_INIT "${COMMON_FLAGS}")

# Debug flags with optimizations for code size
set(CMAKE_C_FLAGS_DEBUG "-Og -g")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")

# Release flags with high optimizations
set(CMAKE_C_FLAGS_RELEASE "-Os -flto")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -flto")

# Linker flags for ESP32-C6
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--gc-sections -Wl,-Map=output.map")

# Skip compiler check - cross compiling makes it difficult
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# Avoid testing compiler for cross-builds
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Define additional tools for flashing
set(ESP_FLASH_TOOL "${ESP_IDF_PATH}/tools/tools/esptool.py" CACHE STRING "Path to ESP32 flash tool")