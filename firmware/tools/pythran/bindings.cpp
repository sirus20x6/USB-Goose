/**
 * bindings.cpp - C/C++ bindings for Python payloads
 * 
 * This file provides the bridge between compiled Python code and 
 * the USB Goose payload framework API.
 */

 #include "payload_framework.h"
 #include <string.h>
 
 extern "C" {
 
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
 
 void led_off() {
     StatusLED.off();
 }
 
 // ======== System Bindings ========
 void system_delay(int ms) {
     System.delay(ms);
 }
 
 void log_message(const char* message) {
     System.log("%s", message);
 }
 
 bool system_is_usb_controller() {
     return System.isUSBController();
 }
 
 bool system_is_wireless_controller() {
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
 
 int os_get_confidence() {
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
 
 bool interchip_is_connected() {
     return ChipComm.isConnected();
 }
 
 // ======== Payload Entry Point ========
 // This is the bridge between the compiled Python code and the framework
 void payload_main(PayloadContext* ctx) {
     // Create a dictionary-like structure for Python context
     typedef struct {
         int detected_os;
         int security_level;
         unsigned int target_capabilities;
         char hostname[32];
         char username[32];
         unsigned long long timestamp;
     } PyContext;
     
     // Initialize Python context from C context
     PyContext py_ctx;
     py_ctx.detected_os = ctx->detected_os;
     py_ctx.security_level = ctx->security_level;
     py_ctx.target_capabilities = ctx->target_capabilities;
     strncpy(py_ctx.hostname, ctx->hostname, sizeof(py_ctx.hostname) - 1);
     strncpy(py_ctx.username, ctx->username, sizeof(py_ctx.username) - 1);
     py_ctx.timestamp = ctx->timestamp;
     
     // Call the pythran-generated payload_main function
     // Note: The actual call is implemented by Pythran and will vary
     // based on the exported function signature
     // We expect: int payload_main(PyContext py_ctx)
     int result = payload_main_python(py_ctx);
     
     // Could process result if needed
     (void)result;
 }
 
 // For compatibility with Pythran's export requirements
 // Modification of the context
 int payload_main_python(PyContext& ctx) {
     // This function is defined by the pythran-compiled code
     // It will be implemented in the generated C++ file
     return 0;
 }
 
 } // extern "C"