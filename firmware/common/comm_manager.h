// comm_manager.h
typedef enum {
    COMM_CHANNEL_KEYBOARD = 0,
    COMM_CHANNEL_MOUSE,
    COMM_CHANNEL_NIC,
    COMM_CHANNEL_MASS_STORAGE,
    COMM_CHANNEL_BLUETOOTH,
    COMM_CHANNEL_WIFI,
    COMM_CHANNEL_HID_RAW,
    COMM_CHANNEL_COUNT
} CommChannelType;

typedef enum {
    CHANNEL_STATUS_UNAVAILABLE = 0,
    CHANNEL_STATUS_AVAILABLE,
    CHANNEL_STATUS_CONNECTING,
    CHANNEL_STATUS_CONNECTED,
    CHANNEL_STATUS_ERROR,
    CHANNEL_STATUS_DISCONNECTED
} ChannelStatus;

typedef struct {
    ChannelStatus status;
    uint8_t priority;          // Lower number = higher priority
    uint32_t speed_kbps;       // Theoretical throughput in Kbps
    uint8_t stealth_rating;    // 0-10 (higher = stealthier)
    uint8_t reliability;       // 0-10 (higher = more reliable)
    uint32_t last_used;        // Timestamp
    uint32_t error_count;      // Number of errors encountered
    bool is_bidirectional;     // Can both send and receive data
} CommChannelState;

// Global communication state
extern CommChannelState comm_channels[COMM_CHANNEL_COUNT];

// Initialize communication manager
void comm_manager_init(void);

// Get best available channel for a specific purpose
CommChannelType comm_get_best_channel(uint8_t min_stealth, uint32_t min_speed, bool require_bidirectional);

// Attempt to establish a specific channel
bool comm_establish_channel(CommChannelType channel, uint32_t timeout_ms);

// Send data through best available channel (with fallbacks)
bool comm_send_data(const void* data, size_t length, CommChannelType preferred_channel);

// Check if specific channel is connected
bool comm_is_channel_connected(CommChannelType channel);

// Register callback for channel status changes
void comm_register_status_callback(void (*callback)(CommChannelType channel, ChannelStatus old_status, ChannelStatus new_status));