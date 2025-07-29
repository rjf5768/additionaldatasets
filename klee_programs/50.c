#include <klee/klee.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

bool isbad_alphabet(const char* alphabet) {
    uint64_t len = strlen(alphabet);
    
    if (len < 2) {
        return true;
    }
    
    for (int i = 0; i < len ; i++) {
        if (strchr(alphabet + i + 1, alphabet[i]))
            return true;
    }
    return false;
}

uint64_t converted_len(uint64_t nb, short base) {
    if (nb > base - 1) {
        return (converted_len(nb/base, base) + 1);
    }
    return 1;
}

void convertion(uint64_t nb, const char* alphabet, short base, char* converted) {
    *(converted) = *(alphabet + nb%base);
    if (nb > base - 1) {
        convertion(nb/base, alphabet, base, --converted);
    }
}

char* decimal_to_anybase(uint64_t nb, const char* alphabet) {
    char* converted;

    if (isbad_alphabet(alphabet)) {
        return NULL;
    }
    
    uint64_t base = strlen(alphabet);
    uint64_t final_len = converted_len(nb, base);
    converted = malloc(sizeof(char) * (final_len + 1));
    converted[final_len] = 0;
    convertion(nb, alphabet, base, converted + final_len - 1);
    return converted;
}

int main()
{
    uint64_t number;
    char alphabet[10];
    
    // Make inputs symbolic
    klee_make_symbolic(&number, sizeof(number), "number");
    klee_make_symbolic(alphabet, sizeof(alphabet), "alphabet");
    
    // Constrain inputs to reasonable values
    klee_assume(number >= 0 && number <= 1000);
    
    // Ensure alphabet is null-terminated
    alphabet[9] = '\0';
    
    // Constrain alphabet to valid characters
    for (int i = 0; i < 9; i++) {
        klee_assume(alphabet[i] >= '0' && alphabet[i] <= '9');
    }
    
    // Ensure alphabet has at least 2 characters
    klee_assume(strlen(alphabet) >= 2);
    
    // Convert number
    char* result = decimal_to_anybase(number, alphabet);
    
    // Verify result is not NULL for valid inputs
    klee_assert(result != NULL);
    
    // Verify result is null-terminated
    klee_assert(result[strlen(result)] == '\0');
    
    // Verify result length is reasonable
    klee_assert(strlen(result) > 0);
    
    // Test with a known base (binary)
    char* binary_result = decimal_to_anybase(number, "01");
    klee_assert(binary_result != NULL);
    
    // Verify binary conversion properties
    if (number == 0) {
        klee_assert(strcmp(binary_result, "0") == 0);
    } else if (number == 1) {
        klee_assert(strcmp(binary_result, "1") == 0);
    }
    
    free(result);
    free(binary_result);
    
    return 0;
} 