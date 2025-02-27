/**
 * bindings.c - C bindings for Nim payloads
 * 
 * This file provides the bridge between Nim code and 
 * the USB Goose payload framework API.
 */

 #include "payload_framework.h"
 #include <string.h>
 
 // ======== Keyboard Bindings ========
 void keyboard_press(int key) {
     KeyboardHID.press((KeyCode)key);
 }
 
 void keyboard_release(int key) {
     KeyboardHID.release((KeyCode)key);
 }
 
 void keyboard_tap(int key) {
     KeyboardHID.tap((KeyCode)key);
 }
 
 void keyboard_press_modifier(int modifiers) {
     KeyboardHID.pressModifier(modifiers);
 }
 
 void keyboard_release_modifier(int modifiers) {
     KeyboardHID.releaseModifier(modifiers);
 }
 
 void keyboard_tap_with_modifiers(int key, int modifiers) {
     KeyboardHID.tapWithModifiers((KeyCode)key, modifiers);
 }
 
 void keyboard_send_string(const char* text) {
     KeyboardHID.sendString(text);
 }
 
 void keyboard_send_line(const char* text) {
     KeyboardHID.sendLine(text);
 }
 
 void keyboard_send_win_key(char key) {
     KeyboardHID.sendWinKey(key);
 }
 
 void keyboard_send_command_key(char key) {
     KeyboardHID.sendCommandKey(key);
 }
 
 void keyboard_send_ctrl_key(char key) {
     KeyboardHID.sendCtrlKey(key);
 }
 
 // ======== LED Control Bindings ========
 void led_set_color(int r, int g, int b, int w) {
     StatusLED.setColor(r, g, b, w);
 }
 
 void led_set_pattern(int pattern) {
     StatusLED.setPattern(pattern);
 }
 
 void led_set_brightness(int brightness) {
     StatusLED.setBrightness(brightness);
 }
 
 void led_off(void) {
     StatusLED.off();
 }
 
 // ======== System Bindings ========
 void system_delay(int ms) {
     System.delay(ms);
 }
 
 void log_message(const char* message) {
     System.log("%s", message);
 }
 
 bool system_is_usb_controller(void) {
     return System.isUSBController();
 }
 
 bool system_is_wireless_controller(void) {
     return System.isWirelessController();
 }
 
 // ======== OS Detection Bindings ========
 int os_detect(int flags) {
     return (int)OSDetect.detect(flags);
 }
 
 bool os_get_hostname(char* buffer, int max_len) {
     return OSDetect.getHostname(buffer, max_len);
 }
 
 bool os_get_username(char* buffer, int max_len) {
     return OSDetect.getUsername(buffer, max_len);
 }
 
 int os_get_confidence(void) {
     return OSDetect.getConfidence();
 }
 
 // ======== Inter-Chip Communication Bindings ========
 bool interchip_trigger_event(int event, const void* data, int data_len) {
     return ChipComm.triggerEvent((EventType)event, data, data_len);
 }
 
 bool interchip_wait_for_event(int event, int timeout_ms) {
     return ChipComm.waitForEvent((EventType)event, timeout_ms);
 }
 
 bool interchip_send_data(int channel, const void* data, int data_len) {
     return ChipComm.sendData(channel, data, data_len);
 }
 
 bool interchip_receive_data(int channel, void* buffer, int buffer_len, int* received_len) {
     size_t received = 0;
     bool result = ChipComm.receiveData(channel, buffer, buffer_len, &received);
     if (received_len) {
         *received_len = received;
     }
     return result;
 }
 
 bool interchip_is_connected(void) {
     return ChipComm.isConnected();
 }