/**
 * spi_slave.c - SPI slave implementation for CH569
 */

#include "spi_slave.h"
#include "ch569_spi.h"  // Vendor-specific SPI headers
#include "protocol.h"
#include <string.h>

// Define buffer sizes
#define RX_BUFFER_SIZE 512
#define TX_BUFFER_SIZE 512

// Buffer definitions
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint8_t tx_buffer[TX_BUFFER_SIZE];
static uint16_t rx_write_pos = 0;
static uint16_t rx_read_pos = 0;
static uint16_t tx_write_pos = 0;
static uint16_t tx_read_pos = 0;
static bool initialized = false;

// Mutex for buffer access (if RTOS is used)
static void* rx_mutex = NULL;
static void* tx_mutex = NULL;

// SPI interrupt handler - would be registered with the actual SPI peripheral
void spi_slave_irq_handler(void) {
    // Check if this is a receive interrupt
    if (spi_is_rx_ready()) {
        uint8_t data = spi_read_byte();
        
        // Lock RX buffer if mutex available
        if (rx_mutex) lock_mutex(rx_mutex);
        
        // Add to RX buffer if space available
        if ((rx_write_pos + 1) % RX_BUFFER_SIZE != rx_read_pos) {
            rx_buffer[rx_write_pos] = data;
            rx_write_pos = (rx_write_pos + 1) % RX_BUFFER_SIZE;
        }
        
        // Unlock RX buffer
        if (rx_mutex) unlock_mutex(rx_mutex);
    }
    
    // Check if this is a transmit interrupt
    if (spi_is_tx_ready()) {
        // Lock TX buffer if mutex available
        if (tx_mutex) lock_mutex(tx_mutex);
        
        // Send data from TX buffer if available
        if (tx_read_pos != tx_write_pos) {
            spi_write_byte(tx_buffer[tx_read_pos]);
            tx_read_pos = (tx_read_pos + 1) % TX_BUFFER_SIZE;
        } else {
            // No data to send, put dummy byte
            spi_write_byte(0xFF);
        }
        
        // Unlock TX buffer
        if (tx_mutex) unlock_mutex(tx_mutex);
    }
}

// Initialize SPI slave
bool spi_slave_init(void) {
    // Initialize SPI in slave mode
    spi_init_slave();
    
    // Set up SPI interrupt
    spi_set_interrupt_handler(spi_slave_irq_handler);
    
    // Clear buffers
    rx_write_pos = rx_read_pos = 0;
    tx_write_pos = tx_read_pos = 0;
    
    // Create mutexes if RTOS is used
    // rx_mutex = create_mutex();
    // tx_mutex = create_mutex();
    
    // Register with protocol layer
    ProtocolTransport transport = {
        .send = spi_slave_send,
        .receive = spi_slave_receive,
        .get_time_ms = spi_slave_get_time_ms,
        .delay_ms = spi_slave_delay_ms
    };
    protocol_init(&transport);
    
    initialized = true;
    return true;
}

// Send data over SPI
bool spi_slave_send(const void* data, size_t length) {
    if (!initialized || data == NULL || length == 0) {
        return false;
    }
    
    const uint8_t* bytes = (const uint8_t*)data;
    
    // Lock TX buffer if mutex available
    if (tx_mutex) lock_mutex(tx_mutex);
    
    // Check if there's enough space
    uint16_t available = (tx_read_pos > tx_write_pos) 
                        ? tx_read_pos - tx_write_pos 
                        : TX_BUFFER_SIZE - (tx_write_pos - tx_read_pos);
    
    if (available < length) {
        // Not enough space in buffer
        if (tx_mutex) unlock_mutex(tx_mutex);
        return false;
    }
    
    // Copy data to TX buffer
    for (size_t i = 0; i < length; i++) {
        tx_buffer[tx_write_pos] = bytes[i];
        tx_write_pos = (tx_write_pos + 1) % TX_BUFFER_SIZE;
    }
    
    // Unlock TX buffer
    if (tx_mutex) unlock_mutex(tx_mutex);
    
    // Trigger SPI transmission if needed
    spi_trigger_tx();
    
    return true;
}

// Receive data over SPI
bool spi_slave_receive(void* buffer, size_t buffer_size, size_t* received, uint32_t timeout_ms) {
    if (!initialized || buffer == NULL || buffer_size == 0 || received == NULL) {
        return false;
    }
    
    uint8_t* bytes = (uint8_t*)buffer;
    *received = 0;
    
    uint32_t start_time = spi_slave_get_time_ms();
    
    while (*received < buffer_size) {
        // Check timeout
        if (spi_slave_get_time_ms() - start_time > timeout_ms) {
            return *received > 0;
        }
        
        // Lock RX buffer if mutex available
        if (rx_mutex) lock_mutex(rx_mutex);
        
        // Check if data is available
        if (rx_read_pos != rx_write_pos) {
            bytes[*received] = rx_buffer[rx_read_pos];
            rx_read_pos = (rx_read_pos + 1) % RX_BUFFER_SIZE;
            (*received)++;
        } else {
            // No data available
            if (rx_mutex) unlock_mutex(rx_mutex);
            
            // If we've received something, return success
            if (*received > 0) {
                return true;
            }
            
            // Otherwise, wait a bit and try again
            spi_slave_delay_ms(1);
            continue;
        }
        
        // Unlock RX buffer
        if (rx_mutex) unlock_mutex(rx_mutex);
    }
    
    return true;
}

// Get current time in milliseconds
uint32_t spi_slave_get_time_ms(void) {
    return get_system_time_ms(); // System-specific function
}

// Delay for specified milliseconds
void spi_slave_delay_ms(uint32_t ms) {
    delay_ms(ms); // System-specific function
}