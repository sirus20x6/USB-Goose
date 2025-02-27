/**
 * payload_framework.c - Core implementation of the payload framework
 */

#include "payload_framework.h"
#include <string.h>

// Global API instances - default implementations will be overridden by chip-specific code
OsDetection OSDetect = {0};
Keyboard KeyboardHID = {0};
Mouse MouseHID = {0};
InterChip ChipComm = {0};
LED StatusLED = {0};
SystemInfo System = {0};

// Default payload context
static PayloadContext default_context = {
    .detected_os = OS_UNKNOWN,
    .security_level = 0,
    .target_capabilities = 0,
    .hostname = {0},
    .username = {0},
    .timestamp = 0,
    .reserved = {0}
};

// Keep track of registered event handlers
typedef struct {
    EventType event;
    void (*handler)(const void* data, size_t data_len);
} EventHandler;

#define MAX_EVENT_HANDLERS 32
static EventHandler event_handlers[MAX_EVENT_HANDLERS];
static uint8_t num_event_handlers = 0;

// Initialize the framework
bool framework_init(void) {
    // Reset the context
    memset(&default_context, 0, sizeof(PayloadContext));
    default_context.detected_os = OS_UNKNOWN;
    
    // Clear event handlers
    memset(event_handlers, 0, sizeof(event_handlers));
    num_event_handlers = 0;
    
    // Initialize subsystems - this will be platform-specific
    // and will override the global API instances
    
    // Placeholder for chip-specific initialization
    if (System.isUSBController()) {
        // USB controller (CH569) specific initialization
    } else {
        // Wireless controller (ESP32-C6) specific initialization
    }
    
    // Framework is initialized successfully
    return true;
}

// Register event handler implementation
void register_event_handler(EventType event, void (*handler)(const void* data, size_t data_len)) {
    if (num_event_handlers < MAX_EVENT_HANDLERS) {
        event_handlers[num_event_handlers].event = event;
        event_handlers[num_event_handlers].handler = handler;
        num_event_handlers++;
    }
}

// Unregister event handler implementation
void unregister_event_handler(EventType event) {
    for (int i = 0; i < num_event_handlers; i++) {
        if (event_handlers[i].event == event) {
            // Remove this handler by shifting all subsequent handlers
            for (int j = i; j < num_event_handlers - 1; j++) {
                event_handlers[j] = event_handlers[j + 1];
            }
            num_event_handlers--;
            break;
        }
    }
}

// Dispatch event to registered handlers
void dispatch_event(EventType event, const void* data, size_t data_len) {
    for (int i = 0; i < num_event_handlers; i++) {
        if (event_handlers[i].event == event && event_handlers[i].handler != NULL) {
            event_handlers[i].handler(data, data_len);
        }
    }
}

// Execute a payload within the framework
int execute_payload(PayloadHandler handler, PayloadContext* ctx) {
    if (handler == NULL) {
        return HANDLER_FAILURE;
    }
    
    // Use the provided context or the default one
    PayloadContext* context = (ctx != NULL) ? ctx : &default_context;
    
    // Fill in any missing context information
    if (context->detected_os == OS_UNKNOWN && System.detectOS != NULL) {
        context->detected_os = System.detectOS();
    }
    
    if (context->timestamp == 0 && System.getTimestamp != NULL) {
        context->timestamp = System.getTimestamp();
    }
    
    // Execute the payload
    return handler(context);
}