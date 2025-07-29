#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

#define MAX_NUMBER 1000

// Calculate sum of proper divisors
unsigned long sum_of_proper_divisors(unsigned long n) {
    if (n <= 1) {
        return 0;
    }
    
    unsigned long sum = 1; // 1 is always a proper divisor
    
    for (unsigned long i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            sum += i;
            unsigned long other_divisor = n / i;
            if (other_divisor != i) {
                sum += other_divisor;
            }
        }
    }
    
    return sum;
}

// Check if number is perfect (sum of proper divisors equals the number)
int is_perfect(unsigned long n) {
    if (n <= 1) {
        return 0;
    }
    return sum_of_proper_divisors(n) == n;
}

// Check if number is abundant (sum of proper divisors greater than the number)
int is_abundant(unsigned long n) {
    if (n <= 1) {
        return 0;
    }
    return sum_of_proper_divisors(n) > n;
}

// Check if number is deficient (sum of proper divisors less than the number)
int is_deficient(unsigned long n) {
    if (n <= 1) {
        return 0;
    }
    return sum_of_proper_divisors(n) < n;
}

// Get number classification: -1 for deficient, 0 for perfect, 1 for abundant
int get_number_classification(unsigned long n) {
    if (n <= 1) {
        return -1; // Deficient
    }
    
    unsigned long sum = sum_of_proper_divisors(n);
    
    if (sum == n) {
        return 0; // Perfect
    } else if (sum > n) {
        return 1; // Abundant
    } else {
        return -1; // Deficient
    }
}

// Find next abundant number after n
unsigned long next_abundant_number(unsigned long n) {
    unsigned long i = n + 1;
    while (!is_abundant(i)) {
        i++;
        if (i > MAX_NUMBER) {
            return 0; // Not found within range
        }
    }
    return i;
}

// Check if number can be expressed as sum of two abundant numbers
int is_sum_of_two_abundant(unsigned long n) {
    if (n < 24) {
        return 0; // Smallest sum of two abundant numbers is 24
    }
    
    for (unsigned long i = 12; i <= n / 2; i++) {
        if (is_abundant(i) && is_abundant(n - i)) {
            return 1;
        }
    }
    
    return 0;
}

// Find all abundant numbers up to n
int find_abundant_numbers(unsigned long n, unsigned long* abundant_list, int max_count) {
    int count = 0;
    
    for (unsigned long i = 12; i <= n && count < max_count; i++) {
        if (is_abundant(i)) {
            abundant_list[count++] = i;
        }
    }
    
    return count;
}

// Check if number is prime
int is_prime(unsigned long n) {
    if (n <= 1) {
        return 0;
    }
    if (n <= 3) {
        return 1;
    }
    if (n % 2 == 0 || n % 3 == 0) {
        return 0;
    }
    
    for (unsigned long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return 0;
        }
    }
    
    return 1;
}

// Count number of divisors
unsigned long count_divisors(unsigned long n) {
    if (n <= 1) {
        return n;
    }
    
    unsigned long count = 2; // 1 and n
    for (unsigned long i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            count += 2; // i and n/i
            if (i * i == n) {
                count--; // Don't count square root twice
            }
        }
    }
    
    return count;
}

// Check if number is square-free
int is_square_free(unsigned long n) {
    if (n <= 1) {
        return 1;
    }
    
    for (unsigned long i = 2; i * i <= n; i++) {
        if (n % (i * i) == 0) {
            return 0;
        }
    }
    
    return 1;
}

// KLEE test function
int main() {
    unsigned long numbers[MAX_NUMBER];
    int num_count;
    
    // Make inputs symbolic
    klee_make_symbolic(&num_count, sizeof(int), "num_count");
    klee_assume(num_count >= 1 && num_count <= MAX_NUMBER);
    
    for (int i = 0; i < num_count; i++) {
        klee_make_symbolic(&numbers[i], sizeof(unsigned long), "number");
        klee_assume(numbers[i] >= 1 && numbers[i] <= MAX_NUMBER);
    }
    
    // Test known perfect numbers
    klee_assert(is_perfect(6));   // 6 = 1 + 2 + 3
    klee_assert(is_perfect(28));  // 28 = 1 + 2 + 4 + 7 + 14
    klee_assert(!is_perfect(12));
    klee_assert(!is_perfect(10));
    
    // Test known abundant numbers
    klee_assert(is_abundant(12)); // 12: 1 + 2 + 3 + 4 + 6 = 16 > 12
    klee_assert(is_abundant(18)); // 18: 1 + 2 + 3 + 6 + 9 = 21 > 18
    klee_assert(!is_abundant(10)); // 10: 1 + 2 + 5 = 8 < 10
    
    // Test known deficient numbers
    klee_assert(is_deficient(10)); // 10: 1 + 2 + 5 = 8 < 10
    klee_assert(is_deficient(8));  // 8: 1 + 2 + 4 = 7 < 8
    klee_assert(!is_deficient(12));
    
    // Test number classification
    klee_assert(get_number_classification(6) == 0);   // Perfect
    klee_assert(get_number_classification(12) == 1);  // Abundant
    klee_assert(get_number_classification(10) == -1); // Deficient
    
    // Test sum of proper divisors
    klee_assert(sum_of_proper_divisors(6) == 6);
    klee_assert(sum_of_proper_divisors(12) == 16);
    klee_assert(sum_of_proper_divisors(10) == 8);
    
    // Test next abundant number
    klee_assert(next_abundant_number(10) == 12);
    klee_assert(next_abundant_number(12) == 18);
    
    // Test sum of two abundant numbers
    klee_assert(is_sum_of_two_abundant(24)); // 12 + 12 = 24
    klee_assert(!is_sum_of_two_abundant(23)); // Cannot be expressed as sum of two abundant
    
    // Test with symbolic inputs
    for (int i = 0; i < num_count; i++) {
        unsigned long n = numbers[i];
        
        // Test that classification is consistent
        int classification = get_number_classification(n);
        klee_assert(classification >= -1 && classification <= 1);
        
        if (classification == 0) {
            klee_assert(is_perfect(n));
            klee_assert(!is_abundant(n));
            klee_assert(!is_deficient(n));
        } else if (classification == 1) {
            klee_assert(is_abundant(n));
            klee_assert(!is_perfect(n));
            klee_assert(!is_deficient(n));
        } else {
            klee_assert(is_deficient(n));
            klee_assert(!is_perfect(n));
            klee_assert(!is_abundant(n));
        }
        
        // Test that sum of proper divisors is consistent
        unsigned long sum = sum_of_proper_divisors(n);
        klee_assert(sum >= 0);
        
        if (n > 1) {
            klee_assert(sum >= 1); // At least 1 is a proper divisor
        }
        
        // Test that abundant numbers have sum > n
        if (is_abundant(n)) {
            klee_assert(sum > n);
        }
        
        // Test that perfect numbers have sum == n
        if (is_perfect(n)) {
            klee_assert(sum == n);
        }
        
        // Test that deficient numbers have sum < n
        if (is_deficient(n)) {
            klee_assert(sum < n);
        }
    }
    
    // Test mathematical properties
    // 1. All prime numbers are deficient
    for (int i = 0; i < num_count; i++) {
        if (is_prime(numbers[i])) {
            klee_assert(is_deficient(numbers[i]));
        }
    }
    
    // 2. Powers of 2 are deficient (except 6)
    for (int i = 0; i < num_count; i++) {
        unsigned long n = numbers[i];
        if (n > 1 && n != 6 && (n & (n - 1)) == 0) { // Power of 2
            klee_assert(is_deficient(n));
        }
    }
    
    // 3. All abundant numbers are even (except 945 and 1575)
    for (int i = 0; i < num_count; i++) {
        if (is_abundant(numbers[i])) {
            if (numbers[i] != 945 && numbers[i] != 1575) {
                klee_assert(numbers[i] % 2 == 0);
            }
        }
    }
    
    // Test edge cases
    // Zero and one
    klee_assert(!is_perfect(0));
    klee_assert(!is_perfect(1));
    klee_assert(!is_abundant(0));
    klee_assert(!is_abundant(1));
    klee_assert(is_deficient(0));
    klee_assert(is_deficient(1));
    
    // Test divisor counting
    klee_assert(count_divisors(6) == 4);   // 1, 2, 3, 6
    klee_assert(count_divisors(12) == 6);  // 1, 2, 3, 4, 6, 12
    klee_assert(count_divisors(16) == 5);  // 1, 2, 4, 8, 16
    
    // Test square-free property
    klee_assert(is_square_free(6));   // 6 = 2 * 3
    klee_assert(is_square_free(10));  // 10 = 2 * 5
    klee_assert(!is_square_free(12)); // 12 = 2^2 * 3
    
    // Test abundant number finding
    unsigned long abundant_list[100];
    int abundant_count = find_abundant_numbers(50, abundant_list, 100);
    klee_assert(abundant_count > 0);
    
    // Verify all found numbers are actually abundant
    for (int i = 0; i < abundant_count; i++) {
        klee_assert(is_abundant(abundant_list[i]));
    }
    
    return 0;
} 