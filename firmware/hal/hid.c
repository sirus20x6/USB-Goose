/**
 * @file hal/hid.c
 * @brief Implementation of HID functions using low-level drivers
 * 
 * This file implements the HAL HID API using the lower-level
 * driver implementations from the CH569 and ESP32-C6 codebase.
 */

 #include "hal/hid.h"
 #include "payload_framework.h"
 #include "log.h"
 #include <string.h>
 
 /* Key conversion tables for mapping between HAL keys and framework keys */
 static const KeyCode key_to_framework[KEY_MAX] = {
     /* KEY_NONE */        KEY_NONE,
     
     /* Modifier keys */
     /* KEY_CTRL */        0, /* Special handling for modifiers */
     /* KEY_SHIFT */       0, /* Special handling for modifiers */
     /* KEY_ALT */         0, /* Special handling for modifiers */
     /* KEY_GUI */         0, /* Special handling for modifiers */
     
     /* Function keys */
     /* KEY_F1 */          KEY_F1,
     /* KEY_F2 */          KEY_F2,
     /* KEY_F3 */          KEY_F3,
     /* KEY_F4 */          KEY_F4,
     /* KEY_F5 */          KEY_F5,
     /* KEY_F6 */          KEY_F6,
     /* KEY_F7 */          KEY_F7,
     /* KEY_F8 */          KEY_F8,
     /* KEY_F9 */          KEY_F9,
     /* KEY_F10 */         KEY_F10,
     /* KEY_F11 */         KEY_F11,
     /* KEY_F12 */         KEY_F12,
     
     /* Navigation keys */
     /* KEY_UP */          KEY_UP,
     /* KEY_DOWN */        KEY_DOWN,
     /* KEY_LEFT */        KEY_LEFT,
     /* KEY_RIGHT */       KEY_RIGHT,
     /* KEY_PAGEUP */      KEY_PAGEUP,
     /* KEY_PAGEDOWN */    KEY_PAGEDOWN,
     /* KEY_HOME */        KEY_HOME,
     /* KEY_END */         KEY_END,
     /* KEY_INSERT */      KEY_INSERT,
     /* KEY_DELETE */      KEY_DELETE,
     /* KEY_BACKSPACE */   KEY_BACKSPACE,
     /* KEY_TAB */         KEY_TAB,
     /* KEY_RETURN */      KEY_RETURN,
     /* KEY_ESCAPE */      KEY_ESCAPE,
     
     /* Media keys */
     /* KEY_MUTE */        0, /* Map to framework equivalent if available */
     /* KEY_VOLUME_UP */   0, /* Map to framework equivalent if available */
     /* KEY_VOLUME_DOWN */ 0, /* Map to framework equivalent if available */
     /* KEY_PLAY_PAUSE */  0, /* Map to framework equivalent if available */
     
     /* Other keys */
     /* KEY_PRINTSCREEN */ 0, /* Map to framework equivalent if available */
     /* KEY_SCROLLLOCK */  0, /* Map to framework equivalent if available */
     /* KEY_PAUSE */       0, /* Map to framework equivalent if available */
     /* KEY_MENU */        0, /* Map to framework equivalent if available */
 };
 
 /* Convert modifier flags from HAL to framework format */
 static uint8_t modifiers_to_framework(bool ctrl, bool shift, bool alt, bool gui) {
     uint8_t mods = 0;
     
     if (ctrl)  mods |= MOD_LCTRL;
     if (shift) mods |= MOD_LSHIFT;
     if (alt)   mods |= MOD_LALT;
     if (gui)   mods |= MOD_LMETA;
     
     return mods;
 }
 
 bool hid_init(void) {
     LOG_INFO("Initializing HID subsystem");
     
     /* We rely on the framework's HID interface being initialized already */
     /* Validate that the keyboard/mouse interfaces are working */
     bool keyboard_ok = (KeyboardHID.press != NULL && 
                       KeyboardHID.release != NULL && 
                       KeyboardHID.type != NULL);
                       
     bool mouse_ok = (MouseHID.press != NULL && 
                    MouseHID.release != NULL && 
                    MouseHID.move != NULL);
     
     if (!keyboard_ok || !mouse_ok) {
         LOG_ERROR("HID interfaces not available");
         return false;
     }
     
     LOG_INFO("HID subsystem initialized successfully");
     return true;
 }
 
 bool type(const char *text, uint16_t delay_ms) {
     if (text == NULL) {
         LOG_ERROR("NULL text passed to type()");
         return false;
     }
     
     LOG_DEBUG("Typing text with %ums delay", delay_ms);
     
     /* If delay is 0, use the framework's default delay */
     if (delay_ms == 0) {
         KeyboardHID.sendString(text);
     } else {
         KeyboardHID.type(text, delay_ms);
     }
     
     return true;
 }
 
 bool key_press(key_code_t key) {
     if (key <= KEY_NONE || key >= KEY_MAX) {
         LOG_ERROR("Invalid key code: %d", key);
         return false;
     }
     
     /* Handle special cases */
     if (key == KEY_CTRL) {
         KeyboardHID.pressModifier(MOD_LCTRL);
         KeyboardHID.delay(5);
         KeyboardHID.releaseModifier(MOD_LCTRL);
         return true;
     } else if (key == KEY_SHIFT) {
         KeyboardHID.pressModifier(MOD_LSHIFT);
         KeyboardHID.delay(5);
         KeyboardHID.releaseModifier(MOD_LSHIFT);
         return true;
     } else if (key == KEY_ALT) {
         KeyboardHID.pressModifier(MOD_LALT);
         KeyboardHID.delay(5);
         KeyboardHID.releaseModifier(MOD_LALT);
         return true;
     } else if (key == KEY_GUI) {
         KeyboardHID.pressModifier(MOD_LMETA);
         KeyboardHID.delay(5);
         KeyboardHID.releaseModifier(MOD_LMETA);
         return true;
     }
     
     KeyCode framework_key = key_to_framework[key];
     if (framework_key == 0) {
         LOG_ERROR("Unmapped key code: %d", key);
         return false;
     }
     
     KeyboardHID.tap(framework_key);
     return true;
 }
 
 bool key_down(key_code_t key) {
     if (key <= KEY_NONE || key >= KEY_MAX) {
         LOG_ERROR("Invalid key code: %d", key);
         return false;
     }
     
     /* Handle special cases */
     if (key == KEY_CTRL) {
         KeyboardHID.pressModifier(MOD_LCTRL);
         return true;
     } else if (key == KEY_SHIFT) {
         KeyboardHID.pressModifier(MOD_LSHIFT);
         return true;
     } else if (key == KEY_ALT) {
         KeyboardHID.pressModifier(MOD_LALT);
         return true;
     } else if (key == KEY_GUI) {
         KeyboardHID.pressModifier(MOD_LMETA);
         return true;
     }
     
     KeyCode framework_key = key_to_framework[key];
     if (framework_key == 0) {
         LOG_ERROR("Unmapped key code: %d", key);
         return false;
     }
     
     KeyboardHID.press(framework_key);
     return true;
 }
 
 bool key_up(key_code_t key) {
     if (key <= KEY_NONE || key >= KEY_MAX) {
         LOG_ERROR("Invalid key code: %d", key);
         return false;
     }
     
     /* Handle special cases */
     if (key == KEY_CTRL) {
         KeyboardHID.releaseModifier(MOD_LCTRL);
         return true;
     } else if (key == KEY_SHIFT) {
         KeyboardHID.releaseModifier(MOD_LSHIFT);
         return true;
     } else if (key == KEY_ALT) {
         KeyboardHID.releaseModifier(MOD_LALT);
         return true;
     } else if (key == KEY_GUI) {
         KeyboardHID.releaseModifier(MOD_LMETA);
         return true;
     }
     
     KeyCode framework_key = key_to_framework[key];
     if (framework_key == 0) {
         LOG_ERROR("Unmapped key code: %d", key);
         return false;
     }
     
     KeyboardHID.release(framework_key);
     return true;
 }
 
 bool key_combo(const key_code_t *keys, uint8_t num_keys) {
     if (keys == NULL || num_keys == 0) {
         LOG_ERROR("Invalid keys array");
         return false;
     }
     
     /* Press all keys in sequence */
     for (uint8_t i = 0; i < num_keys; i++) {
         if (!key_down(keys[i])) {
             /* Release any keys we've already pressed if there's an error */
             for (uint8_t j = 0; j < i; j++) {
                 key_up(keys[j]);
             }
             return false;
         }
     }
     
     /* Small delay to ensure the combination is registered */
     KeyboardHID.delay(10);
     
     /* Release all keys in reverse order */
     for (int i = num_keys - 1; i >= 0; i--) {
         key_up(keys[i]);
     }
     
     return true;
 }
 
 bool key_with_modifiers(key_code_t key, bool ctrl, bool shift, bool alt, bool gui) {
     if (key <= KEY_NONE || key >= KEY_MAX) {
         LOG_ERROR("Invalid key code: %d", key);
         return false;
     }
     
     /* Skip modifier keys when used as the main key in a combo */
     if (key == KEY_CTRL || key == KEY_SHIFT || key == KEY_ALT || key == KEY_GUI) {
         LOG_ERROR("Cannot use modifier as primary key in combo");
         return false;
     }
     
     KeyCode framework_key = key_to_framework[key];
     if (framework_key == 0) {
         LOG_ERROR("Unmapped key code: %d", key);
         return false;
     }
     
     uint8_t modifiers = modifiers_to_framework(ctrl, shift, alt, gui);
     KeyboardHID.tapWithModifiers(framework_key, modifiers);
     return true;
 }
 
 bool release_all(void) {
     /* Release all modifiers */
     KeyboardHID.releaseModifier(MOD_LCTRL | MOD_LSHIFT | MOD_LALT | MOD_LMETA | 
                               MOD_RCTRL | MOD_RSHIFT | MOD_RALT | MOD_RMETA);
     
     /* Release all keys by sending an empty report */
     for (int i = 2; i < 8; i++) {
         KeyboardHID.release(0);
     }
     
     return true;
 }
 
 bool mouse_move(int16_t dx, int16_t dy) {
     MouseHID.move(dx, dy);
     return true;
 }
 
 bool mouse_move_to(uint16_t x, uint16_t y) {
     /* Some hardware doesn't support absolute positioning */
     if (MouseHID.moveTo != NULL) {
         MouseHID.moveTo(x, y);
         return true;
     } else {
         LOG_WARN("Absolute mouse positioning not supported by hardware");
         return false;
     }
 }
 
 bool mouse_down(uint8_t button) {
     if (button < 1 || button > 3) {
         LOG_ERROR("Invalid mouse button: %d", button);
         return false;
     }
     
     uint8_t btn_mask = (1 << (button - 1));
     MouseHID.press(btn_mask);
     return true;
 }
 
 bool mouse_up(uint8_t button) {
     if (button < 1 || button > 3) {
         LOG_ERROR("Invalid mouse button: %d", button);
         return false;
     }
     
     uint8_t btn_mask = (1 << (button - 1));
     MouseHID.release(btn_mask);
     return true;
 }
 
 bool mouse_click(uint8_t button, uint8_t count) {
     if (button < 1 || button > 3) {
         LOG_ERROR("Invalid mouse button: %d", button);
         return false;
     }
     
     if (count == 0) {
         return true;  /* Nothing to do */
     }
     
     uint8_t btn_mask = (1 << (button - 1));
     
     if (count == 1) {
         MouseHID.click(btn_mask);
     } else if (count == 2) {
         MouseHID.doubleClick(btn_mask);
     } else {
         /* For more than 2 clicks, we need to do them manually */
         for (uint8_t i = 0; i < count; i++) {
             MouseHID.click(btn_mask);
             KeyboardHID.delay(10);  /* Small delay between clicks */
         }
     }
     
     return true;
 }
 
 bool mouse_scroll(int8_t amount) {
     MouseHID.scroll(amount);
     return true;
 }