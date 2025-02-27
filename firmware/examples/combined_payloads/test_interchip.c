/**
 * test_interchip.c - Test payload for inter-chip communication
 */

 #include "payload_framework.h"

 // Define payload metadata
 PAYLOAD_INFO("Inter-Chip Test", "1.0", PAYLOAD_PRIORITY_NORMAL);
 
 void payload_main(PayloadContext* ctx) {
     // Identify which chip we're running on
     bool is_usb_controller = System.isUSBController();
     System.log("Running on %s controller", is_usb_controller ? "USB" : "Wireless");
     
     // Set LED color to indicate chip
     if (is_usb_controller) {
         StatusLED.setColor(0, 0, 255, 0); // Blue for USB controller
     } else {
         StatusLED.setColor(0, 255, 0, 0); // Green for Wireless controller
     }
     
     // Test inter-chip communication
     if (is_usb_controller) {
         // USB controller sends event
         System.log("USB controller: Sending event to wireless controller");
         
         // Prepare data to send
         uint8_t test_data[] = {1, 2, 3, 4, 5};
         
         // Send event
         if (ChipComm.triggerEvent(EVENT_CUSTOM_BASE, test_data, sizeof(test_data))) {
             System.log("USB controller: Event sent successfully");
         } else {
             System.log("USB controller: Failed to send event");
             StatusLED.setColor(255, 0, 0, 0); // Red for error
             return;
         }
         
         // Wait for response event
         System.log("USB controller: Waiting for response event");
         if (ChipComm.waitForEvent(EVENT_CUSTOM_BASE + 1, 5000)) {
             System.log("USB controller: Response received!");
             StatusLED.setColor(255, 255, 0, 0); // Yellow for success
         } else {
             System.log("USB controller: No response received");
             StatusLED.setColor(255, 0, 0, 0); // Red for error
         }
     } else {
         // Wireless controller waits for event
         System.log("Wireless controller: Waiting for event from USB controller");
         
         // Register event handler
         System.log("Wireless controller: Registering event handler");
         ChipComm.registerHandler(EVENT_CUSTOM_BASE, handle_test_event);
         
         // Just wait for event handler to be called
         System.delay(5000);
     }
     
     // Turn off LED when done
     System.delay(1000);
     StatusLED.off();
 }
 
 // Event handler for wireless controller
 void handle_test_event(const void* data, size_t data_len) {
     System.log("Wireless controller: Received event with %d bytes", data_len);
     
     // Print received data
     if (data != NULL && data_len > 0) {
         const uint8_t* bytes = (const uint8_t*)data;
         System.log("Wireless controller: Data: [%d, %d, %d, ...]", 
                   bytes[0], bytes[1], bytes[2]);
     }
     
     // Send response event
     System.log("Wireless controller: Sending response event");
     ChipComm.triggerEvent(EVENT_CUSTOM_BASE + 1, NULL, 0);
     
     // Set LED to indicate success
     StatusLED.setColor(255, 255, 0, 0); // Yellow for success
 }