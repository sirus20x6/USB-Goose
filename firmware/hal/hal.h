/**
 * @file hal.h
 * @brief Main header for USB Goose Hardware Abstraction Layer
 * 
 * This file includes all HAL modules for easy access.
 */

 #ifndef HAL_H
 #define HAL_H
 
 /**
  * @mainpage USB Goose Hardware Abstraction Layer (HAL)
  * 
  * @section overview Overview
  * 
  * The USB Goose HAL provides a clean, platform-independent API for accessing
  * the dual-chip hardware capabilities. It abstracts away the complexity of
  * the underlying hardware implementation and provides a consistent interface
  * for applications.
  * 
  * @section modules Modules
  * 
  * - Core: Basic device operations and configuration
  * - HID: Keyboard and mouse emulation
  * - Storage: File and mass storage operations
  * - Network: Wi-Fi, Bluetooth, and exfiltration
  * - Payload: Script and command execution
  * - Log: Logging and debugging
  * 
  * @section usage Usage Example
  * 
  * ```c
  * #include "hal.h"
  * 
  * int main() {
  *     // Initialize device in standard mode
  *     if (!init(MODE_STANDARD)) {
  *         return 1;
  *     }
  *     
  *     // Initialize HID subsystem
  *     hid_init();
  *     
  *     // Detect operating system
  *     os_type_t os = detect_os();
  *     
  *     // Type a string
  *     type("Hello, World!", 0);
  *     
  *     // Press a key combination (Win+R on Windows, Cmd+Space on macOS)
  *     if (os == OS_WINDOWS) {
  *         key_with_modifiers(KEY_R, false, false, false, true);
  *     } else if (os == OS_MACOS) {
  *         key_with_modifiers(KEY_SPACE, false, false, false, true);
  *     }
  *     
  *     // Clean shutdown
  *     shutdown();
  *     return 0;
  * }
  * ```
  */
 
 /* Include all module headers */
 #include "hal/core.h"
 #include "hal/hid.h"
 #include "hal/storage.h"
 #include "hal/network.h"
 #include "hal/payload.h"
 #include "hal/log.h"
 
 /* Library version information */
 #define HAL_VERSION_MAJOR 0
 #define HAL_VERSION_MINOR 1
 #define HAL_VERSION_PATCH 0
 #define HAL_VERSION_STRING "0.1.0"
 
 /**
  * @brief Get HAL version information
  * 
  * @param major Pointer to store major version (can be NULL)
  * @param minor Pointer to store minor version (can be NULL)
  * @param patch Pointer to store patch version (can be NULL)
  * @return Version string (static, do not free)
  */
 const char* hal_version(int* major, int* minor, int* patch);
 
 #endif /* HAL_H */