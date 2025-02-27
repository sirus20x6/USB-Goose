/**
 * spi_master.c - SPI master implementation for ESP32-C6
 */

 #include "spi_master.h"
 #include "esp_spi.h"  // ESP-IDF SPI driver
 #include "protocol.h"
 #include <string.h>
 
 // Define buffer sizes
 #define RX_BUFFER_SIZE 512
 #define TX_BUFFER_SIZE 512
 
 // SPI device handle
 static spi_device_handle_t spi_handle = NULL;
 static bool initialized = false;
 
 // Initialize SPI master
 bool spi_master_init(void) {
     esp_err_t ret;
     
     // Configure SPI bus
     spi_bus_config_t bus_config = {
         .miso_io_num = SPI_MISO_PIN,
         .mosi_io_num = SPI_MOSI_PIN,
         .sclk_io_num = SPI_CLK_PIN,
         .quadwp_io_num = -1,
         .quadhd_io_num = -1,
         .max_transfer_sz = 4096
     };
     
     // Initialize SPI bus
     ret = spi_bus_initialize(SPI_HOST, &bus_config, SPI_DMA_CH_AUTO);
     if (ret != ESP_OK) {
         return false;
     }
     
     // Configure SPI device
     spi_device_interface_config_t dev_config = {
         .clock_speed_hz = 10000000, // 10 MHz
         .mode = 0,                  // SPI mode 0
         .spics_io_num = SPI_CS_PIN,
         .queue_size = 8,
         .flags = 0,
         .pre_cb = NULL,
         .post_cb = NULL
     };
     
     // Add SPI device to bus
     ret = spi_bus_add_device(SPI_HOST, &dev_config, &spi_handle);
     if (ret != ESP_OK) {
         spi_bus_free(SPI_HOST);
         return false;
     }
     
     // Register with protocol layer
     ProtocolTransport transport = {
         .send = spi_master_send,
         .receive = spi_master_receive,
         .get_time_ms = spi_master_get_time_ms,
         .delay_ms = spi_master_delay_ms
     };
     protocol_init(&transport);
     
     initialized = true;
     return true;
 }
 
 // Send data over SPI
 bool spi_master_send(const void* data, size_t length) {
     if (!initialized || data == NULL || length == 0) {
         return false;
     }
     
     esp_err_t ret;
     spi_transaction_t trans = {0};
     
     // Set up transaction
     trans.length = length * 8;  // in bits
     trans.tx_buffer = data;
     trans.flags = 0;
     
     // Execute transaction
     ret = spi_device_transmit(spi_handle, &trans);
     return (ret == ESP_OK);
 }
 
 // Receive data over SPI
 bool spi_master_receive(void* buffer, size_t buffer_size, size_t* received, uint32_t timeout_ms) {
     if (!initialized || buffer == NULL || buffer_size == 0 || received == NULL) {
         return false;
     }
     
     esp_err_t ret;
     spi_transaction_t trans = {0};
     
     // Set up transaction
     trans.length = buffer_size * 8;  // in bits
     trans.rxlength = buffer_size * 8;  // in bits
     trans.rx_buffer = buffer;
     trans.flags = 0;
     
     // Execute transaction with timeout
     uint32_t start_time = spi_master_get_time_ms();
     
     while (spi_master_get_time_ms() - start_time < timeout_ms) {
         ret = spi_device_polling_transmit(spi_handle, &trans);
         if (ret == ESP_OK) {
             *received = trans.rxlength / 8;  // convert bits to bytes
             return true;
         }
         
         // Wait a bit before retrying
         spi_master_delay_ms(1);
     }
     
     *received = 0;
     return false;
 }
 
 // Get current time in milliseconds
 uint32_t spi_master_get_time_ms(void) {
     return esp_timer_get_time() / 1000;  // Convert microseconds to milliseconds
 }
 
 // Delay for specified milliseconds
 void spi_master_delay_ms(uint32_t ms) {
     vTaskDelay(ms / portTICK_PERIOD_MS);
 }