#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#define ALPHABET_SIZE 95
#define MAX_MESSAGE_LENGTH 100
#define Z95_CONVERSION_CONSTANT 32

typedef struct {
    int a;
    int b;
} AffineKey;

// Calculate greatest common divisor
int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// Check if two numbers are coprime
int is_coprime(int a, int b) {
    return gcd(a, b) == 1;
}

// Calculate modular multiplicative inverse
int modular_multiplicative_inverse(int a, int m) {
    int x[2] = {1, 0};
    int y[2] = {0, 1};
    int r[2] = {m, a};
    
    while (r[1] != 0) {
        int q = r[0] / r[1];
        int temp = r[1];
        r[1] = r[0] % r[1];
        r[0] = temp;
        
        temp = x[1];
        x[1] = x[0] - q * x[1];
        x[0] = temp;
        
        temp = y[1];
        y[1] = y[0] - q * y[1];
        y[0] = temp;
    }
    
    if (r[0] != 1) {
        return 0; // No inverse exists
    }
    
    int result = x[0];
    if (result < 0) {
        result += m;
    }
    return result;
}

// Check if key is valid (a must be coprime with alphabet size)
int is_valid_key(AffineKey key) {
    return is_coprime(key.a, ALPHABET_SIZE) && key.a != 0;
}

// Generate inverse key
AffineKey inverse_key(AffineKey key) {
    AffineKey inverse;
    
    inverse.a = modular_multiplicative_inverse(key.a, ALPHABET_SIZE);
    if (inverse.a < 0) {
        inverse.a += ALPHABET_SIZE;
    }
    
    inverse.b = (-key.b % ALPHABET_SIZE + ALPHABET_SIZE) % ALPHABET_SIZE;
    
    return inverse;
}

// Convert character to Z95 (0-94)
int char_to_z95(char c) {
    if (c < 32 || c > 126) {
        return -1; // Invalid character
    }
    return c - Z95_CONVERSION_CONSTANT;
}

// Convert Z95 (0-94) to character
char z95_to_char(int z) {
    if (z < 0 || z >= ALPHABET_SIZE) {
        return '\0'; // Invalid value
    }
    return z + Z95_CONVERSION_CONSTANT;
}

// Encrypt a single character
char encrypt_char(char c, AffineKey key) {
    int z = char_to_z95(c);
    if (z == -1) {
        return c; // Return unchanged if invalid
    }
    
    int encrypted_z = (key.a * z + key.b) % ALPHABET_SIZE;
    if (encrypted_z < 0) {
        encrypted_z += ALPHABET_SIZE;
    }
    
    return z95_to_char(encrypted_z);
}

// Decrypt a single character
char decrypt_char(char c, AffineKey key) {
    int z = char_to_z95(c);
    if (z == -1) {
        return c; // Return unchanged if invalid
    }
    
    int decrypted_z = (key.a * (z - key.b)) % ALPHABET_SIZE;
    if (decrypted_z < 0) {
        decrypted_z += ALPHABET_SIZE;
    }
    
    return z95_to_char(decrypted_z);
}

// Encrypt a string
void encrypt_string(char* message, AffineKey key) {
    int len = strlen(message);
    for (int i = 0; i < len; i++) {
        message[i] = encrypt_char(message[i], key);
    }
}

// Decrypt a string
void decrypt_string(char* message, AffineKey key) {
    int len = strlen(message);
    for (int i = 0; i < len; i++) {
        message[i] = decrypt_char(message[i], key);
    }
}

// Check if string contains only printable ASCII
int is_printable_ascii(const char* str) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (str[i] < 32 || str[i] > 126) {
            return 0;
        }
    }
    return 1;
}

// Count printable characters in string
int count_printable_chars(const char* str) {
    int count = 0;
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (str[i] >= 32 && str[i] <= 126) {
            count++;
        }
    }
    return count;
}

// Check if encryption/decryption is reversible
int is_reversible(const char* original, AffineKey key) {
    char* encrypted = strdup(original);
    char* decrypted = strdup(original);
    
    encrypt_string(encrypted, key);
    decrypt_string(encrypted, inverse_key(key));
    
    int result = strcmp(original, encrypted) == 0;
    
    free(encrypted);
    free(decrypted);
    
    return result;
}

// Check if encryption changes the string
int encryption_changes_string(const char* original, AffineKey key) {
    char* encrypted = strdup(original);
    encrypt_string(encrypted, key);
    
    int result = strcmp(original, encrypted) != 0;
    
    free(encrypted);
    return result;
}

// KLEE test function
int main() {
    char message[MAX_MESSAGE_LENGTH];
    AffineKey key;
    
    // Make inputs symbolic
    klee_make_symbolic(message, sizeof(message), "message");
    klee_make_symbolic(&key.a, sizeof(int), "key_a");
    klee_make_symbolic(&key.b, sizeof(int), "key_b");
    
    // Constrain inputs
    klee_assume(key.a >= 1 && key.a <= 1000);
    klee_assume(key.b >= 0 && key.b <= 1000);
    klee_assume(strlen(message) < MAX_MESSAGE_LENGTH);
    
    // Test GCD and coprime functions
    klee_assert(gcd(12, 18) == 6);
    klee_assert(gcd(7, 13) == 1);
    klee_assert(is_coprime(7, 13));
    klee_assert(!is_coprime(12, 18));
    
    // Test modular multiplicative inverse
    klee_assert(modular_multiplicative_inverse(3, 11) == 4);
    klee_assert(modular_multiplicative_inverse(7, 13) == 2);
    
    // Test character conversion
    klee_assert(char_to_z95('A') == 33);
    klee_assert(char_to_z95(' ') == 0);
    klee_assert(z95_to_char(33) == 'A');
    klee_assert(z95_to_char(0) == ' ');
    
    // Test key validation
    AffineKey valid_key = {7, 3};
    AffineKey invalid_key = {95, 3}; // 95 is not coprime with 95
    
    klee_assert(is_valid_key(valid_key));
    klee_assert(!is_valid_key(invalid_key));
    
    // Test inverse key
    AffineKey inverse = inverse_key(valid_key);
    klee_assert(is_valid_key(inverse));
    
    // Test that key and its inverse are inverses
    AffineKey double_inverse = inverse_key(inverse);
    klee_assert(double_inverse.a == valid_key.a);
    klee_assert(double_inverse.b == valid_key.b);
    
    // Test encryption/decryption with known values
    char test_message[] = "HELLO WORLD";
    AffineKey test_key = {7, 3};
    
    char encrypted_copy[MAX_MESSAGE_LENGTH];
    strcpy(encrypted_copy, test_message);
    encrypt_string(encrypted_copy, test_key);
    
    // Verify encryption changed the string
    klee_assert(strcmp(test_message, encrypted_copy) != 0);
    
    // Verify decryption restores original
    decrypt_string(encrypted_copy, inverse_key(test_key));
    klee_assert(strcmp(test_message, encrypted_copy) == 0);
    
    // Test with symbolic key if valid
    if (is_valid_key(key)) {
        // Test that encryption/decryption is reversible
        klee_assert(is_reversible(message, key));
        
        // Test that encryption changes the string (unless it's empty)
        if (strlen(message) > 0 && count_printable_chars(message) > 0) {
            klee_assert(encryption_changes_string(message, key));
        }
        
        // Test individual character encryption/decryption
        for (int i = 0; i < strlen(message); i++) {
            char original_char = message[i];
            char encrypted_char = encrypt_char(original_char, key);
            char decrypted_char = decrypt_char(encrypted_char, inverse_key(key));
            
            if (original_char >= 32 && original_char <= 126) {
                klee_assert(original_char == decrypted_char);
            }
        }
    }
    
    // Test edge cases
    // Empty string
    char empty_message[] = "";
    AffineKey edge_key = {7, 3};
    encrypt_string(empty_message, edge_key);
    klee_assert(strlen(empty_message) == 0);
    
    // Single character
    char single_char[] = "A";
    encrypt_string(single_char, edge_key);
    klee_assert(strlen(single_char) == 1);
    
    // String with non-printable characters
    char mixed_message[] = "Hello\x01World";
    encrypt_string(mixed_message, edge_key);
    // Non-printable characters should remain unchanged
    klee_assert(mixed_message[5] == '\x01');
    
    // Test with different keys
    AffineKey key1 = {3, 5};
    AffineKey key2 = {11, 7};
    
    if (is_valid_key(key1) && is_valid_key(key2)) {
        char test_str[] = "TEST";
        char encrypted1[MAX_MESSAGE_LENGTH];
        char encrypted2[MAX_MESSAGE_LENGTH];
        
        strcpy(encrypted1, test_str);
        strcpy(encrypted2, test_str);
        
        encrypt_string(encrypted1, key1);
        encrypt_string(encrypted2, key2);
        
        // Different keys should produce different encryptions
        klee_assert(strcmp(encrypted1, encrypted2) != 0);
    }
    
    // Test mathematical properties
    // 1. Identity: encrypting with key (1, 0) should not change the message
    AffineKey identity_key = {1, 0};
    char identity_test[] = "IDENTITY";
    encrypt_string(identity_test, identity_key);
    klee_assert(strcmp(identity_test, "IDENTITY") == 0);
    
    // 2. Zero shift: encrypting with key (a, 0) should only scale
    AffineKey zero_shift_key = {3, 0};
    char zero_shift_test[] = "A";
    char original_char = zero_shift_test[0];
    encrypt_string(zero_shift_test, zero_shift_key);
    klee_assert(zero_shift_test[0] != original_char);
    
    return 0;
} 