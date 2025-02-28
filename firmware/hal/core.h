/**
 * @file hal/core.h
 * @brief Core definitions and functions for USB Goose HAL
 * 
 * This file contains the primary API for the USB Goose hardware
 * abstraction layer, providing platform-independent access
 * to the device functionality.
 */

 #ifndef HAL_CORE_H
 #define HAL_CORE_H
 
 #include <stdint.h>
 #include <stdbool.h>
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /**
  * @brief Operation mode for the device
  */
 typedef enum {
     MODE_STANDARD, /**< Standard mode: USB HID + Mass Storage, passive wireless monitoring */
     MODE_ADMIN,    /**< Admin mode: Configuration via Wi-Fi AP, exposes hidden partitions */
     MODE_STEALTH   /**< Stealth mode: Minimal indicators, covert operation */
 } device_mode_t;
 
 /**
  * @brief Communication channel priority
  */
 typedef enum {
     PRIORITY_SPEED,     /**< Prioritize speed over stealth */
     PRIORITY_STEALTH,   /**< Prioritize stealth over speed */
     PRIORITY_RELIABILITY /**< Prioritize reliability over speed and stealth */
 } channel_priority_t;
 
 /**
  * @brief Target operating system
  */
 typedef enum {
     OS_UNKNOWN,
     OS_WINDOWS,
     OS_MACOS,
     OS_LINUX,
     OS_ANDROID,
     OS_IOS,
     OS_CHROMEOS
 } os_type_t;
 
 /**
  * @brief Initialize the device
  * 
  * @param mode Initial operation mode
  * @return true if initialization successful, false otherwise
  */
 bool init(device_mode_t mode);
 
 /**
  * @brief Shutdown the device cleanly
  */
 void shutdown(void);
 
 /**
  * @brief Change the current operation mode
  * 
  * @param mode New operation mode
  * @return true if mode change successful, false otherwise
  */
 bool set_mode(device_mode_t mode);
 
 /**
  * @brief Get the current operation mode
  * 
  * @return Current operation mode
  */
 device_mode_t get_mode(void);
 
 /**
  * @brief Detect the host operating system
  * 
  * @return Detected operating system
  */
 os_type_t detect_os(void);
 
 /**
  * @brief Check if the device has an active internet connection
  * 
  * @return true if internet is available, false otherwise
  */
 bool has_internet(void);
 
 /**
  * @brief Set LED behavior
  * 
  * @param enable true to enable LEDs, false to disable
  */
 void set_leds(bool enable);
 
 /**
  * @brief Check if the device is in a detected state
  * 
  * This checks various signals to determine if the device might 
  * have been detected as malicious.
  * 
  * @return Risk level from 0 (not detected) to 100 (confirmed detected)
  */
 uint8_t detection_risk(void);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* HAL_CORE_H */