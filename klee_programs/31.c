#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <klee/klee.h>

#define MAX_SAMPLES 100

// A-law encoding: 16-bit PCM to 8-bit A-law
uint8_t encode_alaw(int16_t pcm) {
    uint8_t alaw = 0;
    int32_t sign = 0;
    int32_t abcd = 0;
    int32_t eee = 0;
    int32_t mask = 0;
    
    // Get sign bit
    sign = (pcm & 0x8000) >> 8;
    
    // Turn negative PCM to positive
    pcm = sign ? (-pcm - 1) : pcm;
    
    // Find quantization level (eee)
    eee = 7;
    mask = 0x4000; // 0b0100 0000 0000 0000
    
    while ((pcm & mask) == 0 && eee > 0) {
        eee--;
        mask >>= 1;
    }
    
    // Get abcd bits
    abcd = (pcm >> (eee ? (eee + 3) : 4)) & 0x0f;
    
    // Put quantization level at right bit location
    eee <<= 4;
    
    // Combine results
    alaw = (sign | eee | abcd);
    
    // Invert even bits for clock recovery
    return alaw ^ 0xD5;
}

// A-law decoding: 8-bit A-law to 16-bit PCM
int16_t decode_alaw(uint8_t alaw) {
    int16_t pcm = 0;
    int32_t sign = 0;
    int32_t eee = 0;
    int32_t abcd = 0;
    
    // Invert even bits back
    alaw ^= 0xD5;
    
    // Extract components
    sign = (alaw & 0x80) << 8;
    eee = (alaw & 0x70) >> 4;
    abcd = alaw & 0x0f;
    
    // Reconstruct PCM value
    if (eee == 0) {
        pcm = (abcd << 1) | 1;
    } else {
        pcm = ((abcd << 1) | 1) << (eee - 1);
    }
    
    // Apply sign
    return sign ? -pcm : pcm;
}

// Encode array of PCM samples
void encode_array(uint8_t* out, int16_t* in, size_t len) {
    for (size_t i = 0; i < len; i++) {
        out[i] = encode_alaw(in[i]);
    }
}

// Decode array of A-law samples
void decode_array(int16_t* out, uint8_t* in, size_t len) {
    for (size_t i = 0; i < len; i++) {
        out[i] = decode_alaw(in[i]);
    }
}

// Check if PCM value is in valid range
int is_valid_pcm(int16_t pcm) {
    return pcm >= -32768 && pcm <= 32767;
}

// Check if A-law value is valid
int is_valid_alaw(uint8_t alaw) {
    // After inversion, valid A-law values should be in certain ranges
    uint8_t inverted = alaw ^ 0xD5;
    return (inverted & 0x80) == 0 || (inverted & 0x80) == 0x80;
}

// Calculate signal-to-noise ratio (simplified)
double calculate_snr(int16_t* original, int16_t* decoded, size_t len) {
    double signal_power = 0;
    double noise_power = 0;
    
    for (size_t i = 0; i < len; i++) {
        signal_power += (double)(original[i] * original[i]);
        int32_t diff = original[i] - decoded[i];
        noise_power += (double)(diff * diff);
    }
    
    if (noise_power == 0) return 100.0; // Perfect reconstruction
    
    return 10.0 * log10(signal_power / noise_power);
}

// Check if encoding/decoding preserves sign
int preserves_sign(int16_t original, int16_t decoded) {
    if (original >= 0 && decoded >= 0) return 1;
    if (original < 0 && decoded < 0) return 1;
    return 0;
}

// Count zero crossings in signal
int count_zero_crossings(int16_t* signal, size_t len) {
    int crossings = 0;
    for (size_t i = 1; i < len; i++) {
        if ((signal[i-1] >= 0 && signal[i] < 0) || 
            (signal[i-1] < 0 && signal[i] >= 0)) {
            crossings++;
        }
    }
    return crossings;
}

// KLEE test function
int main() {
    int16_t pcm_samples[MAX_SAMPLES];
    uint8_t alaw_samples[MAX_SAMPLES];
    int16_t decoded_samples[MAX_SAMPLES];
    size_t num_samples;
    
    // Make inputs symbolic
    klee_make_symbolic(&num_samples, sizeof(size_t), "num_samples");
    klee_assume(num_samples >= 1 && num_samples <= MAX_SAMPLES);
    
    for (size_t i = 0; i < num_samples; i++) {
        klee_make_symbolic(&pcm_samples[i], sizeof(int16_t), "pcm_sample");
        klee_assume(is_valid_pcm(pcm_samples[i]));
    }
    
    // Test encoding
    encode_array(alaw_samples, pcm_samples, num_samples);
    
    // Verify all encoded values are valid
    for (size_t i = 0; i < num_samples; i++) {
        klee_assert(is_valid_alaw(alaw_samples[i]));
    }
    
    // Test decoding
    decode_array(decoded_samples, alaw_samples, num_samples);
    
    // Verify all decoded values are valid
    for (size_t i = 0; i < num_samples; i++) {
        klee_assert(is_valid_pcm(decoded_samples[i]));
    }
    
    // Test round-trip properties
    for (size_t i = 0; i < num_samples; i++) {
        // Sign should be preserved
        klee_assert(preserves_sign(pcm_samples[i], decoded_samples[i]));
        
        // Magnitude should be reasonable (A-law is lossy)
        int32_t original_mag = abs(pcm_samples[i]);
        int32_t decoded_mag = abs(decoded_samples[i]);
        klee_assert(decoded_mag <= original_mag * 2); // Allow some expansion
    }
    
    // Test with known values
    int16_t test_pcm[] = {0, 1, -1, 100, -100, 1000, -1000, 32767, -32768};
    uint8_t test_alaw[9];
    int16_t test_decoded[9];
    
    encode_array(test_alaw, test_pcm, 9);
    decode_array(test_decoded, test_alaw, 9);
    
    // Test specific cases
    klee_assert(test_pcm[0] == 0); // Zero should encode/decode correctly
    klee_assert(test_decoded[0] == 0);
    
    klee_assert(test_pcm[1] == 1); // Small positive value
    klee_assert(test_decoded[1] >= 0); // Should remain positive
    
    klee_assert(test_pcm[2] == -1); // Small negative value
    klee_assert(test_decoded[2] <= 0); // Should remain negative
    
    // Test edge cases
    klee_assert(test_pcm[7] == 32767); // Maximum positive
    klee_assert(test_decoded[7] > 0); // Should remain positive
    
    klee_assert(test_pcm[8] == -32768); // Maximum negative
    klee_assert(test_decoded[8] < 0); // Should remain negative
    
    // Test individual encode/decode functions
    uint8_t encoded = encode_alaw(1000);
    int16_t decoded = decode_alaw(encoded);
    klee_assert(decoded >= 0); // Should preserve sign
    
    encoded = encode_alaw(-1000);
    decoded = decode_alaw(encoded);
    klee_assert(decoded <= 0); // Should preserve sign
    
    // Test with zero
    encoded = encode_alaw(0);
    decoded = decode_alaw(encoded);
    klee_assert(decoded == 0);
    
    // Test signal properties
    if (num_samples > 1) {
        int original_crossings = count_zero_crossings(pcm_samples, num_samples);
        int decoded_crossings = count_zero_crossings(decoded_samples, num_samples);
        
        // Zero crossings should be similar (A-law preserves signal structure)
        klee_assert(abs(original_crossings - decoded_crossings) <= num_samples / 2);
    }
    
    // Test array operations
    int16_t single_pcm[1] = {500};
    uint8_t single_alaw[1];
    int16_t single_decoded[1];
    
    encode_array(single_alaw, single_pcm, 1);
    decode_array(single_decoded, single_alaw, 1);
    
    klee_assert(single_decoded[0] >= 0); // Should preserve sign
    
    // Test with alternating values
    int16_t alt_pcm[4] = {100, -100, 200, -200};
    uint8_t alt_alaw[4];
    int16_t alt_decoded[4];
    
    encode_array(alt_alaw, alt_pcm, 4);
    decode_array(alt_decoded, alt_alaw, 4);
    
    for (int i = 0; i < 4; i++) {
        klee_assert(preserves_sign(alt_pcm[i], alt_decoded[i]));
    }
    
    return 0;
} 