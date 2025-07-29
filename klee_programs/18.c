#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

#define MAX_DEGREE 10

typedef struct Term {
    int coefficient;
    int exponent;
} Term;

typedef struct Polynomial {
    Term terms[MAX_DEGREE];
    int degree;
    int num_terms;
} Polynomial;

// Initialize polynomial
void init_polynomial(Polynomial* poly) {
    poly->degree = 0;
    poly->num_terms = 0;
    for (int i = 0; i < MAX_DEGREE; i++) {
        poly->terms[i].coefficient = 0;
        poly->terms[i].exponent = 0;
    }
}

// Add term to polynomial
void add_term(Polynomial* poly, int coeff, int exp) {
    if (poly->num_terms >= MAX_DEGREE) return;
    
    // Check if term with same exponent already exists
    for (int i = 0; i < poly->num_terms; i++) {
        if (poly->terms[i].exponent == exp) {
            poly->terms[i].coefficient += coeff;
            return;
        }
    }
    
    // Add new term
    poly->terms[poly->num_terms].coefficient = coeff;
    poly->terms[poly->num_terms].exponent = exp;
    poly->num_terms++;
    
    // Update degree
    if (exp > poly->degree) {
        poly->degree = exp;
    }
}

// Sort polynomial by exponent (descending)
void sort_polynomial(Polynomial* poly) {
    for (int i = 0; i < poly->num_terms - 1; i++) {
        for (int j = 0; j < poly->num_terms - i - 1; j++) {
            if (poly->terms[j].exponent < poly->terms[j + 1].exponent) {
                Term temp = poly->terms[j];
                poly->terms[j] = poly->terms[j + 1];
                poly->terms[j + 1] = temp;
            }
        }
    }
}

// Add two polynomials
Polynomial add_polynomials(Polynomial* poly1, Polynomial* poly2) {
    Polynomial result;
    init_polynomial(&result);
    
    // Add terms from first polynomial
    for (int i = 0; i < poly1->num_terms; i++) {
        add_term(&result, poly1->terms[i].coefficient, poly1->terms[i].exponent);
    }
    
    // Add terms from second polynomial
    for (int i = 0; i < poly2->num_terms; i++) {
        add_term(&result, poly2->terms[i].coefficient, poly2->terms[i].exponent);
    }
    
    // Sort result
    sort_polynomial(&result);
    
    return result;
}

// Evaluate polynomial at x
int evaluate_polynomial(Polynomial* poly, int x) {
    int result = 0;
    
    for (int i = 0; i < poly->num_terms; i++) {
        int term_value = poly->terms[i].coefficient;
        for (int j = 0; j < poly->terms[i].exponent; j++) {
            term_value *= x;
        }
        result += term_value;
    }
    
    return result;
}

// Check if polynomial is valid
int is_valid_polynomial(Polynomial* poly) {
    if (poly->num_terms < 0 || poly->num_terms > MAX_DEGREE) {
        return 0;
    }
    
    for (int i = 0; i < poly->num_terms; i++) {
        if (poly->terms[i].exponent < 0) {
            return 0;
        }
    }
    
    return 1;
}

// Get coefficient of term with given exponent
int get_coefficient(Polynomial* poly, int exp) {
    for (int i = 0; i < poly->num_terms; i++) {
        if (poly->terms[i].exponent == exp) {
            return poly->terms[i].coefficient;
        }
    }
    return 0;
}

// Check if polynomials are equal
int polynomials_equal(Polynomial* poly1, Polynomial* poly2) {
    if (poly1->num_terms != poly2->num_terms) {
        return 0;
    }
    
    for (int i = 0; i < poly1->num_terms; i++) {
        if (poly1->terms[i].coefficient != poly2->terms[i].coefficient ||
            poly1->terms[i].exponent != poly2->terms[i].exponent) {
            return 0;
        }
    }
    
    return 1;
}

// KLEE test function
int main() {
    Polynomial poly1, poly2;
    init_polynomial(&poly1);
    init_polynomial(&poly2);
    
    // Symbolic inputs
    int coeff1, exp1, coeff2, exp2, coeff3, exp3, coeff4, exp4;
    int eval_x;
    
    klee_make_symbolic(&coeff1, sizeof(coeff1), "coeff1");
    klee_make_symbolic(&exp1, sizeof(exp1), "exp1");
    klee_make_symbolic(&coeff2, sizeof(coeff2), "exp2");
    klee_make_symbolic(&exp2, sizeof(exp2), "exp2");
    klee_make_symbolic(&coeff3, sizeof(coeff3), "coeff3");
    klee_make_symbolic(&exp3, sizeof(exp3), "exp3");
    klee_make_symbolic(&coeff4, sizeof(coeff4), "coeff4");
    klee_make_symbolic(&exp4, sizeof(exp4), "exp4");
    klee_make_symbolic(&eval_x, sizeof(eval_x), "eval_x");
    
    // Constrain inputs
    klee_assume(coeff1 >= -10 && coeff1 <= 10);
    klee_assume(exp1 >= 0 && exp1 <= 5);
    klee_assume(coeff2 >= -10 && coeff2 <= 10);
    klee_assume(exp2 >= 0 && exp2 <= 5);
    klee_assume(coeff3 >= -10 && coeff3 <= 10);
    klee_assume(exp3 >= 0 && exp3 <= 5);
    klee_assume(coeff4 >= -10 && coeff4 <= 10);
    klee_assume(exp4 >= 0 && exp4 <= 5);
    klee_assume(eval_x >= -5 && eval_x <= 5);
    
    // Add terms to first polynomial
    add_term(&poly1, coeff1, exp1);
    klee_assert(is_valid_polynomial(&poly1));
    klee_assert(poly1.num_terms == 1);
    
    add_term(&poly1, coeff2, exp2);
    klee_assert(is_valid_polynomial(&poly1));
    klee_assert(poly1.num_terms <= 2);
    
    // Add terms to second polynomial
    add_term(&poly2, coeff3, exp3);
    klee_assert(is_valid_polynomial(&poly2));
    klee_assert(poly2.num_terms == 1);
    
    add_term(&poly2, coeff4, exp4);
    klee_assert(is_valid_polynomial(&poly2));
    klee_assert(poly2.num_terms <= 2);
    
    // Sort polynomials
    sort_polynomial(&poly1);
    sort_polynomial(&poly2);
    
    // Test polynomial addition
    Polynomial result = add_polynomials(&poly1, &poly2);
    klee_assert(is_valid_polynomial(&result));
    klee_assert(result.num_terms <= poly1.num_terms + poly2.num_terms);
    
    // Test evaluation
    int eval1 = evaluate_polynomial(&poly1, eval_x);
    int eval2 = evaluate_polynomial(&poly2, eval_x);
    int eval_result = evaluate_polynomial(&result, eval_x);
    
    // Result evaluation should equal sum of individual evaluations
    klee_assert(eval_result == eval1 + eval2);
    
    // Test coefficient retrieval
    int coeff_result1 = get_coefficient(&result, exp1);
    int coeff_result2 = get_coefficient(&result, exp2);
    int coeff_result3 = get_coefficient(&result, exp3);
    int coeff_result4 = get_coefficient(&result, exp4);
    
    // Verify coefficients are reasonable
    klee_assert(coeff_result1 >= -20 && coeff_result1 <= 20);
    klee_assert(coeff_result2 >= -20 && coeff_result2 <= 20);
    klee_assert(coeff_result3 >= -20 && coeff_result3 <= 20);
    klee_assert(coeff_result4 >= -20 && coeff_result4 <= 20);
    
    // Test polynomial properties
    klee_assert(result.degree >= 0 && result.degree <= 5);
    klee_assert(result.num_terms >= 0 && result.num_terms <= 4);
    
    // Test with zero polynomial
    Polynomial zero_poly;
    init_polynomial(&zero_poly);
    
    Polynomial result2 = add_polynomials(&poly1, &zero_poly);
    klee_assert(polynomials_equal(&result2, &poly1));
    
    // Test commutativity
    Polynomial result3 = add_polynomials(&poly2, &poly1);
    klee_assert(polynomials_equal(&result, &result3));
    
    return 0;
} 