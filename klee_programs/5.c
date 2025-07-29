#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <klee/klee.h>

#define bb 128
#define KK_MAX 64
#define NN_MAX 64
#define CEIL(a, b) (((a) / (b)) + ((a) % (b) != 0))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ROTR64(n, offset) (((n) >> (offset)) ^ ((n) << (64 - (offset))))

typedef uint64_t u128[2];
typedef uint64_t block_t[bb / sizeof(uint64_t)];

// Blake2b initialization vector
static const uint64_t blake2b_iv[8] = {
    0x6A09E667F3BCC908, 0xBB67AE8584CAA73B, 0x3C6EF372FE94F82B,
    0xA54FF53A5F1D36F1, 0x510E527FADE682D1, 0x9B05688C2B3E6C1F,
    0x1F83D9ABFB41BD6B, 0x5BE0CD19137E2179
};

// Blake2b sigma permutation
static const uint8_t blake2b_sigma[12][16] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
    {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4},
    {7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8},
    {9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13},
    {2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9},
    {12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11},
    {13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10},
    {6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5},
    {10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0},
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3}
};

// Fill u128 with value
static inline void u128_fill(u128 dest, size_t n) {
    dest[0] = (uint64_t)n;
    dest[1] = 0;
}

// Increment u128
static inline void u128_increment(u128 dest, uint64_t n) {
    dest[0] += n;
    if (dest[0] < n) {
        dest[1]++;
    }
}

// G mixing function
static void G(block_t v, uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint64_t x, uint64_t y) {
    v[a] = v[a] + v[b] + x;
    v[d] = ROTR64(v[d] ^ v[a], 32);
    v[c] = v[c] + v[d];
    v[b] = ROTR64(v[b] ^ v[c], 24);
    v[a] = v[a] + v[b] + y;
    v[d] = ROTR64(v[d] ^ v[a], 16);
    v[c] = v[c] + v[d];
    v[b] = ROTR64(v[b] ^ v[c], 63);
}

// F compression function
static void F(uint64_t h[8], block_t m, u128 t, int f) {
    uint64_t v[16];
    
    // Initialize v
    for (int i = 0; i < 8; i++) {
        v[i] = h[i];
        v[i + 8] = blake2b_iv[i];
    }
    
    v[12] ^= t[0];
    v[13] ^= t[1];
    v[14] ^= f ? 0xFFFFFFFFFFFFFFFF : 0;
    v[15] ^= f ? 0xFFFFFFFFFFFFFFFF : 0;
    
    // Mixing rounds (simplified for KLEE)
    for (int r = 0; r < 2; r++) {
        G(v, 0, 4, 8, 12, m[blake2b_sigma[r][0]], m[blake2b_sigma[r][1]]);
        G(v, 1, 5, 9, 13, m[blake2b_sigma[r][2]], m[blake2b_sigma[r][3]]);
        G(v, 2, 6, 10, 14, m[blake2b_sigma[r][4]], m[blake2b_sigma[r][5]]);
        G(v, 3, 7, 11, 15, m[blake2b_sigma[r][6]], m[blake2b_sigma[r][7]]);
    }
    
    // Update h
    for (int i = 0; i < 8; i++) {
        h[i] ^= v[i] ^ v[i + 8];
    }
}

// Simplified Blake2b hash function
uint8_t* blake2b_simple(const uint8_t* message, size_t len, uint8_t nn) {
    if (nn > NN_MAX || nn == 0) return NULL;
    
    uint8_t* hash = malloc(nn);
    if (!hash) return NULL;
    
    uint64_t h[8];
    block_t m;
    u128 t;
    
    // Initialize h with IV
    for (int i = 0; i < 8; i++) {
        h[i] = blake2b_iv[i];
    }
    
    // Set output length
    h[0] ^= 0x01010000 ^ nn;
    
    // Initialize t
    u128_fill(t, len);
    
    // Process message (simplified for KLEE)
    memset(m, 0, sizeof(m));
    size_t copy_len = MIN(len, bb);
    memcpy(m, message, copy_len);
    
    // Finalize
    F(h, m, t, 1);
    
    // Copy result
    memcpy(hash, h, nn);
    
    return hash;
}

// KLEE test function
int main() {
    // Symbolic input
    uint8_t message[32];
    size_t message_len;
    uint8_t hash_len;
    
    klee_make_symbolic(message, sizeof(message), "message");
    klee_make_symbolic(&message_len, sizeof(message_len), "message_len");
    klee_make_symbolic(&hash_len, sizeof(hash_len), "hash_len");
    
    // Constrain inputs
    klee_assume(message_len >= 1 && message_len <= 16);
    klee_assume(hash_len >= 1 && hash_len <= 32);
    
    // Compute hash
    uint8_t* hash = blake2b_simple(message, message_len, hash_len);
    
    // Add assertions for KLEE exploration
    klee_assert(hash != NULL);
    
    // Test hash properties
    for (int i = 0; i < hash_len; i++) {
        klee_assert(hash[i] >= 0 && hash[i] <= 255);
    }
    
    // Test with different message lengths
    if (message_len > 0) {
        uint8_t* hash2 = blake2b_simple(message, message_len - 1, hash_len);
        klee_assert(hash2 != NULL);
        
        // Different message lengths should produce different hashes
        int different = 0;
        for (int i = 0; i < hash_len; i++) {
            if (hash[i] != hash2[i]) {
                different = 1;
                break;
            }
        }
        klee_assert(different == 1);
        
        free(hash2);
    }
    
    // Test with different hash lengths
    if (hash_len > 1) {
        uint8_t* hash3 = blake2b_simple(message, message_len, hash_len - 1);
        klee_assert(hash3 != NULL);
        free(hash3);
    }
    
    free(hash);
    return 0;
} 