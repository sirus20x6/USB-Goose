/**
 * @file hal/hid.h
 * @brief HID emulation functions
 * 
 * This file contains the API for HID (keyboard/mouse) emulation,
 * allowing for scripted input execution across different hardware.
 */

 #ifndef HAL_HID_H
 #define HAL_HID_H
 
 #include <stdint.h>
 #include <stdbool.h>
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /**
  * @brief Special key codes
  */
 typedef enum {
     KEY_NONE = 0,
     
     /* Modifier keys */
     KEY_CTRL,
     KEY_SHIFT,
     KEY_ALT,
     KEY_GUI, /* Windows key or Command key */
     
     /* Function keys */
     KEY_F1,
     KEY_F2,
     KEY_F3,
     KEY_F4,
     KEY_F5,
     KEY_F6,
     KEY_F7,
     KEY_F8,
     KEY_F9,
     KEY_F10,
     KEY_F11,
     KEY_F12,
     
     /* Navigation keys */
     KEY_UP,
     KEY_DOWN,
     KEY_LEFT,
     KEY_RIGHT,
     KEY_PAGEUP,
     KEY_PAGEDOWN,
     KEY_HOME,
     KEY_END,
     KEY_INSERT,
     KEY_DELETE,
     KEY_BACKSPACE,
     KEY_TAB,
     KEY_RETURN,
     KEY_ESCAPE,
     
     /* Media keys */
     KEY_MUTE,
     KEY_VOLUME_UP,
     KEY_VOLUME_DOWN,
     KEY_PLAY_PAUSE,
     
     /* Other keys as needed */
     KEY_PRINTSCREEN,
     KEY_SCROLLLOCK,
     KEY_PAUSE,
     KEY_MENU,
     
     KEY_MAX
 } key_code_t;
 
 /**
  * @brief Initialize HID subsystem
  * 
  * @return true if initialization successful, false otherwise
  */
 bool hid_init(void);
 
 /**
  * @brief Type a string of characters
  * 
  * This function types the given string with configurable delay between keystrokes.
  * 
  * @param text Text to type
  * @param delay_ms Delay between keystrokes in milliseconds (0 for fastest possible)
  * @return true if typing completed successfully, false otherwise
  */
 bool type(const char *text, uint16_t delay_ms);
 
 /**
  * @brief Press and release a single key
  * 
  * @param key Key to press
  * @return true if successful, false otherwise
  */
 bool key_press(key_code_t key);
 
 /**
  * @brief Press and hold a key
  * 
  * @param key Key to press and hold
  * @return true if successful, false otherwise
  */
 bool key_down(key_code_t key);
 
 /**
  * @brief Release a previously held key
  * 
  * @param key Key to release
  * @return true if successful, false otherwise
  */
 bool key_up(key_code_t key);
 
 /**
  * @brief Press a combination of keys simultaneously
  * 
  * @param keys Array of keys to press
  * @param num_keys Number of keys in the array
  * @return true if successful, false otherwise
  */
 bool key_combo(const key_code_t *keys, uint8_t num_keys);
 
 /**
  * @brief Shorthand for pressing a key with modifiers
  * 
  * @param key Main key to press
  * @param ctrl true to include Ctrl
  * @param shift true to include Shift
  * @param alt true to include Alt
  * @param gui true to include GUI (Win/Cmd)
  * @return true if successful, false otherwise
  */
 bool key_with_modifiers(key_code_t key, bool ctrl, bool shift, bool alt, bool gui);
 
 /**
  * @brief Release all currently pressed keys
  * 
  * @return true if successful, false otherwise
  */
 bool release_all(void);
 
 /**
  * @brief Move the mouse cursor
  * 
  * @param dx Horizontal movement (positive = right, negative = left)
  * @param dy Vertical movement (positive = down, negative = up)
  * @return true if successful, false otherwise
  */
 bool mouse_move(int16_t dx, int16_t dy);
 
 /**
  * @brief Move the mouse cursor to absolute coordinates (when supported)
  * 
  * @param x Horizontal position (0-32767)
  * @param y Vertical position (0-32767)
  * @return true if successful, false otherwise
  */
 bool mouse_move_to(uint16_t x, uint16_t y);
 
 /**
  * @brief Press a mouse button
  * 
  * @param button Button number (1=left, 2=right, 3=middle)
  * @return true if successful, false otherwise
  */
 bool mouse_down(uint8_t button);
 
 /**
  * @brief Release a mouse button
  * 
  * @param button Button number (1=left, 2=right, 3=middle)
  * @return true if successful, false otherwise
  */
 bool mouse_up(uint8_t button);
 
 /**
  * @brief Click a mouse button
  * 
  * @param button Button number (1=left, 2=right, 3=middle)
  * @param count Number of clicks (1 for single-click, 2 for double-click)
  * @return true if successful, false otherwise
  */
 bool mouse_click(uint8_t button, uint8_t count);
 
 /**
  * @brief Scroll the mouse wheel
  * 
  * @param amount Scroll amount (positive = down/back, negative = up/forward)
  * @return true if successful, false otherwise
  */
 bool mouse_scroll(int8_t amount);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* HAL_HID_H */