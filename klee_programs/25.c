#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <klee/klee.h>

#define MAX_DEGREE 10
#define MAX_ITERATIONS 100
#define ACCURACY 1e-6

typedef struct {
    double real;
    double imag;
} Complex;

// Create complex number
Complex make_complex(double real, double imag) {
    Complex c;
    c.real = real;
    c.imag = imag;
    return c;
}

// Complex addition
Complex complex_add(Complex a, Complex b) {
    return make_complex(a.real + b.real, a.imag + b.imag);
}

// Complex subtraction
Complex complex_sub(Complex a, Complex b) {
    return make_complex(a.real - b.real, a.imag - b.imag);
}

// Complex multiplication
Complex complex_mul(Complex a, Complex b) {
    return make_complex(a.real * b.real - a.imag * b.imag,
                       a.real * b.imag + a.imag * b.real);
}

// Complex division
Complex complex_div(Complex a, Complex b) {
    double denominator = b.real * b.real + b.imag * b.imag;
    if (denominator == 0) {
        return make_complex(0, 0); // Return zero for division by zero
    }
    return make_complex((a.real * b.real + a.imag * b.imag) / denominator,
                       (a.imag * b.real - a.real * b.imag) / denominator);
}

// Complex magnitude
double complex_magnitude(Complex c) {
    return sqrt(c.real * c.real + c.imag * c.imag);
}

// Evaluate polynomial at complex point
Complex evaluate_polynomial(double coeffs[], int degree, Complex x) {
    Complex result = make_complex(0, 0);
    Complex power = make_complex(1, 0);
    
    for (int i = degree - 1; i >= 0; i--) {
        Complex term = complex_mul(power, make_complex(coeffs[i], 0));
        result = complex_add(result, term);
        power = complex_mul(power, x);
    }
    
    return result;
}

// Evaluate derivative at complex point
Complex evaluate_derivative(double coeffs[], int degree, Complex x) {
    Complex result = make_complex(0, 0);
    Complex power = make_complex(1, 0);
    
    for (int i = degree - 1; i >= 1; i--) {
        Complex term = complex_mul(power, make_complex(coeffs[i] * i, 0));
        result = complex_add(result, term);
        power = complex_mul(power, x);
    }
    
    return result;
}

// Durand-Kerner method for finding roots
int durand_kerner_roots(double coeffs[], int degree, Complex roots[]) {
    if (degree <= 0) return 0;
    
    // Initialize roots with equidistant points on unit circle
    for (int i = 0; i < degree; i++) {
        double angle = 2 * M_PI * i / degree;
        roots[i] = make_complex(cos(angle), sin(angle));
    }
    
    // Iterative refinement
    for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
        int converged = 1;
        
        for (int i = 0; i < degree; i++) {
            Complex numerator = evaluate_polynomial(coeffs, degree, roots[i]);
            Complex denominator = make_complex(1, 0);
            
            // Calculate denominator: product of (root[i] - root[j]) for j != i
            for (int j = 0; j < degree; j++) {
                if (i != j) {
                    Complex diff = complex_sub(roots[i], roots[j]);
                    denominator = complex_mul(denominator, diff);
                }
            }
            
            // Update root
            Complex correction = complex_div(numerator, denominator);
            Complex new_root = complex_sub(roots[i], correction);
            
            // Check convergence
            if (complex_magnitude(correction) > ACCURACY) {
                converged = 0;
            }
            
            roots[i] = new_root;
        }
        
        if (converged) break;
    }
    
    return 1;
}

// Newton's method for finding a single root
Complex newton_method(double coeffs[], int degree, Complex initial_guess) {
    Complex x = initial_guess;
    
    for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
        Complex fx = evaluate_polynomial(coeffs, degree, x);
        Complex fprime = evaluate_derivative(coeffs, degree, x);
        
        if (complex_magnitude(fprime) < ACCURACY) {
            break; // Avoid division by zero
        }
        
        Complex correction = complex_div(fx, fprime);
        Complex new_x = complex_sub(x, correction);
        
        if (complex_magnitude(correction) < ACCURACY) {
            break; // Converged
        }
        
        x = new_x;
    }
    
    return x;
}

// Check if complex number is approximately zero
int is_approximately_zero(Complex c) {
    return complex_magnitude(c) < ACCURACY;
}

// Check if two complex numbers are approximately equal
int complex_approx_equal(Complex a, Complex b) {
    Complex diff = complex_sub(a, b);
    return complex_magnitude(diff) < ACCURACY;
}

// KLEE test function
int main() {
    double coeffs[MAX_DEGREE];
    Complex roots[MAX_DEGREE];
    Complex test_point;
    
    // Make coefficients symbolic
    for (int i = 0; i < MAX_DEGREE; i++) {
        klee_make_symbolic(&coeffs[i], sizeof(double), "coeff");
        klee_assume(coeffs[i] >= -100 && coeffs[i] <= 100);
    }
    
    // Make test point symbolic
    klee_make_symbolic(&test_point.real, sizeof(double), "test_real");
    klee_make_symbolic(&test_point.imag, sizeof(double), "test_imag");
    klee_assume(test_point.real >= -10 && test_point.real <= 10);
    klee_assume(test_point.imag >= -10 && test_point.imag <= 10);
    
    // Test polynomial evaluation
    Complex result = evaluate_polynomial(coeffs, MAX_DEGREE, test_point);
    klee_assert(!isnan(result.real) && !isnan(result.imag));
    
    // Test complex arithmetic
    Complex a = make_complex(1, 2);
    Complex b = make_complex(3, 4);
    
    Complex sum = complex_add(a, b);
    klee_assert(sum.real == 4 && sum.imag == 6);
    
    Complex diff = complex_sub(a, b);
    klee_assert(diff.real == -2 && diff.imag == -2);
    
    Complex product = complex_mul(a, b);
    klee_assert(product.real == -5 && product.imag == 10);
    
    Complex quotient = complex_div(a, b);
    klee_assert(quotient.real == 0.44 && quotient.imag == 0.08);
    
    // Test magnitude
    double mag = complex_magnitude(a);
    klee_assert(mag == sqrt(5));
    
    // Test with simple polynomial: x^2 - 1 = 0
    double simple_coeffs[3] = {1, 0, -1}; // x^2 - 1
    Complex simple_roots[2];
    
    int success = durand_kerner_roots(simple_coeffs, 2, simple_roots);
    klee_assert(success);
    
    // Check that roots are approximately ±1
    int found_plus_one = 0, found_minus_one = 0;
    for (int i = 0; i < 2; i++) {
        if (complex_approx_equal(simple_roots[i], make_complex(1, 0))) {
            found_plus_one = 1;
        }
        if (complex_approx_equal(simple_roots[i], make_complex(-1, 0))) {
            found_minus_one = 1;
        }
    }
    klee_assert(found_plus_one && found_minus_one);
    
    // Test Newton's method
    Complex newton_root = newton_method(simple_coeffs, 2, make_complex(2, 0));
    klee_assert(complex_approx_equal(newton_root, make_complex(1, 0)) ||
                complex_approx_equal(newton_root, make_complex(-1, 0)));
    
    // Test with cubic polynomial: x^3 - 1 = 0
    double cubic_coeffs[4] = {1, 0, 0, -1}; // x^3 - 1
    Complex cubic_roots[3];
    
    success = durand_kerner_roots(cubic_coeffs, 3, cubic_roots);
    klee_assert(success);
    
    // Check that one root is approximately 1
    int found_one = 0;
    for (int i = 0; i < 3; i++) {
        if (complex_approx_equal(cubic_roots[i], make_complex(1, 0))) {
            found_one = 1;
            break;
        }
    }
    klee_assert(found_one);
    
    // Test edge cases
    // Zero polynomial
    double zero_coeffs[MAX_DEGREE] = {0};
    Complex zero_roots[MAX_DEGREE];
    success = durand_kerner_roots(zero_coeffs, MAX_DEGREE, zero_roots);
    klee_assert(success);
    
    // Constant polynomial
    double const_coeffs[1] = {5}; // f(x) = 5
    Complex const_roots[1];
    success = durand_kerner_roots(const_coeffs, 1, const_roots);
    klee_assert(success);
    
    // Linear polynomial: x - 2 = 0
    double linear_coeffs[2] = {1, -2}; // x - 2
    Complex linear_roots[1];
    success = durand_kerner_roots(linear_coeffs, 1, linear_roots);
    klee_assert(success);
    klee_assert(complex_approx_equal(linear_roots[0], make_complex(2, 0)));
    
    return 0;
} 