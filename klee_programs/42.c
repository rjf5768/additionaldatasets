#include <klee/klee.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * store arbitratily large integer values
 * as a linked list of digits.
 */
typedef struct _big_int
{
    char value;                  /**< tens place (single digit) */
    struct _big_int *next_digit; /**< hundreds place */
    struct _big_int *prev_digit; /**< units place */
} big_int;

/**
 * Function that allocates memory to add another
 * digit at the MSB
 */
big_int *add_digit(big_int *digit, char value)
{
    if (digit == NULL)
    {
        digit = (big_int *)malloc(sizeof(big_int));
        if (!digit)
        {
            return NULL;
        }
        digit->value = value;
        digit->next_digit = NULL;
        digit->prev_digit = NULL;
        return digit;
    }

    if (digit->next_digit)
    {
        digit->next_digit->value = value;
        return digit->next_digit;
    }

    digit->next_digit = (big_int *)malloc(sizeof(big_int));
    if (digit->next_digit == NULL)
    {
        return NULL;
    }
    digit->next_digit->value = value;
    digit->next_digit->next_digit = NULL;
    digit->next_digit->prev_digit = digit;
    return digit->next_digit;
}

/**
 * Function to remove digits preceeding the
 * current digit.
 */
char remove_digits(big_int *digit, int N)
{
    if (digit == NULL)
        return 0;

    if (digit->next_digit == NULL)
    {
        free(digit);
        digit = NULL;
        return 0;
    }

    if (N > 0)
        return remove_digits(digit->next_digit, N - 1);

    return remove_digits(digit->next_digit, 0);
}

int main()
{
    unsigned int N;
    klee_make_symbolic(&N, sizeof(N), "N");
    
    // Constrain N to reasonable values for testing
    klee_assume(N >= 1 && N <= 10);
    
    big_int *ptr = add_digit(NULL, 1); /* start with 1 */
    const big_int *ptr0 = ptr;         /* save the first location */
    unsigned long sum_digits = 0;
    unsigned long num_digits = 0;

    for (unsigned int i = 1; i <= N; i++)
    {
        int carry = 0;
        ptr = (big_int *)ptr0; /* multiply every digit with i */
        while (ptr)
        {
            unsigned int tmp = ptr->value * i + carry;
            if (tmp >= 10)
            {
                div_t tmp2 = div(tmp, 10);
                carry = tmp2.quot;
                tmp = tmp2.rem;
            }
            else
                carry = 0;

            if (carry > 0 && ptr->next_digit == NULL)
                add_digit(ptr, 0);

            ptr->value = tmp;

            if (i == N)
                sum_digits += tmp;

            if (ptr->next_digit)
                ptr = ptr->next_digit;
            else
                break;
        }
    }

    /* Print the result */
    do
    {
        putchar(ptr->value + 0x30); /* convert digit to ASCII char */
        ptr = ptr->prev_digit;
        num_digits++;
    } while (ptr);

    // Verify that factorial is always positive
    klee_assert(sum_digits > 0);
    
    // Verify that N! has at least N digits for N > 5
    if (N > 5) {
        klee_assert(num_digits >= N);
    }

    remove_digits((big_int *)ptr0, -1);
    return 0;
} 