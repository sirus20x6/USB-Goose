# Toolchain file for WCH CH569 (RISC-V)
# This file configures CMake to use the RISC-V LLVM toolchain for CH569

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv32)

# Specify the LLVM toolchain executables
# These may need to be adjusted based on your environment
if(DEFINED ENV{RISCV_LLVM_ROOT})
    set(LLVM_RISCV_ROOT $ENV{RISCV_LLVM_ROOT})
else()
    # Try to find LLVM in standard locations
    if(EXISTS "/usr/local/opt/llvm/bin/clang")
        # macOS with Homebrew
        set(LLVM_RISCV_ROOT "/usr/local/opt/llvm")
    elseif(EXISTS "/usr/bin/clang")
        # Linux with LLVM installed
        set(LLVM_RISCV_ROOT "/usr")
    else()
        message(FATAL_ERROR "LLVM toolchain not found. Please set RISCV_LLVM_ROOT environment variable.")
    endif()
endif()

# Check if compiler exists
if(NOT EXISTS "${LLVM_RISCV_ROOT}/bin/clang")
    message(FATAL_ERROR "Clang compiler not found at ${LLVM_RISCV_ROOT}/bin/clang")
endif()

# Set compiler executables
set(CMAKE_C_COMPILER "${LLVM_RISCV_ROOT}/bin/clang")
set(CMAKE_CXX_COMPILER "${LLVM_RISCV_ROOT}/bin/clang++")
set(CMAKE_ASM_COMPILER "${LLVM_RISCV_ROOT}/bin/clang")
set(CMAKE_AR "${LLVM_RISCV_ROOT}/bin/llvm-ar")
set(CMAKE_RANLIB "${LLVM_RISCV_ROOT}/bin/llvm-ranlib")
set(CMAKE_OBJCOPY "${LLVM_RISCV_ROOT}/bin/llvm-objcopy")
set(CMAKE_OBJDUMP "${LLVM_RISCV_ROOT}/bin/llvm-objdump")
set(CMAKE_SIZE "${LLVM_RISCV_ROOT}/bin/llvm-size")
set(CMAKE_STRIP "${LLVM_RISCV_ROOT}/bin/llvm-strip")

# CH569 specific target triple and CPU
set(RISCV_TARGET_TRIPLE "riscv32-unknown-none-elf")
set(RISCV_CPU "rv32imac")
set(RISCV_ABI "ilp32")

# Compiler flags for RISC-V CH569
set(COMMON_FLAGS "-target ${RISCV_TARGET_TRIPLE} -mcpu=${RISCV_CPU} -mabi=${RISCV_ABI}")
set(COMMON_FLAGS "${COMMON_FLAGS} -march=rv32imac")
set(COMMON_FLAGS "${COMMON_FLAGS} -ffunction-sections -fdata-sections")
set(COMMON_FLAGS "${COMMON_FLAGS} -nostdlib -ffreestanding")
set(COMMON_FLAGS "${COMMON_FLAGS} -DCH569 -DRISCV -DUSE_USB3")

# Add support for C++ exceptions and RTTI if needed
# By default, these are turned off for embedded systems
set(CMAKE_CXX_FLAGS_INIT "${COMMON_FLAGS} -fno-rtti -fno-exceptions")

# Check LLVM/Clang version and add appropriate flags
execute_process(
    COMMAND ${CMAKE_C_COMPILER} --version
    OUTPUT_VARIABLE CLANG_VERSION_OUTPUT
)
string(REGEX MATCH "version ([0-9]+)" CLANG_VERSION_MATCH "${CLANG_VERSION_OUTPUT}")
if(CLANG_VERSION_MATCH)
    set(CLANG_VERSION_MAJOR ${CMAKE_MATCH_1})
    # Add version-specific flags
    if(CLANG_VERSION_MAJOR GREATER_EQUAL 10)
        # Modern LLVM/Clang versions may need additional flags
        set(COMMON_FLAGS "${COMMON_FLAGS} -Wno-unused-command-line-argument")
    endif()
endif()

# CH569 specific optimizations and flags
set(CMAKE_C_FLAGS_INIT "${COMMON_FLAGS}")
set(CMAKE_ASM_FLAGS_INIT "${COMMON_FLAGS}")

# Debug flags with optimizations for code size
set(CMAKE_C_FLAGS_DEBUG "-Og -g")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")

# Release flags with high optimizations
set(CMAKE_C_FLAGS_RELEASE "-Os -flto")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -flto")

# Linker flags for CH569
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--gc-sections -Wl,-Map=output.map")

# Skip compiler check - cross compiling makes it difficult
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# Avoid testing compiler for cross-builds
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Define additional tools for flashing
set(WCH_FLASH_TOOL "${CMAKE_CURRENT_LIST_DIR}/../tools/wch-flasher.py" CACHE STRING "Path to WCH flash tool")

# Additional variables for language support
if(ENABLE_RUST)
    # Find rustc and cargo
    find_program(RUSTC_EXECUTABLE rustc)
    find_program(CARGO_EXECUTABLE cargo)
    
    if(RUSTC_EXECUTABLE AND CARGO_EXECUTABLE)
        message(STATUS "Rust toolchain found: ${RUSTC_EXECUTABLE}")
        
        # Check if rust-src component is installed
        execute_process(
            COMMAND rustup component list --installed
            OUTPUT_VARIABLE RUSTUP_COMPONENTS
        )
        
        if(NOT "${RUSTUP_COMPONENTS}" MATCHES "rust-src")
            message(WARNING "Rust source component not found. Please install with 'rustup component add rust-src'")
        endif()
    endif()
endif()