# USB Goose: USB 3.0 and Multi-Protocol Wireless Pentesting Tool Inspired By The USB Rubber Ducky

## Project Overview

This project leverages a powerful **dual-chip architecture** combining the **WCH CH569** (RISC-V with USB 3.0) and **ESP32-C6** (RISC-V with multi-protocol wireless) to create the most advanced USB Rubber Ducky implementation available, featuring SuperSpeed USB 3.0, comprehensive wireless capabilities, and a unified LLVM development framework.

## Architecture Overview

### Dual-Chip Design
- **WCH CH569**: RISC-V USB 3.0 controller (5 Gbps SuperSpeed)
- **ESP32-C6**: RISC-V wireless controller (Wi-Fi 6, BLE 5.0, 802.15.4)
- **High-speed SPI bridge** for inter-chip communication
- **Shared memory buffer** for coordinated operations

### Chip Responsibilities

#### WCH CH569 (USB Controller)
- **Primary USB Interface**:
  - USB 3.0 SuperSpeed HID (keyboard/mouse)
  - USB 3.0 Mass Storage emulation
  - USB Network Interface (NIC) for high-speed communication
  - U2F/FIDO2 security key emulation
- **Storage Management**:
  - SD card interfacing via SPI
  - Encrypted partition handling
  - High-speed data transfer
- **Payload Execution**:
  - ELF loading for USB-based payloads
  - Timing-critical operations
  - System initialization and boot sequence

#### ESP32-C6 (Wireless Controller)
- **Wireless Protocols**:
  - Wi-Fi 6 (802.11ax) with AP mode
  - Bluetooth 5.0 LE for control and proximity
  - IEEE 802.15.4 for Thread/Zigbee/Matter
- **Network Operations**:
  - Wireless reconnaissance
  - Remote payload delivery
  - Data exfiltration
  - Web configuration interface
- **Security Operations**:
  - Hardware-accelerated encryption
  - Secure key storage
  - Authentication handling

## Key Features

### ✅ Intelligent Multi-Vector Communication System
- **Adaptive Channel Selection**: Automatically selects the best communication method based on speed, stealth, or reliability requirements
- **Automatic Fallbacks**: Gracefully degrades to slower but more reliable channels when needed
- **Channel Coordination**: Maintains connection state and performance metrics across all channels
- **Parallel Operation**: Utilizes multiple channels simultaneously for maximum throughput
- **USB 3.0 NIC Foothold**: Establishes high-speed network connection via USB for superior performance

### ✅ Comprehensive Exfiltration System
- **Multi-Priority Exfiltration**: Configurable priorities (speed, stealth, reliability)
- **Chunked Data Transfers**: Handles large data sets efficiently
- **Parallel Exfiltration**: Exfiltrates via multiple channels simultaneously
- **Progress Tracking**: Monitors exfiltration status across channels
- **Automatic Recovery**: Continues exfiltration after disruptions or channel failures

### ✅ SuperSpeed USB 3.0
- **High-Bandwidth Data Transfer**: Significantly faster than USB 2.0
- **Rapid Exfiltration**: Move large datasets quickly
- **Enhanced Storage**: Function as a high-performance flash drive

### ✅ Comprehensive Wireless Capabilities
- **Wi-Fi 6**: Latest standard with improved performance
- **Bluetooth LE**: Low-energy proximity and control
- **802.15.4**: For mesh networking and IoT integration
- **Simultaneous Operation**: Run multiple protocols concurrently

### ✅ Unified LLVM Development Framework
- **Language Flexibility**: C, C++, Rust, Python, and Nim (all LLVM languages)
- **Cross-Chip Framework**: Same languages for both processors
- **Shared Codebase**: Common libraries across the system
- **Modern Practices**: Use contemporary language features

### ✅ Modular Payload System
- **Multi-Vector Payloads**: Combine USB and wireless attack paths
- **Conditional Execution**: Adapt based on detected environment
- **Coordinated Operations**: Synchronized actions across chips
- **Event-Driven Architecture**: React to system and environmental triggers

### ✅ Enhanced Hardware Features
- **RGBW Status LEDs**: Visual feedback on device status
- **IR Transceiver**: Additional communication channel
- **Real-Time Clock**: Time-based operations and persistence
- **Hardware Security Elements**: Secure key storage on both chips

## Hardware Requirements

- **WCH CH569** RISC-V microcontroller (USB 3.0)
- **ESP32-C6** RISC-V wireless microcontroller
- **MicroSD Card Module** (SPI)
- **USB-C Connector** for USB 3.0 connectivity
- **Antennas** for Wi-Fi, BLE, and 802.15.4
- **RGBW LED** for status indication
- **IR Transceiver**
- **RTC Module** with battery backup
- **Push Button** for mode selection
- **Hardware Security Element**

## Software Components

### System Layer
- **Dual-Core Operating System**: Coordinated RTOS across both chips
- **Inter-Chip Communication Protocol**: Optimized for latency and throughput
- **Unified Hardware Abstraction Layer**: Consistent API across both chips

### Communication Framework
- **Channel Management System**: Coordinates all communication methods
- **Priority-Based Selection**: Automatically selects best available channel
- **Channel Metrics**: Tracks relative speed, reliability, and stealth ratings
- **Dynamic Adaptation**: Adjusts to changing conditions and connectivity

### Exfiltration System
- **Multi-Channel Exfiltration**: Coordinates data extraction across channels
- **Chunked Transfer Protocol**: Handles arbitrarily large datasets
- **Priority Options**: Speed, stealth, or reliability optimization
- **Parallel Processing**: Multiple simultaneous exfiltration paths

### USB Capabilities (CH569)
- **USB 3.0 SuperSpeed Stack**: Custom implementation for the CH569
- **HID Profiles**: Advanced keyboard/mouse emulation
- **Mass Storage Controller**: High-performance MSC implementation
- **USB NIC**: Network interface for high-speed communication
- **U2F/FIDO2 Stack**: Security key emulation

### Wireless Capabilities (ESP32-C6)
- **ESP-IDF** with customizations for security operations
- **Multi-Protocol Manager**: Coordinate wireless interfaces
- **AP Mode Configuration**: Web-based device setup
- **Wireless Reconnaissance**: Environment scanning and analysis

### Security Features
- **Hardware-Accelerated Encryption**: AES-256, SHA-256
- **Secure Key Management**: Protected storage
- **Encrypted Payloads**: Obfuscated operation
- **Anti-Tampering Measures**: Detect physical interference

## Code Structure

```
/firmware
 ├── common/                   # Shared code between chips
 │   ├── comm_manager.h        # Communication channel abstraction
 │   ├── comm_manager.c        # Core channel management
 │   ├── exfiltration.h        # Exfiltration system interface
 │   ├── exfiltration.c        # Exfiltration implementation
 │   ├── foothold.h            # Foothold establishment interface
 │   ├── foothold_common.c     # Common foothold code
 │   ├── payload_framework.h   # Core framework API
 │   ├── payload_framework.c   # Framework implementation
 │   ├── protocol.c            # Inter-chip communication protocol
 │   ├── security.c            # Security implementation
 │   ├── comm/                 # Communication modules
 │   │   ├── spi_slave.h       # SPI interface
 │   │   └── spi_slave.c       # SPI communication with ESP32
 │   └── cmake/                # Common CMake modules
 │
 ├── ch569/                    # WCH CH569 USB controller code
 │   ├── ch569_foothold.c      # USB-specific foothold implementation
 │   ├── ch569_hid.c           # HID implementation
 │   ├── os_detect_basic.c     # OS detection implementation
 │   └── CMakeLists.txt        # Build config for CH569
 │
 ├── esp32c6/                  # ESP32-C6 wireless controller code
 │   ├── comm/                 # ESP32 communication
 │   │   └── spi_master.c      # SPI communication with CH569
 │   ├── wireless/             # Wireless functionality
 │   │   └── wifi.c            # Wi-Fi implementation
 │   └── CMakeLists.txt        # Build config for ESP32-C6
 │
 ├── examples/                 # Example payloads
 │   ├── usb_payloads/         # USB-focused payloads
 │   │   └── os_detect_basic.c # C example
 │   ├── wireless_payloads/    # Wireless-focused payloads
 │   │   └── wifi_recon.rs     # Rust example
 │   ├── combined_payloads/    # Multi-vector payloads
 │   │   ├── exfiltration.cpp  # C++ example
 │   │   ├── multi_vector_exfil.c # Multi-vector exfiltration payload
 │   │   └── test_interchip.c  # Inter-chip communication test
 │   ├── python_payloads/      # Python payloads
 │   │   └── os_detect.py      # Python example
 │   ├── nim_payloads/         # Nim payloads
 │   │   └── os_detect.nim     # Nim example
 │   └── CMakeLists.txt        # Build config for examples
 │
 ├── tools/                    # Development tools
 │   ├── nim/                  # Nim compiler tools
 │   │   ├── compile.py        # Nim compiler wrapper
 │   │   └── bindings.c        # C bindings for Nim
 │   ├── pythran/              # Python-to-C compiler tools
 │   │   ├── compile.py        # Pythran compiler wrapper
 │   │   └── bindings.cpp      # C++ bindings for Python
 │   └── test_comm_manager.c   # Communication manager tests
 │
 └── cmake/                    # CMake modules
     ├── ch569_toolchain.cmake # CH569-specific toolchain
     └── esp32c6_toolchain.cmake # ESP32-C6-specific toolchain
```

## Communication Channels

The system supports multiple communication channels, ranked by priority:

1. **USB Network Interface (NIC)**: Highest priority, high-bandwidth bidirectional communication
2. **USB 3.0 Mass Storage**: High priority, very high bandwidth for data transfer
3. **Wi-Fi**: High-medium priority, moderate wireless bandwidth with excellent stealth
4. **Bluetooth**: Medium priority, lower wireless bandwidth with good stealth
5. **HID Raw**: Medium-low priority, limited bandwidth but good compatibility
6. **Mouse**: Low priority, very low bandwidth but somewhat stealthy
7. **Keyboard**: Lowest priority, very low bandwidth but universally available fallback

Each channel is assigned characteristics:
- **Priority**: Lower number = higher priority
- **Relative Speed**: Bandwidth ranking
- **Stealth Rating**: 0-10 (higher = stealthier)
- **Reliability**: 0-10 (higher = more reliable)
- **Bidirectional**: Whether channel supports two-way communication

## Unified LLVM Development

### Building Payloads
All payloads can be built using standard LLVM tools:

```sh
# For CH569 (USB controller)
clang -target riscv32-unknown-none-elf -mcpu=rv32imac -O2 \
    -I../common -o usb_payload.elf usb_payload.c

# For ESP32-C6 (Wireless controller)
clang -target riscv32-unknown-none-elf -mcpu=rv32imac -O2 \
    -I../common -o wireless_payload.elf wireless_payload.c
```

### Multi-Vector Payload Example

```c
// multi_vector_exfil.c
#include "payload_framework.h"
#include "comm_manager.h"
#include "exfiltration.h"

PAYLOAD_INFO("Multi-Vector Exfiltration", "1.0", PAYLOAD_PRIORITY_HIGH);

void payload_main(PayloadContext* ctx) {
    // Initialize status LED
    StatusLED.setColor(255, 255, 0, 0); // Yellow - starting
    
    // Initialize communication and exfiltration systems
    comm_manager_init();
    exfil_init();
    
    // First try to establish NIC connection (highest bandwidth)
    System.log("Attempting to establish NIC foothold...");
    bool nic_success = establish_nic_foothold(ctx);
    
    if (nic_success) {
        StatusLED.setColor(0, 255, 0, 0); // Green - NIC established
        System.log("NIC foothold established successfully!");
    } else {
        StatusLED.setColor(0, 0, 255, 0); // Blue - using fallback
        System.log("Using keyboard fallback for primary communication");
    }
    
    // Initialize Mass Storage for data exfiltration
    System.log("Initializing Mass Storage for exfiltration...");
    if (mass_storage_init_full()) {
        comm_channels[COMM_CHANNEL_MASS_STORAGE].status = CHANNEL_STATUS_CONNECTED;
        System.log("Mass Storage ready for exfiltration");
    }
    
    // Initialize wireless channels if available
    if (System.isWirelessController() || ChipComm.isConnected()) {
        // Initialize Wi-Fi and Bluetooth
        // ...
    }
    
    // Collect data to exfiltrate
    System.log("Collecting target data...");
    size_t data_size = 0;
    uint8_t* data = collect_target_data(&data_size);
    
    // Queue for exfiltration with different priorities
    exfil_queue_data("target_data", data, data_size, 
                    EXFIL_PRIORITY_SPEED, true);
    
    // Process exfiltration until complete
    while (!exfil_is_complete("target_data")) {
        exfil_process_queue();
        System.delay(100);
    }
    
    // Clean up
    free(data);
    StatusLED.off();
}
```

## Exfiltration Priority Options

Data can be exfiltrated with different priorities:

1. **EXFIL_PRIORITY_SPEED**: Uses fastest available channels
2. **EXFIL_PRIORITY_STEALTH**: Uses stealthiest channels
3. **EXFIL_PRIORITY_RELIABILITY**: Uses most reliable channels

The system can exfiltrate data through multiple channels simultaneously, with automatic channel selection based on the chosen priority. If a channel fails, the system automatically falls back to alternative channels.

## Foothold Establishment

The system implements a sophisticated foothold establishment process:

1. **Initial USB Connection**: Always available as baseline
2. **NIC Foothold Attempt**: Executes OS-specific scripts to configure networking
3. **Verification Process**: Confirms successful connectivity
4. **Fallback Chain**: If NIC fails, falls back through other available channels

OS-specific scripts are tailored for Windows, macOS, and Linux to ensure maximum compatibility.

## Python and Nim Payload Support

The project supports writing payloads in Python (via Pythran) and Nim - both compile to native code for optimal performance.

### Python Payload Example

```python
# os_detect.py - Python payload for OS detection
#pythran export payload_main(int, int, int, str, str, int)

# These constants match the C API
OS_UNKNOWN = 0
OS_WINDOWS = 1
OS_MACOS = 2
OS_LINUX = 3

def payload_main(detected_os, security_level, target_capabilities, hostname, username, timestamp):
    # Set LED to yellow during initialization
    led_set_color(255, 255, 0, 0)
    
    # Log which controller we're running on
    if system_is_usb_controller():
        log_message("Running on USB controller")
    else:
        log_message("Running on Wireless controller")
    
    # Different actions based on detected OS
    if detected_os == OS_WINDOWS:
        perform_windows_actions()
    elif detected_os == OS_MACOS:
        perform_macos_actions()
    elif detected_os == OS_LINUX:
        perform_linux_actions()
    else:
        log_message("Unknown or unsupported OS")
    
    # Complete
    led_off()
    return 0
```

### Nim Payload Example

```nim
# os_detect.nim - Nim payload for OS detection
{.passC: "-I../../firmware/common/include".}
{.compile: "../../tools/nim/bindings.c".}

# OS detection constants
const
  OS_UNKNOWN* = 0
  OS_WINDOWS* = 1
  OS_MACOS* = 2
  OS_LINUX* = 3

# Main payload function
proc payload_main*(ctx: ptr PayloadContext): int {.exportc.} =
  # Set LED to yellow during initialization
  led_set_color(255, 255, 0, 0)
  
  # Get current OS from context
  let detectedOs = ctx.detected_os
  
  # Different actions based on detected OS
  case detectedOs:
    of OS_WINDOWS:
      perform_windows_actions()
    of OS_MACOS:
      perform_macos_actions()
    of OS_LINUX:
      perform_linux_actions()
    else:
      log_message("Unknown or unsupported OS")
  
  # Complete
  led_off()
  return 0
```

## Operation Modes

1. **Standard Mode**:
   - Functions as USB HID + Mass Storage
   - Wireless interfaces in passive monitoring
   
2. **Admin Mode** (triggered by button press):
   - Exposes configuration interface via Wi-Fi AP
   - Shows hidden partitions and payload directory
   
3. **Stealth Mode**:
   - Minimal LED activity
   - Covert wireless operation
   - Appears as legitimate USB device

## Build and Flash Instructions

```sh
# Clone repository
git clone https://github.com/yourusername/usb-goose.git
cd usb-goose

# Build firmware for both chips
mkdir build && cd build
cmake ..
make

# Flash CH569
wch-flasher -p /dev/ttyUSB0 -f ch569_firmware.bin

# Flash ESP32-C6
esptool.py --chip esp32c6 --port /dev/ttyUSB1 write_flash 0x0 esp32c6_firmware.bin
```

## Future Enhancements

- **Enhanced foothold techniques** for various environments
- **Additional exfiltration channels** and protocols
- **Encrypted channel communications**
- **AI-powered targeting** using ESP32's ML capabilities
- **Physical penetration hardware extensions**
