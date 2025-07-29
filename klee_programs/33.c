#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <klee/klee.h>

#define MAX_TEXT_LENGTH 100
#define ALPHABET_SIZE 26

// Extended Euclidean algorithm to find modular multiplicative inverse
int mod_inverse(int a, int m) {
    a = a % m;
    for (int x = 1; x < m; x++) {
        if ((a * x) % m == 1) {
            return x;
        }
    }
    return -1; // No inverse exists
}

// Check if two numbers are coprime
int is_coprime(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a == 1;
}

// Encrypt character using affine cipher
char encrypt_char(char c, int a, int b) {
    if (!isalpha(c)) {
        return c; // Return unchanged if not a letter
    }
    
    char base = islower(c) ? 'a' : 'A';
    int x = c - base;
    int encrypted = (a * x + b) % ALPHABET_SIZE;
    return base + encrypted;
}

// Decrypt character using affine cipher
char decrypt_char(char c, int a, int b) {
    if (!isalpha(c)) {
        return c; // Return unchanged if not a letter
    }
    
    char base = islower(c) ? 'a' : 'A';
    int y = c - base;
    int a_inv = mod_inverse(a, ALPHABET_SIZE);
    
    if (a_inv == -1) {
        return c; // Cannot decrypt if no inverse exists
    }
    
    int decrypted = (a_inv * (y - b + ALPHABET_SIZE)) % ALPHABET_SIZE;
    return base + decrypted;
}

// Encrypt entire string
void encrypt_string(char* plaintext, char* ciphertext, int a, int b) {
    int len = strlen(plaintext);
    for (int i = 0; i < len; i++) {
        ciphertext[i] = encrypt_char(plaintext[i], a, b);
    }
    ciphertext[len] = '\0';
}

// Decrypt entire string
void decrypt_string(char* ciphertext, char* plaintext, int a, int b) {
    int len = strlen(ciphertext);
    for (int i = 0; i < len; i++) {
        plaintext[i] = decrypt_char(ciphertext[i], a, b);
    }
    plaintext[len] = '\0';
}

// Count letters in string
int count_letters(const char* text) {
    int count = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        if (isalpha(text[i])) {
            count++;
        }
    }
    return count;
}

// Count specific character in string
int count_char(const char* text, char c) {
    int count = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == c) {
            count++;
        }
    }
    return count;
}

// Check if string contains only valid characters
int is_valid_text(const char* text) {
    for (int i = 0; text[i] != '\0'; i++) {
        if (!isalpha(text[i]) && !isspace(text[i]) && !ispunct(text[i])) {
            return 0;
        }
    }
    return 1;
}

// Check if encryption preserves letter count
int preserves_letter_count(const char* original, const char* encrypted) {
    return count_letters(original) == count_letters(encrypted);
}

// Check if decryption recovers original text
int recovers_original(const char* original, const char* decrypted) {
    return strcmp(original, decrypted) == 0;
}

// Check if key is valid for affine cipher
int is_valid_key(int a, int b) {
    return a > 0 && a < ALPHABET_SIZE && 
           b >= 0 && b < ALPHABET_SIZE && 
           is_coprime(a, ALPHABET_SIZE);
}

// KLEE test function
int main() {
    char plaintext[MAX_TEXT_LENGTH];
    char ciphertext[MAX_TEXT_LENGTH];
    char decrypted[MAX_TEXT_LENGTH];
    int a, b;
    
    // Make inputs symbolic
    klee_make_symbolic(plaintext, sizeof(plaintext), "plaintext");
    
    // Constrain plaintext to valid characters
    for (int i = 0; i < MAX_TEXT_LENGTH - 1; i++) {
        klee_assume(plaintext[i] == '\0' || 
                   isalpha(plaintext[i]) || 
                   isspace(plaintext[i]) || 
                   ispunct(plaintext[i]));
    }
    plaintext[MAX_TEXT_LENGTH - 1] = '\0';
    
    klee_make_symbolic(&a, sizeof(int), "a");
    klee_make_symbolic(&b, sizeof(int), "b");
    klee_assume(a >= 1 && a < ALPHABET_SIZE);
    klee_assume(b >= 0 && b < ALPHABET_SIZE);
    
    // Test key validation
    klee_assert(is_valid_key(a, b) == is_coprime(a, ALPHABET_SIZE));
    
    // Test coprime function
    klee_assert(is_coprime(1, 26));
    klee_assert(is_coprime(3, 26));
    klee_assert(is_coprime(5, 26));
    klee_assert(!is_coprime(2, 26));
    klee_assert(!is_coprime(13, 26));
    
    // Test modular inverse
    klee_assert(mod_inverse(1, 26) == 1);
    klee_assert(mod_inverse(3, 26) == 9); // 3 * 9 = 27 ≡ 1 (mod 26)
    klee_assert(mod_inverse(5, 26) == 21); // 5 * 21 = 105 ≡ 1 (mod 26)
    klee_assert(mod_inverse(2, 26) == -1); // No inverse exists
    
    // Test character encryption/decryption
    char test_char = 'a';
    char encrypted_char = encrypt_char(test_char, 3, 5);
    char decrypted_char = decrypt_char(encrypted_char, 3, 5);
    klee_assert(decrypted_char == test_char);
    
    test_char = 'Z';
    encrypted_char = encrypt_char(test_char, 5, 7);
    decrypted_char = decrypt_char(encrypted_char, 5, 7);
    klee_assert(decrypted_char == test_char);
    
    // Test non-letter characters
    char space_char = ' ';
    char punct_char = '!';
    klee_assert(encrypt_char(space_char, 3, 5) == space_char);
    klee_assert(encrypt_char(punct_char, 3, 5) == punct_char);
    klee_assert(decrypt_char(space_char, 3, 5) == space_char);
    klee_assert(decrypt_char(punct_char, 3, 5) == punct_char);
    
    // Test with valid key
    if (is_valid_key(a, b)) {
        // Test string encryption/decryption
        encrypt_string(plaintext, ciphertext, a, b);
        decrypt_string(ciphertext, decrypted, a, b);
        
        // Verify properties
        klee_assert(is_valid_text(plaintext));
        klee_assert(is_valid_text(ciphertext));
        klee_assert(is_valid_text(decrypted));
        
        klee_assert(preserves_letter_count(plaintext, ciphertext));
        klee_assert(recovers_original(plaintext, decrypted));
        
        // Test that encryption changes the text (unless it's empty or all non-letters)
        if (count_letters(plaintext) > 0) {
            klee_assert(strcmp(plaintext, ciphertext) != 0);
        }
    }
    
    // Test with specific known values
    const char* test_plain = "HELLO";
    char test_cipher[10];
    char test_decrypt[10];
    
    encrypt_string(test_plain, test_cipher, 3, 5);
    decrypt_string(test_cipher, test_decrypt, 3, 5);
    
    klee_assert(strcmp(test_plain, test_decrypt) == 0);
    klee_assert(strcmp(test_plain, test_cipher) != 0);
    
    // Test with lowercase
    const char* test_lower = "hello";
    char test_lower_cipher[10];
    char test_lower_decrypt[10];
    
    encrypt_string(test_lower, test_lower_cipher, 3, 5);
    decrypt_string(test_lower_cipher, test_lower_decrypt, 3, 5);
    
    klee_assert(strcmp(test_lower, test_lower_decrypt) == 0);
    
    // Test with mixed case
    const char* test_mixed = "HeLLo";
    char test_mixed_cipher[10];
    char test_mixed_decrypt[10];
    
    encrypt_string(test_mixed, test_mixed_cipher, 3, 5);
    decrypt_string(test_mixed_cipher, test_mixed_decrypt, 3, 5);
    
    klee_assert(strcmp(test_mixed, test_mixed_decrypt) == 0);
    
    // Test with spaces and punctuation
    const char* test_complex = "Hello, World!";
    char test_complex_cipher[20];
    char test_complex_decrypt[20];
    
    encrypt_string(test_complex, test_complex_cipher, 3, 5);
    decrypt_string(test_complex_cipher, test_complex_decrypt, 3, 5);
    
    klee_assert(strcmp(test_complex, test_complex_decrypt) == 0);
    klee_assert(count_char(test_complex, ' ') == count_char(test_complex_cipher, ' '));
    klee_assert(count_char(test_complex, ',') == count_char(test_complex_cipher, ','));
    klee_assert(count_char(test_complex, '!') == count_char(test_complex_cipher, '!'));
    
    // Test edge cases
    const char* empty_text = "";
    char empty_cipher[10];
    char empty_decrypt[10];
    
    encrypt_string(empty_text, empty_cipher, 3, 5);
    decrypt_string(empty_cipher, empty_decrypt, 3, 5);
    
    klee_assert(strcmp(empty_text, empty_decrypt) == 0);
    
    const char* single_char = "A";
    char single_cipher[10];
    char single_decrypt[10];
    
    encrypt_string(single_char, single_cipher, 3, 5);
    decrypt_string(single_cipher, single_decrypt, 3, 5);
    
    klee_assert(strcmp(single_char, single_decrypt) == 0);
    
    // Test with different keys
    const char* test_key = "ABC";
    char test_key_cipher[10];
    char test_key_decrypt[10];
    
    // Key (1, 0) should be identity
    encrypt_string(test_key, test_key_cipher, 1, 0);
    klee_assert(strcmp(test_key, test_key_cipher) == 0);
    
    // Key (1, 1) should shift by 1
    encrypt_string(test_key, test_key_cipher, 1, 1);
    decrypt_string(test_key_cipher, test_key_decrypt, 1, 1);
    klee_assert(strcmp(test_key, test_key_decrypt) == 0);
    
    // Test counting functions
    klee_assert(count_letters("Hello, World!") == 10);
    klee_assert(count_char("Hello", 'l') == 2);
    klee_assert(count_char("Hello", 'x') == 0);
    
    return 0;
} 