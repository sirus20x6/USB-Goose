# os_detect.py - Python payload for OS detection
#pythran export payload_main(int, int, int, str, str, int)

"""
Example Python payload that detects the operating system and performs
different actions based on the detected OS.

This demonstrates how to use the Python bindings to the payload framework.
"""

# OS detection constants - must match the C definitions
OS_UNKNOWN = 0
OS_WINDOWS = 1
OS_MACOS = 2
OS_LINUX = 3
OS_CHROMEOS = 4
OS_ANDROID = 5
OS_IOS = 6
OS_BSD = 7

# Key codes - subset of the full definition
KEY_NONE = 0x00
KEY_A = 0x04
KEY_B = 0x05
KEY_C = 0x06
KEY_R = 0x15
KEY_T = 0x17
KEY_RETURN = 0x28
KEY_ESCAPE = 0x29
KEY_BACKSPACE = 0x2A
KEY_TAB = 0x2B
KEY_SPACE = 0x2C

# Modifier keys
MOD_NONE = 0x00
MOD_LCTRL = 0x01
MOD_LSHIFT = 0x02
MOD_LALT = 0x04
MOD_LMETA = 0x08  # Windows/Command key
MOD_RCTRL = 0x10
MOD_RSHIFT = 0x20
MOD_RALT = 0x40
MOD_RMETA = 0x80

# OS detection flags
OS_DETECT_USB_ONLY = 1
OS_DETECT_NETWORK_ONLY = 2
OS_DETECT_THOROUGH = 4
OS_DETECT_CACHED = 8

def payload_main(detected_os, security_level, target_capabilities, hostname, username, timestamp):
    """
    Main entry point for the Python payload.
    
    Args:
        detected_os: The detected operating system code
        security_level: Estimated security level (0-100)
        target_capabilities: Bitfield of target capabilities
        hostname: Target hostname (if available)
        username: Target username (if available)
        timestamp: Current timestamp
    
    Returns:
        int: Status code (0 for success)
    """
    # Set LED to yellow during initialization
    led_set_color(255, 255, 0, 0)
    
    # Log which controller we're running on
    if system_is_usb_controller():
        log_message("Running on USB controller")
    else:
        log_message("Running on Wireless controller")
    
    # Try to detect OS if not already detected
    if detected_os == OS_UNKNOWN:
        log_message("OS not detected, running detection...")
        detected_os = os_detect(OS_DETECT_THOROUGH)
    
    # Log OS detection info
    log_message(f"Detected OS: {get_os_name(detected_os)}")
    log_message(f"Detection confidence: {os_get_confidence()}%")
    
    # Log hostname if available
    if hostname:
        log_message(f"Hostname: {hostname}")
    
    # Different actions based on detected OS
    if detected_os == OS_WINDOWS:
        perform_windows_actions()
    elif detected_os == OS_MACOS:
        perform_macos_actions()
    elif detected_os == OS_LINUX:
        perform_linux_actions()
    else:
        # Unknown or unsupported OS
        led_set_color(255, 0, 255, 0)  # Purple for unknown
        log_message("Unknown or unsupported OS")
    
    # Wait 5 seconds before completing
    system_delay(5000)
    led_off()
    return 0

def perform_windows_actions():
    """Perform Windows-specific actions"""
    led_set_color(0, 0, 255, 0)  # Blue for Windows
    log_message("Performing Windows-specific actions")
    
    # Open Run dialog with Win+R
    keyboard_tap_with_modifiers(KEY_R, MOD_LMETA)
    system_delay(500)
    
    # Type PowerShell and press Enter
    keyboard_send_string("powershell")
    keyboard_tap(KEY_RETURN)
    system_delay(1000)
    
    # Execute commands
    keyboard_send_line("Get-ComputerInfo | Select-Object OsName, OsVersion")
    system_delay(1000)
    keyboard_send_line("exit")

def perform_macos_actions():
    """Perform macOS-specific actions"""
    led_set_color(0, 255, 0, 0)  # Green for macOS
    log_message("Performing macOS-specific actions")
    
    # Open Spotlight with Cmd+Space
    keyboard_tap_with_modifiers(KEY_SPACE, MOD_LMETA)
    system_delay(300)
    
    # Type Terminal and press Enter
    keyboard_send_string("terminal")
    system_delay(300)
    keyboard_tap(KEY_RETURN)
    system_delay(1000)
    
    # Execute commands
    keyboard_send_line("sw_vers")
    system_delay(500)
    keyboard_send_line("exit")

def perform_linux_actions():
    """Perform Linux-specific actions"""
    led_set_color(255, 0, 0, 0)  # Red for Linux
    log_message("Performing Linux-specific actions")
    
    # Open Terminal with Ctrl+Alt+T (common shortcut)
    keyboard_tap_with_modifiers(KEY_T, MOD_LCTRL | MOD_LALT)
    system_delay(1000)
    
    # Execute commands
    keyboard_send_line("uname -a && cat /etc/os-release")
    system_delay(500)
    keyboard_send_line("exit")

def get_os_name(os_code):
    """Convert OS code to human-readable name"""
    os_names = {
        OS_UNKNOWN: "Unknown",
        OS_WINDOWS: "Windows",
        OS_MACOS: "macOS",
        OS_LINUX: "Linux",
        OS_CHROMEOS: "ChromeOS",
        OS_ANDROID: "Android",
        OS_IOS: "iOS",
        OS_BSD: "BSD"
    }
    return os_names.get(os_code, f"Unknown OS code: {os_code}")