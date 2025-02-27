/**
 * C Example: Basic OS Detection + Action
 *
 * This payload detects the operating system and then
 * executes different actions based on the OS detected.
 */

// C payload (usb_payloads/os_detect_basic.c)
#include "payload_framework.h"

// Define payload metadata
PAYLOAD_INFO("Basic OS Detection + Action", "1.0", PAYLOAD_PRIORITY_NORMAL);

void payload_main(PayloadContext* ctx) {
    // Set LED to yellow during detection
    StatusLED.setColor(255, 255, 0, 0);
    
    // Detect OS
    OperatingSystem os = OSDetect.detect(OS_DETECT_THOROUGH);
    ctx->detected_os = os;
    
    // Show detection confidence
    System.log("OS Detection confidence: %d%%", OSDetect.getConfidence());
    
    // Get hostname if available
    char hostname[32];
    if (OSDetect.getHostname(hostname, sizeof(hostname))) {
        System.log("Hostname: %s", hostname);
        strncpy(ctx->hostname, hostname, sizeof(ctx->hostname) - 1);
    }
    
    // Different actions based on detected OS
    switch (os) {
        case OS_WINDOWS:
            StatusLED.setColor(0, 0, 255, 0); // Blue
            System.log("Windows detected - opening PowerShell");
            
            // Send Win+R to open Run dialog
            KeyboardHID.tapWithModifiers(KEY_R, MOD_LMETA);
            System.delay(500);
            
            // Type PowerShell and press Enter
            KeyboardHID.sendString("powershell");
            KeyboardHID.tap(KEY_RETURN);
            System.delay(1000);
            
            // Execute a simple command
            KeyboardHID.sendLine("Get-ComputerInfo | Select-Object OsName, OsVersion");
            break;
            
        case OS_MACOS:
            StatusLED.setColor(0, 255, 0, 0); // Green
            System.log("macOS detected - opening Terminal");
            
            // Open Terminal with keyboard shortcut
            KeyboardHID.tapWithModifiers(KEY_SPACE, MOD_LMETA);
            System.delay(300);
            KeyboardHID.sendString("terminal");
            System.delay(300);
            KeyboardHID.tap(KEY_RETURN);
            System.delay(1000);
            
            // Execute a simple command
            KeyboardHID.sendLine("sw_vers");
            break;
            
        case OS_LINUX:
            StatusLED.setColor(255, 0, 0, 0); // Red
            System.log("Linux detected - opening Terminal");
            
            // Open Terminal with keyboard shortcut (varies by distro, using common ones)
            KeyboardHID.tapWithModifiers(KEY_T, MOD_LCTRL | MOD_LALT);
            System.delay(1000);
            
            // Execute a simple command
            KeyboardHID.sendLine("uname -a && cat /etc/os-release");
            break;
            
        default:
            StatusLED.setColor(255, 0, 255, 0); // Purple
            System.log("Unknown OS or detection failed");
            break;
    }
    
    // Wait 5 seconds before completing
    System.delay(5000);
    StatusLED.off();
}