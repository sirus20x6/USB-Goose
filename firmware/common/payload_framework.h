/**
 * payload_framework.h - Core framework header for  USB Goose
 * 
 * This header defines the common interfaces for the payload framework
 * that works consistently across both CH569 and ESP32-C6 chips.
 */

#ifndef PAYLOAD_FRAMEWORK_H
#define PAYLOAD_FRAMEWORK_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Payload metadata macro - used to define payload information
 * This works across C, C++, and other LLVM languages
 */
#define PAYLOAD_INFO(name, version, priority) \
    __attribute__((section(".payload_info"))) \
    const struct { \
        const char* name; \
        const char* version; \
        uint8_t priority; \
    } _payload_info = { name, version, priority }

/**
 * Priority levels for payload execution
 */
#define PAYLOAD_PRIORITY_LOW     0
#define PAYLOAD_PRIORITY_NORMAL  1
#define PAYLOAD_PRIORITY_HIGH    2
#define PAYLOAD_PRIORITY_URGENT  3

/**
 * Operating system identifiers
 */
typedef enum {
    OS_UNKNOWN = 0,
    OS_WINDOWS,
    OS_MACOS,
    OS_LINUX,
    OS_CHROMEOS,
    OS_ANDROID,
    OS_IOS,
    OS_BSD
} OperatingSystem;

/**
 * Basic payload context - shared between chips
 */
typedef struct {
    OperatingSystem detected_os;
    uint8_t security_level;     // 0-100 estimated security level
    uint32_t target_capabilities;
    char hostname[32];
    char username[32];
    uint64_t timestamp;         // RTC timestamp
    uint8_t reserved[32];       // Future expansion
} PayloadContext;

/**
 * Handler function pointer type
 */
typedef int (*PayloadHandler)(PayloadContext* ctx);

/**
 * Handler return codes
 */
#define HANDLER_SUCCESS       0
#define HANDLER_FAILURE       1
#define HANDLER_RETRY         2
#define HANDLER_ABORT         3

/**
 * Handler definition macro
 */
#define HANDLER_DEFINE(name) \
    int name(PayloadContext* ctx)

/**
 * ==========================================
 * OS Detection Subsystem
 * ==========================================
 */

/**
 * OS detection flags
 */
#define OS_DETECT_USB_ONLY     (1 << 0)  // Only use USB-based detection
#define OS_DETECT_NETWORK_ONLY (1 << 1)  // Only use network-based detection
#define OS_DETECT_THOROUGH     (1 << 2)  // Use all available methods (slower)
#define OS_DETECT_CACHED       (1 << 3)  // Use cached results if available

/**
 * OS Detection API
 */
typedef struct {
    // Primary detection function
    OperatingSystem (*detect)(uint32_t flags);
    
    // Additional OS information retrieval
    bool (*getHostname)(char* buffer, size_t max_len);
    bool (*getUsername)(char* buffer, size_t max_len);
    bool (*getOSVersion)(char* buffer, size_t max_len);
    
    // Detection confidence (0-100)
    uint8_t (*getConfidence)(void);
    
    // Internal flags and methods
    uint32_t _internal_flags;
    void* _reserved;
} OsDetection;

/**
 * ==========================================
 * HID (Keyboard/Mouse) Subsystem
 * ==========================================
 */

/**
 * Key codes - compatible with USB HID spec
 * Only a subset shown here
 */
typedef enum {
    KEY_NONE       = 0x00,
    KEY_A          = 0x04,
    KEY_B          = 0x05,
    // ... more standard keys ...
    KEY_Z          = 0x1D,
    KEY_1          = 0x1E,
    // ... more number keys ...
    KEY_0          = 0x27,
    KEY_RETURN     = 0x28,
    KEY_ESCAPE     = 0x29,
    KEY_BACKSPACE  = 0x2A,
    KEY_TAB        = 0x2B,
    KEY_SPACE      = 0x2C,
    // ... more special keys ...
    KEY_F1         = 0x3A,
    // ... more function keys ...
    KEY_F12        = 0x45,
    // ... additional keys ...
} KeyCode;

/**
 * Modifier keys
 */
#define MOD_NONE      0x00
#define MOD_LCTRL     0x01
#define MOD_LSHIFT    0x02
#define MOD_LALT      0x04
#define MOD_LMETA     0x08  // Windows/Command key
#define MOD_RCTRL     0x10
#define MOD_RSHIFT    0x20
#define MOD_RALT      0x40
#define MOD_RMETA     0x80

/**
 * Mouse button definitions
 */
#define MOUSE_LEFT    0x01
#define MOUSE_RIGHT   0x02
#define MOUSE_MIDDLE  0x04
#define MOUSE_BACK    0x08
#define MOUSE_FORWARD 0x10

/**
 * Keyboard API
 */
typedef struct {
    // Basic key press/release
    void (*press)(KeyCode key);
    void (*release)(KeyCode key);
    
    // Combined press and release with delay
    void (*tap)(KeyCode key);
    
    // Modifier operations
    void (*pressModifier)(uint8_t modifiers);
    void (*releaseModifier)(uint8_t modifiers);
    
    // Press key with modifiers, then release all
    void (*tapWithModifiers)(KeyCode key, uint8_t modifiers);
    
    // Type a string with specified delay between keystrokes
    void (*type)(const char* text, uint16_t delay_ms);
    
    // Shorthand for common operations
    void (*sendString)(const char* text);  // Uses default delay
    void (*sendLine)(const char* text);    // Sends text + Enter
    
    // Windows, macOS and Linux specific shortcuts
    void (*sendWinKey)(char key);        // Windows key + letter
    void (*sendCommandKey)(char key);    // Command + letter (macOS)
    void (*sendCtrlKey)(char key);       // Ctrl + letter
    
    // Delay all keyboard operations
    void (*delay)(uint32_t ms);
    
    // Buffer operations (for atomic execution)
    void (*beginBuffer)(void);           // Start collecting commands
    void (*sendBuffer)(void);            // Send all buffered commands
    void (*clearBuffer)(void);           // Clear buffered commands
    
    // Internal functions
    void* _reserved;
} Keyboard;

/**
 * Mouse API
 */
typedef struct {
    // Button operations
    void (*press)(uint8_t buttons);
    void (*release)(uint8_t buttons);
    void (*click)(uint8_t buttons);
    void (*doubleClick)(uint8_t buttons);
    
    // Movement
    void (*move)(int16_t x, int16_t y);
    void (*moveTo)(uint16_t x, uint16_t y);  // Absolute (if supported)
    void (*scroll)(int8_t amount);
    
    // Combined operations
    void (*dragTo)(uint16_t x, uint16_t y);
    
    // Buffer operations (for atomic execution)
    void (*beginBuffer)(void);
    void (*sendBuffer)(void);
    void (*clearBuffer)(void);
    
    // Internal functions
    void* _reserved;
} Mouse;

/**
 * ==========================================
 * Inter-Chip Communication
 * ==========================================
 */

/**
 * Common event types for inter-chip communication
 */
typedef enum {
    EVENT_SYSTEM_BOOT = 0,
    EVENT_USB_CONNECTED,
    EVENT_USB_DISCONNECTED,
    EVENT_WIFI_CONNECTED,
    EVENT_WIFI_DISCONNECTED,
    EVENT_BLE_CONNECTED,
    EVENT_BLE_DISCONNECTED,
    EVENT_OS_DETECTED,
    EVENT_PAYLOAD_START,
    EVENT_PAYLOAD_COMPLETE,
    EVENT_BUTTON_PRESSED,
    EVENT_SECURITY_ALERT,
    EVENT_CUSTOM_BASE = 0x1000   // Start of custom event IDs
} EventType;

/**
 * Inter-chip communication API
 */
typedef struct {
    // Event-based communication
    void (*triggerEvent)(EventType event, const void* data, size_t data_len);
    bool (*waitForEvent)(EventType event, uint32_t timeout_ms);
    
    // Register event handlers
    void (*registerHandler)(EventType event, void (*handler)(const void* data, size_t data_len));
    void (*unregisterHandler)(EventType event);
    
    // Data transfer
    bool (*sendData)(uint16_t channel, const void* data, size_t data_len);
    bool (*receiveData)(uint16_t channel, void* buffer, size_t buffer_len, size_t* received_len);
    
    // Status information
    bool (*isConnected)(void);
    uint8_t (*getRemoteStatus)(void);
    
    // Internal functions
    void* _reserved;
} InterChip;

/**
 * ==========================================
 * System Information and Utilities
 * ==========================================
 */

/**
 * LED control API
 */
typedef struct {
    void (*setColor)(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    void (*setPattern)(uint8_t pattern);
    void (*setBrightness)(uint8_t brightness);
    void (*off)(void);
    void* _reserved;
} LED;

/**
 * System information API
 */
typedef struct {
    // Chip identification
    bool (*isUSBController)(void);
    bool (*isWirelessController)(void);
    
    // Clock and timing
    uint64_t (*getTimestamp)(void);
    void (*delay)(uint32_t ms);
    
    // OS detection (convenience wrapper around OsDetection)
    OperatingSystem (*detectOS)(void);
    
    // Memory information
    uint32_t (*getFreeMemory)(void);
    
    // Hardware status
    uint8_t (*getBatteryLevel)(void);  // 0-100 percentage
    int8_t (*getTemperature)(void);    // Temperature in celsius
    
    // Power management
    void (*sleep)(uint32_t ms);
    void (*deepSleep)(uint32_t ms);
    
    // Logging
    void (*log)(const char* format, ...);
    
    // Internal functions
    void* _reserved;
} SystemInfo;

/**
 * ==========================================
 * Global API Access
 * ==========================================
 */

// Main API instances - extern references that will be initialized by the system
extern OsDetection OSDetect;
extern Keyboard KeyboardHID;
extern Mouse MouseHID;
extern InterChip ChipComm;
extern LED StatusLED;
extern SystemInfo System;

/**
 * Initialize all subsystems - called by framework, not by payload
 */
bool framework_init(void);

/**
 * Entry point for payloads
 * Each payload must define this function
 */
void payload_main(PayloadContext* ctx);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* PAYLOAD_FRAMEWORK_H */