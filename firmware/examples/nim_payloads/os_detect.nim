# os_detect.nim - Nim payload for OS detection
#
# This example demonstrates using Nim for USB Goose payloads
# Nim provides Python-like syntax with static typing and
# compile-time features, ideal for embedded development.

# C bindings to the payload framework
{.passC: "-I../../firmware/common/include".}
{.compile: "../../tools/nim/bindings.c".}

# OS detection constants
const
  OS_UNKNOWN* = 0
  OS_WINDOWS* = 1
  OS_MACOS* = 2
  OS_LINUX* = 3
  OS_CHROMEOS* = 4
  OS_ANDROID* = 5
  OS_IOS* = 6
  OS_BSD* = 7

# Key codes
const
  KEY_NONE* = 0x00
  KEY_A* = 0x04
  KEY_B* = 0x05
  KEY_C* = 0x06
  KEY_R* = 0x15
  KEY_T* = 0x17
  KEY_RETURN* = 0x28
  KEY_ESCAPE* = 0x29
  KEY_BACKSPACE* = 0x2A
  KEY_TAB* = 0x2B
  KEY_SPACE* = 0x2C

# Modifier keys
const
  MOD_NONE* = 0x00
  MOD_LCTRL* = 0x01
  MOD_LSHIFT* = 0x02
  MOD_LALT* = 0x04
  MOD_LMETA* = 0x08  # Windows/Command key
  MOD_RCTRL* = 0x10
  MOD_RSHIFT* = 0x20
  MOD_RALT* = 0x40
  MOD_RMETA* = 0x80

# OS detection flags
const
  OS_DETECT_USB_ONLY* = 1
  OS_DETECT_NETWORK_ONLY* = 2
  OS_DETECT_THOROUGH* = 4
  OS_DETECT_CACHED* = 8

# PayloadContext structure
type
  PayloadContext* = object
    detected_os*: int
    security_level*: int
    target_capabilities*: uint32
    hostname*: array[32, char]
    username*: array[32, char]
    timestamp*: uint64
    reserved*: array[32, uint8]

# Import framework functions
proc keyboard_press*(key: cint) {.importc.}
proc keyboard_release*(key: cint) {.importc.}
proc keyboard_tap*(key: cint) {.importc.}
proc keyboard_press_modifier*(modifiers: cint) {.importc.}
proc keyboard_release_modifier*(modifiers: cint) {.importc.}
proc keyboard_tap_with_modifiers*(key: cint, modifiers: cint) {.importc.}
proc keyboard_send_string*(text: cstring) {.importc.}
proc keyboard_send_line*(text: cstring) {.importc.}
proc keyboard_send_win_key*(key: char) {.importc.}
proc keyboard_send_command_key*(key: char) {.importc.}
proc keyboard_send_ctrl_key*(key: char) {.importc.}

proc led_set_color*(r, g, b, w: cint) {.importc.}
proc led_set_pattern*(pattern: cint) {.importc.}
proc led_set_brightness*(brightness: cint) {.importc.}
proc led_off*() {.importc.}

proc system_delay*(ms: cint) {.importc.}
proc log_message*(message: cstring) {.importc.}
proc system_is_usb_controller*(): bool {.importc.}
proc system_is_wireless_controller*(): bool {.importc.}

proc os_detect*(flags: cint): cint {.importc.}
proc os_get_hostname*(buffer: cstring, max_len: cint): bool {.importc.}
proc os_get_username*(buffer: cstring, max_len: cint): bool {.importc.}
proc os_get_confidence*(): cint {.importc.}

proc interchip_trigger_event*(event: cint, data: pointer, data_len: cint): bool {.importc.}
proc interchip_wait_for_event*(event: cint, timeout_ms: cint): bool {.importc.}
proc interchip_send_data*(channel: cint, data: pointer, data_len: cint): bool {.importc.}
proc interchip_is_connected*(): bool {.importc.}

# Helper function to convert OS code to string
proc getOsName(osCode: int): string =
  case osCode:
    of OS_WINDOWS: "Windows"
    of OS_MACOS: "macOS"
    of OS_LINUX: "Linux"
    of OS_CHROMEOS: "ChromeOS"
    of OS_ANDROID: "Android"
    of OS_IOS: "iOS"
    of OS_BSD: "BSD"
    else: "Unknown"

# OS-specific action functions
proc performWindowsActions() =
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

proc performMacOsActions() =
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

proc performLinuxActions() =
  led_set_color(255, 0, 0, 0)  # Red for Linux
  log_message("Performing Linux-specific actions")
  
  # Open Terminal with Ctrl+Alt+T (common shortcut)
  keyboard_tap_with_modifiers(KEY_T, MOD_LCTRL or MOD_LALT)
  system_delay(1000)
  
  # Execute commands
  keyboard_send_line("uname -a && cat /etc/os-release")
  system_delay(500)
  keyboard_send_line("exit")

# Main payload function - exported for C calling
proc payload_main*(ctx: ptr PayloadContext): int {.exportc.} =
  # Set LED to yellow during initialization
  led_set_color(255, 255, 0, 0)
  
  # Log which controller we're running on
  if system_is_usb_controller():
    log_message("Running on USB controller (Nim)")
  else:
    log_message("Running on Wireless controller (Nim)")
  
  # Get current OS from context or detect if unknown
  var detectedOs = ctx.detected_os
  if detectedOs == OS_UNKNOWN:
    log_message("OS not detected, running detection...")
    detectedOs = os_detect(OS_DETECT_THOROUGH)
    ctx.detected_os = detectedOs
  
  # Log OS detection info
  log_message("Detected OS: " & getOsName(detectedOs))
  log_message("Detection confidence: " & $os_get_confidence() & "%")
  
  # Log hostname if available
  var hostname = newString(32)
  if os_get_hostname(hostname.cstring, 32):
    log_message("Hostname: " & hostname)
  
  # Different actions based on detected OS
  case detectedOs:
    of OS_WINDOWS:
      performWindowsActions()
    of OS_MACOS:
      performMacOsActions()
    of OS_LINUX:
      performLinuxActions()
    else:
      # Unknown or unsupported OS
      led_set_color(255, 0, 255, 0)  # Purple for unknown
      log_message("Unknown or unsupported OS")
  
  # Wait 5 seconds before completing
  system_delay(5000)
  led_off()
  return 0