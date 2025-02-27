/**
 * protocol.c - Implementation of the inter-chip communication protocol
 */

#include "protocol.h"
#include <string.h>

// Protocol constants
#define PROTOCOL_MAGIC        0x55AA
#define PROTOCOL_VERSION      0x01
#define MAX_PACKET_SIZE       256
#define MAX_PAYLOAD_SIZE      (MAX_PACKET_SIZE - sizeof(PacketHeader))
#define MAX_RETRIES           3
#define PROTOCOL_TIMEOUT_MS   100

// Packet header structure
typedef struct {
    uint16_t magic;           // Magic value for packet identification
    uint8_t version;          // Protocol version
    uint8_t packet_type;      // Type of packet (command, response, event)
    uint16_t sequence;        // Sequence number for packet ordering
    uint16_t length;          // Length of data payload
    uint16_t checksum;        // CRC-16 checksum of data
} PacketHeader;

// Packet types
#define PACKET_TYPE_COMMAND   0x01
#define PACKET_TYPE_RESPONSE  0x02
#define PACKET_TYPE_EVENT     0x03
#define PACKET_TYPE_ACK       0x04
#define PACKET_TYPE_NACK      0x05

// Command types
#define CMD_PING              0x01
#define CMD_GET_STATUS        0x02
#define CMD_EXECUTE_PAYLOAD   0x03
#define CMD_TRIGGER_EVENT     0x04
#define CMD_SEND_DATA         0x05
#define CMD_RECEIVE_DATA      0x06

// Status codes
#define STATUS_SUCCESS        0x00
#define STATUS_ERROR          0x01
#define STATUS_BUSY           0x02
#define STATUS_TIMEOUT        0x03
#define STATUS_INVALID        0x04

// Static variables
static uint16_t next_sequence = 0;
static bool is_initialized = false;
static ProtocolTransport transport = {0};

// Forward declarations for internal functions
static uint16_t calculate_checksum(const uint8_t* data, uint16_t length);
static bool send_packet(uint8_t packet_type, uint8_t command, const void* data, uint16_t length);
static bool wait_for_ack(uint16_t sequence, uint32_t timeout_ms);

// Initialize the protocol with transport functions
bool protocol_init(ProtocolTransport* transport_functions) {
    if (transport_functions == NULL) {
        return false;
    }
    
    // Copy transport functions
    memcpy(&transport, transport_functions, sizeof(ProtocolTransport));
    
    // Initialize sequence number
    next_sequence = 0;
    is_initialized = true;
    
    return true;
}

// Send a command and wait for response
bool protocol_send_command(uint8_t command, const void* data, uint16_t length, 
                          void* response, uint16_t* response_length, uint32_t timeout_ms) {
    if (!is_initialized || data == NULL || length > MAX_PAYLOAD_SIZE) {
        return false;
    }
    
    // Send the command packet
    uint16_t sequence = next_sequence++;
    if (!send_packet(PACKET_TYPE_COMMAND, command, data, length)) {
        return false;
    }
    
    // Wait for ACK
    if (!wait_for_ack(sequence, timeout_ms)) {
        return false;
    }
    
    // If no response data expected, we're done
    if (response == NULL || response_length == NULL || *response_length == 0) {
        return true;
    }
    
    // Wait for response data
    uint8_t buffer[MAX_PACKET_SIZE];
    uint16_t received = 0;
    
    if (!transport.receive(buffer, sizeof(buffer), &received, timeout_ms)) {
        return false;
    }
    
    // Validate response
    if (received < sizeof(PacketHeader)) {
        return false;
    }
    
    PacketHeader* header = (PacketHeader*)buffer;
    if (header->magic != PROTOCOL_MAGIC || 
        header->packet_type != PACKET_TYPE_RESPONSE ||
        header->sequence != sequence) {
        return false;
    }
    
    // Check checksum
    uint16_t calc_checksum = calculate_checksum(buffer + sizeof(PacketHeader), header->length);
    if (calc_checksum != header->checksum) {
        return false;
    }
    
    // Copy response data
    uint16_t to_copy = (header->length < *response_length) ? header->length : *response_length;
    memcpy(response, buffer + sizeof(PacketHeader), to_copy);
    *response_length = to_copy;
    
    return true;
}

// Trigger an event on the remote chip
bool protocol_trigger_event(EventType event, const void* data, size_t data_len) {
    if (!is_initialized || data_len > MAX_PAYLOAD_SIZE - sizeof(EventType)) {
        return false;
    }
    
    // Prepare event data packet
    uint8_t buffer[MAX_PAYLOAD_SIZE];
    memcpy(buffer, &event, sizeof(EventType));
    
    if (data != NULL && data_len > 0) {
        memcpy(buffer + sizeof(EventType), data, data_len);
    }
    
    // Send the event packet
    return send_packet(PACKET_TYPE_EVENT, 0, buffer, sizeof(EventType) + data_len);
}

// Wait for an event from the remote chip
bool protocol_wait_for_event(EventType event, uint32_t timeout_ms) {
    if (!is_initialized) {
        return false;
    }
    
    uint32_t start_time = transport.get_time_ms();
    
    while (transport.get_time_ms() - start_time < timeout_ms) {
        uint8_t buffer[MAX_PACKET_SIZE];
        uint16_t received = 0;
        
        if (transport.receive(buffer, sizeof(buffer), &received, 10)) {
            if (received >= sizeof(PacketHeader)) {
                PacketHeader* header = (PacketHeader*)buffer;
                
                if (header->magic == PROTOCOL_MAGIC && 
                    header->packet_type == PACKET_TYPE_EVENT &&
                    header->length >= sizeof(EventType)) {
                    
                    // Check if this is the event we're waiting for
                    EventType received_event;
                    memcpy(&received_event, buffer + sizeof(PacketHeader), sizeof(EventType));
                    
                    if (received_event == event) {
                        // Send ACK
                        uint16_t ack_sequence = header->sequence;
                        send_packet(PACKET_TYPE_ACK, 0, &ack_sequence, sizeof(ack_sequence));
                        return true;
                    }
                }
            }
        }
        
        // Allow other operations during wait
        transport.delay_ms(1);
    }
    
    return false;
}

// Send a data chunk to the remote chip
bool protocol_send_data(uint16_t channel, const void* data, size_t data_len) {
    if (!is_initialized || data == NULL || data_len == 0) {
        return false;
    }
    
    // Send data in chunks if needed
    const uint8_t* data_ptr = (const uint8_t*)data;
    size_t remaining = data_len;
    
    while (remaining > 0) {
        // Prepare header with channel info
        uint8_t buffer[MAX_PAYLOAD_SIZE];
        memcpy(buffer, &channel, sizeof(channel));
        
        // Calculate chunk size
        size_t chunk_size = (remaining > MAX_PAYLOAD_SIZE - sizeof(channel)) 
                          ? MAX_PAYLOAD_SIZE - sizeof(channel) 
                          : remaining;
        
        // Copy data chunk
        memcpy(buffer + sizeof(channel), data_ptr, chunk_size);
        
        // Send the data packet
        if (!send_packet(PACKET_TYPE_COMMAND, CMD_SEND_DATA, buffer, sizeof(channel) + chunk_size)) {
            return false;
        }
        
        // Update pointers
        data_ptr += chunk_size;
        remaining -= chunk_size;
        
        // Short delay between chunks
        transport.delay_ms(1);
    }
    
    return true;
}

// Internal function to calculate checksum
static uint16_t calculate_checksum(const uint8_t* data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001; // CRC-16 MODBUS polynomial
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

// Internal function to send a packet
static bool send_packet(uint8_t packet_type, uint8_t command, const void* data, uint16_t length) {
    if (length > MAX_PAYLOAD_SIZE) {
        return false;
    }
    
    // Prepare packet buffer
    uint8_t buffer[MAX_PACKET_SIZE];
    PacketHeader* header = (PacketHeader*)buffer;
    
    // Fill header
    header->magic = PROTOCOL_MAGIC;
    header->version = PROTOCOL_VERSION;
    header->packet_type = packet_type;
    header->sequence = next_sequence++;
    header->length = length + 1; // +1 for command byte
    
    // Add command byte
    buffer[sizeof(PacketHeader)] = command;
    
    // Copy data if any
    if (data != NULL && length > 0) {
        memcpy(buffer + sizeof(PacketHeader) + 1, data, length);
    }
    
    // Calculate checksum
    header->checksum = calculate_checksum(buffer + sizeof(PacketHeader), header->length);
    
    // Send packet
    return transport.send(buffer, sizeof(PacketHeader) + header->length);
}

// Internal function to wait for ACK
static bool wait_for_ack(uint16_t sequence, uint32_t timeout_ms) {
    uint32_t start_time = transport.get_time_ms();
    
    while (transport.get_time_ms() - start_time < timeout_ms) {
        uint8_t buffer[MAX_PACKET_SIZE];
        uint16_t received = 0;
        
        if (transport.receive(buffer, sizeof(buffer), &received, 10)) {
            if (received >= sizeof(PacketHeader)) {
                PacketHeader* header = (PacketHeader*)buffer;
                
                if (header->magic == PROTOCOL_MAGIC && 
                    header->packet_type == PACKET_TYPE_ACK &&
                    header->length >= sizeof(uint16_t)) {
                    
                    // Check if this is the ACK for our sequence
                    uint16_t ack_sequence;
                    memcpy(&ack_sequence, buffer + sizeof(PacketHeader), sizeof(uint16_t));
                    
                    if (ack_sequence == sequence) {
                        return true;
                    }
                }
            }
        }
        
        // Allow other operations during wait
        transport.delay_ms(1);
    }
    
    return false;
}