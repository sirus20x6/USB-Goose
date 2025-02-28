/**
 * @file goose/core.h
 * @brief Core definitions and functions for USB Goose project
 * 
 * This file contains the primary API for the USB Goose project,
 * providing easy access to the most commonly used features.
 */

 #ifndef GOOSE_CORE_H
 #define GOOSE_CORE_H
 
 #include <stdint.h>
 #include <stdbool.h>
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /**
  * @brief Operation mode for the device
  */
 typedef enum {
     GOOSE_MODE_STANDARD, /**< Standard mode: USB HID + Mass Storage, passive wireless monitoring */
     GOOSE_MODE_ADMIN,    /**< Admin mode: Configuration via Wi-Fi AP, exposes hidden partitions */
     GOOSE_MODE_STEALTH   /**< Stealth mode: Minimal indicators, covert operation */
 } goose_mode_t;
 
 /**
  * @brief Communication channel priority
  */
 typedef enum {
     GOOSE_PRIORITY_SPEED,    /**< Prioritize speed over stealth */
     GOOSE_PRIORITY_STEALTH,  /**< Prioritize stealth over speed */
     GOOSE_PRIORITY_RELIABILITY /**< Prioritize reliability over speed and stealth */
 } goose_priority_t;
 
 /**
  * @brief Target operating system
  */
 typedef enum {
     GOOSE_OS_UNKNOWN,
     GOOSE_OS_WINDOWS,
     GOOSE_OS_MACOS,
     GOOSE_OS_LINUX,
     GOOSE_OS_ANDROID,
     GOOSE_OS_IOS,
     GOOSE_OS_CHROMEOS
 } goose_os_t;
 
 /**
  * @brief Initialize the USB Goose
  * 
  * @param mode Initial operation mode
  * @return true if initialization successful, false otherwise
  */
 bool goose_init(goose_mode_t mode);
 
 /**
  * @brief Shutdown the USB Goose cleanly
  */
 void goose_shutdown(void);
 
 /**
  * @brief Change the current operation mode
  * 
  * @param mode New operation mode
  * @return true if mode change successful, false otherwise
  */
 bool goose_set_mode(goose_mode_t mode);
 
 /**
  * @brief Get the current operation mode
  * 
  * @return Current operation mode
  */
 goose_mode_t goose_get_mode(void);
 
 /**
  * @brief Detect the host operating system
  * 
  * @return Detected operating system
  */
 goose_os_t goose_detect_os(void);
 
 /**
  * @brief Check if the device has an active internet connection
  * 
  * @return true if internet is available, false otherwise
  */
 bool goose_has_internet(void);
 
 /**
  * @brief Set LED behavior
  * 
  * @param enable true to enable LEDs, false to disable
  */
 void goose_set_leds(bool enable);
 
 /**
  * @brief Check if the device is in a detected state
  * 
  * This checks various signals to determine if the device might 
  * have been detected as malicious.
  * 
  * @return Risk level from 0 (not detected) to 100 (confirmed detected)
  */
 uint8_t goose_detection_risk(void);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* GOOSE_CORE_H */