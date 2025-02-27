/**
 * ch569_hid.c - Implementation of HID functionality for CH569
 * 
 * This file implements the keyboard and mouse HID functionality
 * for the WCH CH569 USB controller.
 */

#include "payload_framework.h"
#include "ch569_usb.h"  // CH569 USB hardware interface
#include "ch569_gpio.h" // CH569 GPIO for status LEDs
#include <string.h>

// HID report buffer sizes
#define KEYBOARD_REPORT_SIZE 8
#define MOUSE_REPORT_SIZE 5

// Report buffer for keyboard
static uint8_t keyboard_report[KEYBOARD_REPORT_SIZE] = {0};
static uint8_t keyboard_buffer[KEYBOARD_REPORT_SIZE * 32];  // For buffered operations
static uint16_t keyboard_buffer_pos = 0;
static bool keyboard_buffering = false;

// Report buffer for mouse
static uint8_t mouse_report[MOUSE_REPORT_SIZE] = {0};
static uint8_t mouse_buffer[MOUSE_REPORT_SIZE * 32];  // For buffered operations
static uint16_t mouse_buffer_pos = 0;
static bool mouse_buffering = false;

// Default delay between keystrokes (milliseconds)
static uint16_t default_keystroke_delay = 5;

// ASCII to HID conversion table
static const KeyCode ascii_to_hid[128] = {
    0, 0, 0, 0, 0, 0, 0, 0,                                     // 0-7
    KEY_BACKSPACE, KEY_TAB, KEY_RETURN, 0, 0, 0, 0, 0,          // 8-15
    0, 0, 0, 0, 0, 0, 0, 0,                                     // 16-23
    0, 0, 0, KEY_ESCAPE, 0, 0, 0, 0,                            // 24-31
    KEY_SPACE, 0, 0, 0, 0, 0, 0, 0,                             // 32-39
    0, 0, 0, 0, 0, 0, 0, 0,                                     // 40-47
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7,     // 48-55
    KEY_8, KEY_9, 0, 0, 0, 0, 0, 0,                             // 56-63
    0, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G,         // 64-71
    KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O,     // 72-79
    KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W,     // 80-87
    KEY_X, KEY_Y, KEY_Z, 0, 0, 0, 0, 0,                         // 88-95
    0, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G,         // 96-103 (lowercase maps to same keys)
    KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O,     // 104-111
    KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W,     // 112-119
    KEY_X, KEY_Y, KEY_Z, 0, 0, 0, 0, 0                          // 120-127
};

// Shift required for ASCII conversion
static const uint8_t ascii_requires_shift[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 0-15
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 16-31
    0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0,             // 32-47
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,             // 48-63
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,             // 64-79 (uppercase requires shift)
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0,             // 80-95
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 96-111 (lowercase doesn't need shift)
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0              // 112-127
};

// Forward declarations for Keyboard API implementation
static void keyboard_press(KeyCode key);
static void keyboard_release(KeyCode key);
static void keyboard_tap(KeyCode key);
static void keyboard_press_modifier(uint8_t modifiers);
static void keyboard_release_modifier(uint8_t modifiers);
static void keyboard_tap_with_modifiers(KeyCode key, uint8_t modifiers);
static void keyboard_type(const char* text, uint16_t delay_ms);
static void keyboard_send_string(const char* text);
static void keyboard_send_line(const char* text);
static void keyboard_send_win_key(char key);
static void keyboard_send_command_key(char key);
static void keyboard_send_ctrl_key(char key);
static void keyboard_delay(uint32_t ms);
static void keyboard_begin_buffer(void);
static void keyboard_send_buffer(void);
static void keyboard_clear_buffer(void);

// Forward declarations for Mouse API implementation
static void mouse_press(uint8_t buttons);
static void mouse_release(uint8_t buttons);
static void mouse_click(uint8_t buttons);
static void mouse_double_click(uint8_t buttons);
static void mouse_move(int16_t x, int16_t y);
static void mouse_move_to(uint16_t x, uint16_t y);
static void mouse_scroll(int8_t amount);
static void mouse_drag_to(uint16_t x, uint16_t y);
static void mouse_begin_buffer(void);
static void mouse_send_buffer(void);
static void mouse_clear_buffer(void);

// Initialize the Keyboard API structure
Keyboard KeyboardHID = {
    .press = keyboard_press,
    .release = keyboard_release,
    .tap = keyboard_tap,
    .pressModifier = keyboard_press_modifier,
    .releaseModifier = keyboard_release_modifier,
    .tapWithModifiers = keyboard_tap_with_modifiers,
    .type = keyboard_type,
    .sendString = keyboard_send_string,
    .sendLine = keyboard_send_line,
    .sendWinKey = keyboard_send_win_key,
    .sendCommandKey = keyboard_send_command_key,
    .sendCtrlKey = keyboard_send_ctrl_key,
    .delay = keyboard_delay,
    .beginBuffer = keyboard_begin_buffer,
    .sendBuffer = keyboard_send_buffer,
    .clearBuffer = keyboard_clear_buffer,
    ._reserved = NULL
};

// Initialize the Mouse API structure
Mouse MouseHID = {
    .press = mouse_press,
    .release = mouse_release,
    .click = mouse_click,
    .doubleClick = mouse_double_click,
    .move = mouse_move,
    .moveTo = mouse_move_to,
    .scroll = mouse_scroll,
    .dragTo = mouse_drag_to,
    .beginBuffer = mouse_begin_buffer,
    .sendBuffer = mouse_send_buffer,
    .clearBuffer = mouse_clear_buffer,
    ._reserved = NULL
};

// Send the current keyboard report over USB
static void send_keyboard_report(void) {
    ch569_usb_send_hid_report(HID_KEYBOARD_INTERFACE, keyboard_report, KEYBOARD_REPORT_SIZE);
}

// Send the current mouse report over USB
static void send_mouse_report(void) {
    ch569_usb_send_hid_report(HID_MOUSE_INTERFACE, mouse_report, MOUSE_REPORT_SIZE);
}

// Keyboard implementation

bool keyboard_init(void) {
    // Clear the keyboard report
    memset(keyboard_report, 0, KEYBOARD_REPORT_SIZE);
    
    // Set up the Keyboard API structure
    KeyboardHID.press = keyboard_press;
    KeyboardHID.release = keyboard_release;
    KeyboardHID.tap = keyboard_tap;
    KeyboardHID.pressModifier = keyboard_press_modifier;
    KeyboardHID.releaseModifier = keyboard_release_modifier;
    KeyboardHID.tapWithModifiers = keyboard_tap_with_modifiers;
    KeyboardHID.type = keyboard_type;
    KeyboardHID.sendString = keyboard_send_string;
    KeyboardHID.sendLine = keyboard_send_line;
    KeyboardHID.sendWinKey = keyboard_send_win_key;
    KeyboardHID.sendCommandKey = keyboard_send_command_key;
    KeyboardHID.sendCtrlKey = keyboard_send_ctrl_key;
    KeyboardHID.delay = keyboard_delay;
    KeyboardHID.beginBuffer = keyboard_begin_buffer;
    KeyboardHID.sendBuffer = keyboard_send_buffer;
    KeyboardHID.clearBuffer = keyboard_clear_buffer;
    
    return true;
}

static void keyboard_press(KeyCode key) {
    // Find an empty slot in the keyboard report
    for (int i = 2; i < KEYBOARD_REPORT_SIZE; i++) {
        if (keyboard_report[i] == 0) {
            keyboard_report[i] = key;
            
            if (keyboard_buffering) {
                // Store in buffer for later transmission
                memcpy(&keyboard_buffer[keyboard_buffer_pos], keyboard_report, KEYBOARD_REPORT_SIZE);
                keyboard_buffer_pos += KEYBOARD_REPORT_SIZE;
            } else {
                // Send immediately
                send_keyboard_report();
            }
            return;
        }
    }
}

static void keyboard_release(KeyCode key) {
    // Find and clear the key from the report
    for (int i = 2; i < KEYBOARD_REPORT_SIZE; i++) {
        if (keyboard_report[i] == key) {
            keyboard_report[i] = 0;
        }
    }
    
    if (keyboard_buffering) {
        // Store in buffer for later transmission
        memcpy(&keyboard_buffer[keyboard_buffer_pos], keyboard_report, KEYBOARD_REPORT_SIZE);
        keyboard_buffer_pos += KEYBOARD_REPORT_SIZE;
    } else {
        // Send immediately
        send_keyboard_report();
    }
}

static void keyboard_tap(KeyCode key) {
    keyboard_press(key);
    keyboard_delay(default_keystroke_delay);
    keyboard_release(key);
}

static void keyboard_press_modifier(uint8_t modifiers) {
    keyboard_report[0] |= modifiers;
    
    if (keyboard_buffering) {
        // Store in buffer for later transmission
        memcpy(&keyboard_buffer[keyboard_buffer_pos], keyboard_report, KEYBOARD_REPORT_SIZE);
        keyboard_buffer_pos += KEYBOARD_REPORT_SIZE;
    } else {
        // Send immediately
        send_keyboard_report();
    }
}

static void keyboard_release_modifier(uint8_t modifiers) {
    keyboard_report[0] &= ~modifiers;
    
    if (keyboard_buffering) {
        // Store in buffer for later transmission
        memcpy(&keyboard_buffer[keyboard_buffer_pos], keyboard_report, KEYBOARD_REPORT_SIZE);
        keyboard_buffer_pos += KEYBOARD_REPORT_SIZE;
    } else {
        // Send immediately
        send_keyboard_report();
    }
}

static void keyboard_tap_with_modifiers(KeyCode key, uint8_t modifiers) {
    keyboard_press_modifier(modifiers);
    keyboard_press(key);
    keyboard_delay(default_keystroke_delay);
    keyboard_release(key);
    keyboard_release_modifier(modifiers);
}

static void keyboard_type(const char* text, uint16_t delay_ms) {
    for (const char* p = text; *p != '\0'; p++) {
        char c = *p;
        if (c < 128) {  // ASCII only
            KeyCode key = ascii_to_hid[c];
            if (key != 0) {
                if (ascii_requires_shift[c]) {
                    keyboard_tap_with_modifiers(key, MOD_LSHIFT);
                } else {
                    keyboard_tap(key);
                }
                keyboard_delay(delay_ms);
            }
        }
    }
}

static void keyboard_send_string(const char* text) {
    keyboard_type(text, default_keystroke_delay);
}

static void keyboard_send_line(const char* text) {
    keyboard_send_string(text);
    keyboard_tap(KEY_RETURN);
}

static void keyboard_send_win_key(char key) {
    if (key >= 'a' && key <= 'z') {
        keyboard_tap_with_modifiers(ascii_to_hid[key], MOD_LMETA);
    } else if (key >= 'A' && key <= 'Z') {
        keyboard_tap_with_modifiers(ascii_to_hid[key + 32], MOD_LMETA);
    } else {
        keyboard_tap_with_modifiers(ascii_to_hid[key], MOD_LMETA);
    }
}

static void keyboard_send_command_key(char key) {
    // Same as Win key for consistency
    keyboard_send_win_key(key);
}

static void keyboard_send_ctrl_key(char key) {
    if (key >= 'a' && key <= 'z') {
        keyboard_tap_with_modifiers(ascii_to_hid[key], MOD_LCTRL);
    } else if (key >= 'A' && key <= 'Z') {
        keyboard_tap_with_modifiers(ascii_to_hid[key + 32], MOD_LCTRL);
    } else {
        keyboard_tap_with_modifiers(ascii_to_hid[key], MOD_LCTRL);
    }
}

static void keyboard_delay(uint32_t ms) {
    ch569_delay_ms(ms);
}

static void keyboard_begin_buffer(void) {
    keyboard_buffering = true;
    keyboard_buffer_pos = 0;
}

static void keyboard_send_buffer(void) {
    if (keyboard_buffering) {
        uint16_t pos = 0;
        while (pos < keyboard_buffer_pos) {
            ch569_usb_send_hid_report(HID_KEYBOARD_INTERFACE, 
                                    &keyboard_buffer[pos], 
                                    KEYBOARD_REPORT_SIZE);
            pos += KEYBOARD_REPORT_SIZE;
            ch569_delay_ms(default_keystroke_delay);
        }
        keyboard_buffering = false;
        keyboard_buffer_pos = 0;
    }
}

static void keyboard_clear_buffer(void) {
    keyboard_buffering = false;
    keyboard_buffer_pos = 0;
}

// Mouse implementation

static void mouse_press(uint8_t buttons) {
    mouse_report[0] |= buttons;
    
    if (mouse_buffering) {
        // Store in buffer for later transmission
        memcpy(&mouse_buffer[mouse_buffer_pos], mouse_report, MOUSE_REPORT_SIZE);
        mouse_buffer_pos += MOUSE_REPORT_SIZE;
    } else {
        // Send immediately
        send_mouse_report();
    }
}

static void mouse_release(uint8_t buttons) {
    mouse_report[0] &= ~buttons;
    
    if (mouse_buffering) {
        // Store in buffer for later transmission
        memcpy(&mouse_buffer[mouse_buffer_pos], mouse_report, MOUSE_REPORT_SIZE);
        mouse_buffer_pos += MOUSE_REPORT_SIZE;
    } else {
        // Send immediately
        send_mouse_report();
    }
}

static void mouse_click(uint8_t buttons) {
    mouse_press(buttons);
    keyboard_delay(10);  // Short delay between press and release
    mouse_release(buttons);
}

static void mouse_double_click(uint8_t buttons) {
    mouse_click(buttons);
    keyboard_delay(10);  // Delay between clicks
    mouse_click(buttons);
}

static void mouse_move(int16_t x, int16_t y) {
    // Set relative movement (clamped to int8_t as per HID spec)
    mouse_report[1] = (x > 127) ? 127 : ((x < -127) ? -127 : x);
    mouse_report[2] = (y > 127) ? 127 : ((y < -127) ? -127 : y);
    
    if (mouse_buffering) {
        // Store in buffer for later transmission
        memcpy(&mouse_buffer[mouse_buffer_pos], mouse_report, MOUSE_REPORT_SIZE);
        mouse_buffer_pos += MOUSE_REPORT_SIZE;
    } else {
        // Send immediately
        send_mouse_report();
    }
    
    // Reset movement values after sending
    mouse_report[1] = 0;
    mouse_report[2] = 0;
}

static void mouse_move_to(uint16_t x, uint16_t y) {
    // This is a simplified implementation, as absolute positioning
    // requires additional HID descriptor support. For now, we log
    // that this operation isn't fully supported.
    SystemInfo.log("Warning: Absolute mouse positioning not fully implemented");
    
    // In the future, this would use the absolute coordinates HID report
}

static void mouse_scroll(int8_t amount) {
    mouse_report[3] = amount;
    
    if (mouse_buffering) {
        // Store in buffer for later transmission
        memcpy(&mouse_buffer[mouse_buffer_pos], mouse_report, MOUSE_REPORT_SIZE);
        mouse_buffer_pos += MOUSE_REPORT_SIZE;
    } else {
        // Send immediately
        send_mouse_report();
    }
    
    // Reset scroll value after sending
    mouse_report[3] = 0;
}

static void mouse_drag_to(uint16_t x, uint16_t y) {
    // Since absolute positioning isn't fully implemented,
    // this is just a placeholder
    SystemInfo.log("Warning: Mouse drag not fully implemented");
}

static void mouse_begin_buffer(void) {
    mouse_buffering = true;
    mouse_buffer_pos = 0;
}

static void mouse_send_buffer(void) {
    if (mouse_buffering) {
        uint16_t pos = 0;
        while (pos < mouse_buffer_pos) {
            ch569_usb_send_hid_report(HID_MOUSE_INTERFACE, 
                                    &mouse_buffer[pos], 
                                    MOUSE_REPORT_SIZE);
            pos += MOUSE_REPORT_SIZE;
            ch569_delay_ms(5);  // Small delay between reports
        }
        mouse_buffering = false;
        mouse_buffer_pos = 0;
    }
}

static void mouse_clear_buffer(void) {
    mouse_buffering = false;
    mouse_buffer_pos = 0;
}


/**
 * ch569_os_detect.c - Implementation of OS detection for CH569
 * 
 * This file implements the OS detection functionality for
 * the WCH CH569 USB controller.
 */

#include "payload_framework.h"
#include "ch569_usb.h"
#include <string.h>

// OS detection state
static OperatingSystem detected_os = OS_UNKNOWN;
static uint8_t detection_confidence = 0;
static char os_hostname[32] = {0};
static char os_username[32] = {0};
static char os_version[32] = {0};
static uint32_t detection_flags = 0;

// Forward declarations for OS Detection API implementation
static OperatingSystem os_detect(uint32_t flags);
static bool os_get_hostname(char* buffer, size_t max_len);
static bool os_get_username(char* buffer, size_t max_len);
static bool os_get_os_version(char* buffer, size_t max_len);
static uint8_t os_get_confidence(void);

// Initialize the OS Detection API structure
OsDetection OSDetect = {
    .detect = os_detect,
    .getHostname = os_get_hostname,
    .getUsername = os_get_username,
    .getOSVersion = os_get_os_version,
    .getConfidence = os_get_confidence,
    ._internal_flags = 0,
    ._reserved = NULL
};

// USB-based OS detection patterns
typedef struct {
    OperatingSystem os;
    const char* usb_pattern;
} UsbOsPattern;

// These patterns match strings in USB descriptor responses
static const UsbOsPattern usb_patterns[] = {
    {OS_WINDOWS, "WINDOWS"},
    {OS_WINDOWS, "MSFT"},
    {OS_MACOS, "APPLE"},
    {OS_MACOS, "MAC"},
    {OS_LINUX, "LINUX"},
    {OS_LINUX, "UBUNTU"},
    {OS_LINUX, "DEBIAN"},
    {OS_LINUX, "FEDORA"},
    {OS_CHROMEOS, "CHROMEOS"},
    {OS_CHROMEOS, "CHROME OS"},
    {OS_CHROMEOS, "GOOGLE"},
    {OS_ANDROID, "ANDROID"},
    {OS_BSD, "BSD"},
    {OS_BSD, "FREEBSD"},
    {0, NULL}  // End marker
};

// USB timing-based OS detection
// Different OSes have characteristic USB enumeration timing patterns
typedef struct {
    OperatingSystem os;
    uint16_t timing_pattern[4];  // Sequential timings in ms
    uint8_t tolerance;           // Tolerance percentage
} UsbTimingPattern;

static const UsbTimingPattern timing_patterns[] = {
    {OS_WINDOWS, {10, 20, 5, 15}, 30},
    {OS_MACOS, {15, 10, 10, 10}, 30},
    {OS_LINUX, {5, 5, 10, 5}, 40},
    {0, {0, 0, 0, 0}, 0}  // End marker
};

// Behavioral detection for BIOS environment
static bool detect_bios_environment(void) {
    // Check for timing patterns consistent with BIOS
    // This is a very simplified implementation
    return false;  // Placeholder
}

// USB descriptor-based OS detection
static OperatingSystem detect_from_usb_descriptors(void) {
    // Get USB descriptor strings from the host
    char manufacturer[64] = {0};
    char product[64] = {0};
    
    // Get string descriptors (simplified - actual implementation would use CH569 USB functions)
    ch569_usb_get_string_descriptor(1, manufacturer, sizeof(manufacturer));
    ch569_usb_get_string_descriptor(2, product, sizeof(product));
    
    // Check for OS-specific patterns in the strings
    for (int i = 0; usb_patterns[i].usb_pattern != NULL; i++) {
        if (strstr(manufacturer, usb_patterns[i].usb_pattern) || 
            strstr(product, usb_patterns[i].usb_pattern)) {
            detection_confidence = 80;
            return usb_patterns[i].os;
        }
    }
    
    return OS_UNKNOWN;
}

// USB timing-based detection
static OperatingSystem detect_from_timing(void) {
    // Get observed timing pattern from USB enumeration
    // This would capture actual timings in real implementation
    uint16_t observed_timings[4] = {12, 18, 6, 14};  // Placeholder values
    
    for (int i = 0; timing_patterns[i].os != 0; i++) {
        bool match = true;
        for (int j = 0; j < 4; j++) {
            uint16_t expected = timing_patterns[i].timing_pattern[j];
            uint16_t tolerance = (expected * timing_patterns[i].tolerance) / 100;
            
            if (observed_timings[j] < expected - tolerance || 
                observed_timings[j] > expected + tolerance) {
                match = false;
                break;
            }
        }
        
        if (match) {
            detection_confidence = 60;  // Timing is less reliable than descriptors
            return timing_patterns[i].os;
        }
    }
    
    return OS_UNKNOWN;
}

// HID usage pattern detection
static OperatingSystem detect_from_hid_usage(void) {
    // Different OSes use different HID report features
    // This would analyze HID report descriptor requests
    
    // Simplified implementation
    uint8_t hid_features = ch569_usb_get_hid_features();
    
    if (hid_features & 0x01) {
        // Features typical of Windows
        detection_confidence = 50;
        return OS_WINDOWS;
    } else if (hid_features & 0x02) {
        // Features typical of macOS
        detection_confidence = 50;
        return OS_MACOS;
    }
    
    return OS_UNKNOWN;
}

// Combined USB-based detection
static OperatingSystem usb_based_detection(void) {
    OperatingSystem descriptor_os = detect_from_usb_descriptors();
    if (descriptor_os != OS_UNKNOWN) {
        return descriptor_os;
    }
    
    OperatingSystem timing_os = detect_from_timing();
    if (timing_os != OS_UNKNOWN) {
        return timing_os;
    }
    
    return detect_from_hid_usage();
}

// Main OS detection function
static OperatingSystem os_detect(uint32_t flags) {
    detection_flags = flags;
    
    // Check if we have a cached result and are allowed to use it
    if (detected_os != OS_UNKNOWN && (flags & OS_DETECT_CACHED)) {
        return detected_os;
    }
    
    // Check if we're in BIOS pre-boot environment
    if (detect_bios_environment()) {
        detected_os = OS_UNKNOWN;
        detection_confidence = 90;
        strcpy(os_hostname, "BIOS");
        return detected_os;
    }
    
    // Perform USB-based detection (always available on the USB controller)
    detected_os = usb_based_detection();
    
    // If we detected the OS and have ESP32 communication, request additional info
    if (detected_os != OS_UNKNOWN && ChipComm.isConnected()) {
        // Request additional OS detection from ESP32 (wireless-based)
        uint8_t result[64];
        size_t received = 0;
        
        if (ChipComm.triggerEvent(EVENT_OS_DETECTED, &detected_os, sizeof(detected_os)) &&
            ChipComm.receiveData(0, result, sizeof(result), &received)) {
            
            // If ESP32 has stronger evidence, use its results
            if (received >= 5 && result[0] != OS_UNKNOWN && result[1] > detection_confidence) {
                detected_os = (OperatingSystem)result[0];
                detection_confidence = result[1];
                
                // Extract hostname, username if provided
                if (received > 6 && result[2] > 0) {
                    strncpy(os_hostname, (char*)&result[5], result[2]);
                    os_hostname[result[2]] = '\0';
                }
                
                if (received > 6 + result[2] && result[3] > 0) {
                    strncpy(os_username, (char*)&result[5 + result[2]], result[3]);
                    os_username[result[3]] = '\0';
                }
                
                if (received > 6 + result[2] + result[3] && result[4] > 0) {
                    strncpy(os_version, (char*)&result[5 + result[2] + result[3]], result[4]);
                    os_version[result[4]] = '\0';
                }
            }
        }
    }
    
    return detected_os;
}

static bool os_get_hostname(char* buffer, size_t max_len) {
    if (buffer && max_len > 0 && os_hostname[0] != '\0') {
        strncpy(buffer, os_hostname, max_len - 1);
        buffer[max_len - 1] = '\0';
        return true;
    }
    return false;
}

static bool os_get_username(char* buffer, size_t max_len) {
    if (buffer && max_len > 0 && os_username[0] != '\0') {
        strncpy(buffer, os_username, max_len - 1);
        buffer[max_len - 1] = '\0';
        return true;
    }
    return false;
}

static bool os_get_os_version(char* buffer, size_t max_len) {
    if (buffer && max_len > 0 && os_version[0] != '\0') {
        strncpy(buffer, os_version, max_len - 1);
        buffer[max_len - 1] = '\0';
        return true;
    }
    return false;
}

static uint8_t os_get_confidence(void) {
    return detection_confidence;
}