// comm_manager.c
#include "comm_manager.h"

// Channel state storage
CommChannelState comm_channels[COMM_CHANNEL_COUNT];

// Initialize with default values
void comm_manager_init(void) {
    // Initialize keyboard channel (always available)
    comm_channels[COMM_CHANNEL_KEYBOARD] = (CommChannelState){
        .status = CHANNEL_STATUS_AVAILABLE,
        .priority = 7,              // Lowest priority but most reliable
        .speed_kbps = 1,            // ~1Kbps (very slow)
        .stealth_rating = 1,        // Very visible
        .reliability = 10,          // Always works
        .last_used = 0,
        .error_count = 0,
        .is_bidirectional = false   // One-way communication
    };
    
    // Initialize mouse channel
    comm_channels[COMM_CHANNEL_MOUSE] = (CommChannelState){
        .status = CHANNEL_STATUS_AVAILABLE,
        .priority = 6,              
        .speed_kbps = 1,            // ~1Kbps (very slow)
        .stealth_rating = 3,        // Less visible than keyboard
        .reliability = 9,           // Very reliable
        .last_used = 0,
        .error_count = 0,
        .is_bidirectional = false   // One-way communication
    };
    
    // Initialize NIC channel
    comm_channels[COMM_CHANNEL_NIC] = (CommChannelState){
        .status = CHANNEL_STATUS_UNAVAILABLE, // Needs to be established
        .priority = 1,              // Highest priority when available
        .speed_kbps = 100000,       // 100Mbps (USB 2.0 effective throughput)
        .stealth_rating = 5,        // Moderate stealth (visible in network adapters)
        .reliability = 7,           // Good but requires specific OS support
        .last_used = 0,
        .error_count = 0,
        .is_bidirectional = true    // Full two-way communication
    };
    
    // Initialize Mass Storage channel
    comm_channels[COMM_CHANNEL_MASS_STORAGE] = (CommChannelState){
        .status = CHANNEL_STATUS_UNAVAILABLE, // Needs to be established
        .priority = 2,              // High priority for data transfer
        .speed_kbps = 400000,       // ~400Mbps (USB 3.0 practical throughput)
        .stealth_rating = 6,        // Reasonably stealthy (looks like normal drive)
        .reliability = 8,           // Very reliable for data transfer
        .last_used = 0,
        .error_count = 0,
        .is_bidirectional = true    // Can read and write
    };
    
    // Initialize Bluetooth channel
    comm_channels[COMM_CHANNEL_BLUETOOTH] = (CommChannelState){
        .status = CHANNEL_STATUS_UNAVAILABLE, // Needs wireless controller
        .priority = 4,              // Medium priority
        .speed_kbps = 2000,         // ~2Mbps (BLE 5.0)
        .stealth_rating = 7,        // Fairly stealthy (no physical connection)
        .reliability = 5,           // Moderate (depends on environment)
        .last_used = 0,
        .error_count = 0,
        .is_bidirectional = true    // Full two-way communication
    };
    
    // Initialize Wi-Fi channel
    comm_channels[COMM_CHANNEL_WIFI] = (CommChannelState){
        .status = CHANNEL_STATUS_UNAVAILABLE, // Needs wireless controller
        .priority = 3,              // High-medium priority
        .speed_kbps = 50000,        // ~50Mbps (practical Wi-Fi throughput)
        .stealth_rating = 8,        // Very stealthy (can connect to external networks)
        .reliability = 6,           // Depends heavily on environment
        .last_used = 0,
        .error_count = 0,
        .is_bidirectional = true    // Full two-way communication
    };
    
    // Initialize HID Raw channel
    comm_channels[COMM_CHANNEL_HID_RAW] = (CommChannelState){
        .status = CHANNEL_STATUS_UNAVAILABLE, // Needs custom software on target
        .priority = 5,              // Medium-low priority
        .speed_kbps = 64,           // 64Kbps (HID report size * frequency)
        .stealth_rating = 4,        // Somewhat visible (unusual HID device)
        .reliability = 4,           // Depends on target system
        .last_used = 0,
        .error_count = 0,
        .is_bidirectional = true    // Can receive input reports
    };
}

// Find best channel matching requirements
CommChannelType comm_get_best_channel(uint8_t min_stealth, uint32_t min_speed, bool require_bidirectional) {
    CommChannelType best_channel = COMM_CHANNEL_KEYBOARD; // Default fallback
    uint8_t best_priority = 255; // Lower is better
    
    for (int i = 0; i < COMM_CHANNEL_COUNT; i++) {
        // Check if channel meets requirements
        if (comm_channels[i].status == CHANNEL_STATUS_CONNECTED &&
            comm_channels[i].stealth_rating >= min_stealth &&
            comm_channels[i].speed_kbps >= min_speed &&
            (!require_bidirectional || comm_channels[i].is_bidirectional)) {
            
            // Check if this channel has higher priority (lower number)
            if (comm_channels[i].priority < best_priority) {
                best_priority = comm_channels[i].priority;
                best_channel = (CommChannelType)i;
            }
        }
    }
    
    return best_channel;
}

// Send data with fallbacks
bool comm_send_data(const void* data, size_t length, CommChannelType preferred_channel) {
    if (data == NULL || length == 0) {
        return false;
    }
    
    // Try preferred channel first if specified and connected
    if (preferred_channel != COMM_CHANNEL_COUNT && 
        comm_channels[preferred_channel].status == CHANNEL_STATUS_CONNECTED) {
        if (send_data_via_channel(preferred_channel, data, length)) {
            comm_channels[preferred_channel].last_used = System.getTimestamp();
            return true;
        } else {
            comm_channels[preferred_channel].error_count++;
        }
    }
    
    // Try channels in priority order
    for (int priority = 1; priority <= 7; priority++) {
        for (int i = 0; i < COMM_CHANNEL_COUNT; i++) {
            if (comm_channels[i].priority == priority && 
                comm_channels[i].status == CHANNEL_STATUS_CONNECTED) {
                
                if (i == preferred_channel) {
                    continue; // Already tried
                }
                
                if (send_data_via_channel((CommChannelType)i, data, length)) {
                    comm_channels[i].last_used = System.getTimestamp();
                    return true;
                } else {
                    comm_channels[i].error_count++;
                }
            }
        }
    }
    
    // All channels failed
    return false;
}

// Helper function to send via specific channel
static bool send_data_via_channel(CommChannelType channel, const void* data, size_t length) {
    switch (channel) {
        case COMM_CHANNEL_KEYBOARD:
            // Convert data to text and send via keyboard
            // (implementation depends on data format)
            return keyboard_send_data(data, length);
            
        case COMM_CHANNEL_MOUSE:
            // Use mouse movements to encode data
            // (very limited, specialized implementation)
            return mouse_send_data(data, length);
            
        case COMM_CHANNEL_NIC:
            // Send data over network
            return nic_send_data(data, length);
            
        case COMM_CHANNEL_MASS_STORAGE:
            // Write data to mass storage
            return mass_storage_write_data(data, length);
            
        case COMM_CHANNEL_BLUETOOTH:
            // Send over Bluetooth
            return bluetooth_send_data(data, length);
            
        case COMM_CHANNEL_WIFI:
            // Send over Wi-Fi
            return wifi_send_data(data, length);
            
        case COMM_CHANNEL_HID_RAW:
            // Send via HID reports
            return hid_raw_send_data(data, length);
            
        default:
            return false;
    }
}