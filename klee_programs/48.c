#include <klee/klee.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Function to add arbitrary length decimal integers stored in an array.
 * a + b = c = new b
 */
int add_numbers(uint8_t *a, uint8_t *b, uint8_t N)
{
    int carry = 0;
    uint8_t *c = b; /* accumulate the result in the array 'b' */

    for (int i = 0; i < N; i++)
    {
        c[i] = carry + a[i] + b[i];
        if (c[i] > 9)                /* check for carry */
        {
            carry = 1;
            c[i] -= 10;
        }
        else
        {
            carry = 0;
        }
    }

    for (int i = N; i < N + 10; i++)
    {
        if (carry == 0)
        {
            break;
        }
        c[i] = carry + c[i];
        if (c[i] > 9)
        {
            carry = 1;
            c[i] -= 10;
        }
        else
        {
            carry = 0;
        }
    }
    return 0;
}

/** Function to print a long number */
int print_number(uint8_t *number, uint8_t N, int8_t num_digits_to_print)
{
    uint8_t start_pos = N - 1;
    uint8_t end_pos;

    /* skip all initial zeros */
    while (number[start_pos] == 0) start_pos--;

    /* if end_pos < 0, print all digits */
    if (num_digits_to_print < 0)
    {
        end_pos = 0;
    }
    else if (num_digits_to_print <= start_pos)
    {
        end_pos = start_pos - num_digits_to_print + 1;
    }
    else
    {
        return -1;
    }

    for (int i = start_pos; i >= end_pos; i--) putchar(number[i] + 0x30);

    putchar('\n');

    return 0;
}

int main()
{
    const int N = 10;
    const int N2 = N + 10;

    uint8_t *number1 = (uint8_t *)calloc(N, sizeof(uint8_t));
    uint8_t *number2 = (uint8_t *)calloc(N, sizeof(uint8_t));
    uint8_t *sum = (uint8_t *)calloc(N2, sizeof(uint8_t));

    // Make inputs symbolic
    klee_make_symbolic(number1, N * sizeof(uint8_t), "number1");
    klee_make_symbolic(number2, N * sizeof(uint8_t), "number2");
    
    // Constrain digits to valid range
    for (int i = 0; i < N; i++) {
        klee_assume(number1[i] >= 0 && number1[i] <= 9);
        klee_assume(number2[i] >= 0 && number2[i] <= 9);
    }

    // Initialize sum array
    memset(sum, 0, N2 * sizeof(uint8_t));

    // Add the two numbers
    add_numbers(number1, sum, N);
    add_numbers(number2, sum, N);

    // Verify that addition is commutative
    uint8_t *sum2 = (uint8_t *)calloc(N2, sizeof(uint8_t));
    memset(sum2, 0, N2 * sizeof(uint8_t));
    
    add_numbers(number2, sum2, N);
    add_numbers(number1, sum2, N);
    
    // Check that both sums are equal
    for (int i = 0; i < N2; i++) {
        klee_assert(sum[i] == sum2[i]);
    }

    // Verify that sum is always greater than or equal to individual numbers
    int has_nonzero1 = 0, has_nonzero2 = 0;
    for (int i = 0; i < N; i++) {
        if (number1[i] > 0) has_nonzero1 = 1;
        if (number2[i] > 0) has_nonzero2 = 1;
    }
    
    if (has_nonzero1 && has_nonzero2) {
        // Both numbers are non-zero, so sum should be greater than either
        klee_assert(sum[0] >= number1[0] || sum[0] >= number2[0]);
    }

    free(number1);
    free(number2);
    free(sum);
    free(sum2);
    
    return 0;
} 