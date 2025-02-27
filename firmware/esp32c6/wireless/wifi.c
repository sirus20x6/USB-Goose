/**
 * wifi.c - Wi-Fi implementation for ESP32-C6
 */

 #include "wifi.h"
 #include "esp_wifi.h"
 #include "esp_event.h"
 #include "esp_log.h"
 #include <string.h>
 
 // Tag for ESP logging
 static const char* TAG = "WIFI";
 
 // Wi-Fi status variables
 static bool wifi_initialized = false;
 static bool wifi_connected = false;
 static wifi_mode_t current_mode = WIFI_MODE_NULL;
 
 // Scan results
 static wifi_ap_record_t* ap_records = NULL;
 static uint16_t ap_count = 0;
 static bool scan_in_progress = false;
 
 // Forward declarations
 static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data);
 
 // Initialize Wi-Fi
 bool wifi_init(void) {
     if (wifi_initialized) {
         return true;
     }
     
     // Initialize TCP/IP stack
     ESP_ERROR_CHECK(esp_netif_init());
     
     // Initialize event loop
     ESP_ERROR_CHECK(esp_event_loop_create_default());
     
     // Create default netif instances
     esp_netif_create_default_wifi_sta();
     esp_netif_create_default_wifi_ap();
     
     // Initialize Wi-Fi
     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
     ESP_ERROR_CHECK(esp_wifi_init(&cfg));
     
     // Register event handlers
     ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
                                               &wifi_event_handler, NULL));
     ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, 
                                               &wifi_event_handler, NULL));
     
     wifi_initialized = true;
     ESP_LOGI(TAG, "Wi-Fi initialized");
     
     return true;
 }
 
 // Start Wi-Fi in station mode
 bool wifi_start_sta(void) {
     if (!wifi_initialized) {
         if (!wifi_init()) {
             return false;
         }
     }
     
     // Configure in station mode
     wifi_config_t wifi_config = {
         .sta = {
             .scan_method = WIFI_FAST_SCAN,
             .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
             .threshold.rssi = -127,
             .threshold.authmode = WIFI_AUTH_OPEN
         }
     };
     
     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
     ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
     ESP_ERROR_CHECK(esp_wifi_start());
     
     current_mode = WIFI_MODE_STA;
     ESP_LOGI(TAG, "Wi-Fi started in station mode");
     
     return true;
 }
 
 // Start Wi-Fi in AP mode
 bool wifi_start_ap(const char* ssid, const char* password) {
     if (!wifi_initialized) {
         if (!wifi_init()) {
             return false;
         }
     }
     
     // Configure in AP mode
     wifi_config_t wifi_config = {
         .ap = {
             .max_connection = 4,
             .authmode = password ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN
         }
     };
     
     // Set SSID and password
     strncpy((char*)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid) - 1);
     wifi_config.ap.ssid_len = strlen(ssid);
     
     if (password) {
         strncpy((char*)wifi_config.ap.password, password, sizeof(wifi_config.ap.password) - 1);
     }
     
     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
     ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
     ESP_ERROR_CHECK(esp_wifi_start());
     
     current_mode = WIFI_MODE_AP;
     ESP_LOGI(TAG, "Wi-Fi started in AP mode with SSID: %s", ssid);
     
     return true;
 }
 
 // Connect to a specific network
 bool wifi_connect(const char* ssid, const char* password) {
     if (current_mode != WIFI_MODE_STA) {
         if (!wifi_start_sta()) {
             return false;
         }
     }
     
     // Configure connection details
     wifi_config_t wifi_config = {0};
     strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
     
     if (password) {
         strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
     }
     
     ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
     ESP_ERROR_CHECK(esp_wifi_connect());
     
     ESP_LOGI(TAG, "Connecting to %s...", ssid);
     
     // Connection status will be updated in event handler
     return true;
 }
 
 // Disconnect from network
 bool wifi_disconnect(void) {
     if (current_mode != WIFI_MODE_STA || !wifi_connected) {
         return true;
     }
     
     ESP_ERROR_CHECK(esp_wifi_disconnect());
     wifi_connected = false;
     
     ESP_LOGI(TAG, "Disconnected from Wi-Fi");
     return true;
 }
 
 // Scan for networks
 bool wifi_scan_start(void) {
     if (scan_in_progress) {
         return false;
     }
     
     if (current_mode != WIFI_MODE_STA) {
         if (!wifi_start_sta()) {
             return false;
         }
     }
     
     // Free previous scan results if any
     if (ap_records) {
         free(ap_records);
         ap_records = NULL;
         ap_count = 0;
     }
     
     // Configure scan parameters
     wifi_scan_config_t scan_config = {
         .ssid = NULL,
         .bssid = NULL,
         .channel = 0,
         .show_hidden = true,
         .scan_type = WIFI_SCAN_TYPE_ACTIVE,
         .scan_time.active.min = 100,
         .scan_time.active.max = 300
     };
     
     ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, false));
     scan_in_progress = true;
     
     ESP_LOGI(TAG, "Wi-Fi scan started");
     return true;
 }
 
 // Get scan results
 uint16_t wifi_scan_get_results(WiFiNetwork* networks, uint16_t max_networks) {
     if (scan_in_progress || !ap_records) {
         return 0;
     }
     
     uint16_t count = (max_networks < ap_count) ? max_networks : ap_count;
     
     // Copy scan results to output buffer
     for (uint16_t i = 0; i < count; i++) {
         strncpy(networks[i].ssid, (char*)ap_records[i].ssid, sizeof(networks[i].ssid) - 1);
         networks[i].ssid[sizeof(networks[i].ssid) - 1] = '\0';
         
         memcpy(networks[i].bssid, ap_records[i].bssid, 6);
         networks[i].channel = ap_records[i].primary;
         networks[i].rssi = ap_records[i].rssi;
         networks[i].auth_mode = ap_records[i].authmode;
         networks[i].is_hidden = (ap_records[i].ssid[0] == '\0');
     }
     
     return count;
 }
 
 // Check if Wi-Fi is connected
 bool wifi_is_connected(void) {
     return wifi_connected;
 }
 
 // Get current Wi-Fi mode
 wifi_mode_t wifi_get_mode(void) {
     return current_mode;
 }
 
 // Stop Wi-Fi
 bool wifi_stop(void) {
     if (!wifi_initialized) {
         return true;
     }
     
     ESP_ERROR_CHECK(esp_wifi_stop());
     current_mode = WIFI_MODE_NULL;
     wifi_connected = false;
     
     ESP_LOGI(TAG, "Wi-Fi stopped");
     return true;
 }
 
 // Wi-Fi event handler
 static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
     // Handle Wi-Fi events
     if (event_base == WIFI_EVENT) {
         switch (event_id) {
             case WIFI_EVENT_SCAN_DONE: {
                 scan_in_progress = false;
                 
                 // Get number of APs found
                 uint16_t ap_num = 0;
                 ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_num));
                 
                 // Allocate memory for AP records
                 if (ap_records) {
                     free(ap_records);
                 }
                 
                 ap_records = malloc(sizeof(wifi_ap_record_t) * ap_num);
                 
                 if (!ap_records) {
                     ESP_LOGE(TAG, "Failed to allocate memory for AP records");
                     ap_count = 0;
                     return;
                 }
                 
                 // Get AP records
                 ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));
                 ap_count = ap_num;
                 
                 ESP_LOGI(TAG, "Scan completed, found %d APs", ap_count);
                 
                 // Optionally trigger protocol event
                 uint8_t event_data = ap_count;
                 protocol_trigger_event(EVENT_WIFI_SCAN_DONE, &event_data, sizeof(event_data));
                 break;
             }
             
             case WIFI_EVENT_STA_START:
                 ESP_LOGI(TAG, "Station started");
                 break;
                 
             case WIFI_EVENT_STA_CONNECTED:
                 ESP_LOGI(TAG, "Connected to AP");
                 break;
                 
             case WIFI_EVENT_STA_DISCONNECTED: {
                 wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*)event_data;
                 ESP_LOGI(TAG, "Disconnected from AP, reason: %d", event->reason);
                 wifi_connected = false;
                 
                 // Trigger protocol event
                 protocol_trigger_event(EVENT_WIFI_DISCONNECTED, NULL, 0);
                 break;
             }
             
             case WIFI_EVENT_AP_START:
                 ESP_LOGI(TAG, "AP started");
                 break;
                 
             case WIFI_EVENT_AP_STACONNECTED: {
                 wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*)event_data;
                 ESP_LOGI(TAG, "Station connected to AP, MAC: " MACSTR, MAC2STR(event->mac));
                 break;
             }
             
             case WIFI_EVENT_AP_STADISCONNECTED: {
                 wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*)event_data;
                 ESP_LOGI(TAG, "Station disconnected from AP, MAC: " MACSTR, MAC2STR(event->mac));
                 break;
             }
         }
     }
     
     // Handle IP events
     if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
         ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
         ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
         wifi_connected = true;
         
         // Trigger protocol event
         protocol_trigger_event(EVENT_WIFI_CONNECTED, NULL, 0);
     }
 }