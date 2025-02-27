/**
 * security.c - Implementation of security functions
 */

#include "security.h"
#include <string.h>

// Define AES block size
#define AES_BLOCK_SIZE 16

// Internal state
static bool security_initialized = false;
static uint8_t current_key[32]; // 256-bit key
static SecurityHardware hw = {0};

// Initialize security subsystem
bool security_init(const SecurityHardware* hardware) {
    if (hardware == NULL) {
        return false;
    }
    
    // Copy hardware functions
    memcpy(&hw, hardware, sizeof(SecurityHardware));
    
    // Initialize hardware security if available
    if (hw.initialize != NULL) {
        if (!hw.initialize()) {
            return false;
        }
    }
    
    // Clear current key
    memset(current_key, 0, sizeof(current_key));
    
    security_initialized = true;
    return true;
}

// Set the active encryption key
bool security_set_key(const uint8_t* key, size_t key_size) {
    if (!security_initialized || key == NULL || 
        (key_size != 16 && key_size != 24 && key_size != 32)) {
        return false;
    }
    
    // Save the key
    memset(current_key, 0, sizeof(current_key));
    memcpy(current_key, key, key_size);
    
    // If hardware key storage is available, use it
    if (hw.set_key != NULL) {
        return hw.set_key(key, key_size);
    }
    
    return true;
}

// Generate a secure random key
bool security_generate_key(uint8_t* key, size_t key_size) {
    if (!security_initialized || key == NULL || 
        (key_size != 16 && key_size != 24 && key_size != 32)) {
        return false;
    }
    
    // Use hardware random if available
    if (hw.generate_random != NULL) {
        if (!hw.generate_random(key, key_size)) {
            return false;
        }
    } else {
        // Simple software fallback - not secure!
        // This should be replaced with a better PRNG if hardware is not available
        for (size_t i = 0; i < key_size; i++) {
            key[i] = (uint8_t)(i * 7 + 13);
        }
    }
    
    return true;
}

// Encrypt data using AES
bool security_encrypt_aes(const uint8_t* data, size_t data_len, 
                         uint8_t* output, size_t output_size,
                         const uint8_t* iv, size_t iv_size) {
    if (!security_initialized || data == NULL || output == NULL || 
        iv == NULL || iv_size != 16 || 
        output_size < data_len + (AES_BLOCK_SIZE - (data_len % AES_BLOCK_SIZE))) {
        return false;
    }
    
    // Use hardware acceleration if available
    if (hw.encrypt_aes != NULL) {
        return hw.encrypt_aes(data, data_len, output, output_size, iv, iv_size);
    } else {
        // Software implementation would go here
        // This is a placeholder - implement a software AES if needed
        memcpy(output, data, data_len);
        return true;
    }
}

// Decrypt data using AES
bool security_decrypt_aes(const uint8_t* data, size_t data_len, 
                         uint8_t* output, size_t output_size,
                         const uint8_t* iv, size_t iv_size) {
    if (!security_initialized || data == NULL || output == NULL || 
        iv == NULL || iv_size != 16 || output_size < data_len) {
        return false;
    }
    
    // Use hardware acceleration if available
    if (hw.decrypt_aes != NULL) {
        return hw.decrypt_aes(data, data_len, output, output_size, iv, iv_size);
    } else {
        // Software implementation would go here
        // This is a placeholder - implement a software AES if needed
        memcpy(output, data, data_len);
        return true;
    }
}

// Calculate SHA-256 hash
bool security_hash_sha256(const uint8_t* data, size_t data_len, 
                         uint8_t* hash, size_t hash_size) {
    if (!security_initialized || data == NULL || hash == NULL || hash_size < 32) {
        return false;
    }
    
    // Use hardware acceleration if available
    if (hw.hash_sha256 != NULL) {
        return hw.hash_sha256(data, data_len, hash, hash_size);
    } else {
        // Software implementation would go here
        // This is a placeholder - implement a software SHA-256 if needed
        memset(hash, 0, hash_size);
        return true;
    }
}

// Sign data using HMAC-SHA256
bool security_sign_hmac(const uint8_t* data, size_t data_len,
                       uint8_t* signature, size_t signature_size) {
    if (!security_initialized || data == NULL || signature == NULL || signature_size < 32) {
        return false;
    }
    
    // Use hardware acceleration if available
    if (hw.sign_hmac != NULL) {
        return hw.sign_hmac(data, data_len, signature, signature_size);
    } else {
        // Software implementation would go here
        // This is a placeholder - implement a software HMAC if needed
        memset(signature, 0, signature_size);
        return true;
    }
}

// Verify data using HMAC-SHA256
bool security_verify_hmac(const uint8_t* data, size_t data_len,
                         const uint8_t* signature, size_t signature_size) {
    if (!security_initialized || data == NULL || signature == NULL || signature_size < 32) {
        return false;
    }
    
    // Calculate expected signature
    uint8_t expected[32];
    if (!security_sign_hmac(data, data_len, expected, sizeof(expected))) {
        return false;
    }
    
    // Compare signatures
    return (memcmp(expected, signature, 32) == 0);
}

// Store a secure value in protected storage
bool security_store_secure(const char* key, const uint8_t* data, size_t data_len) {
    if (!security_initialized || key == NULL || data == NULL || data_len == 0) {
        return false;
    }
    
    // Use hardware secure storage if available
    if (hw.store_secure != NULL) {
        return hw.store_secure(key, data, data_len);
    }
    
    // No secure storage available
    return false;
}

// Retrieve a secure value from protected storage
bool security_retrieve_secure(const char* key, uint8_t* data, size_t* data_len) {
    if (!security_initialized || key == NULL || data == NULL || data_len == NULL) {
        return false;
    }
    
    // Use hardware secure storage if available
    if (hw.retrieve_secure != NULL) {
        return hw.retrieve_secure(key, data, data_len);
    }
    
    // No secure storage available
    return false;
}