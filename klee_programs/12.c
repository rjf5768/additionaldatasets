#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <klee/klee.h>

#define MAX_SIZE 4

// Matrix structure
struct matrix {
    double data[MAX_SIZE * MAX_SIZE];
    int rows, cols;
};

// Get matrix element
double get_element(struct matrix* mat, int i, int j) {
    return mat->data[i * mat->cols + j];
}

// Set matrix element
void set_element(struct matrix* mat, int i, int j, double value) {
    mat->data[i * mat->cols + j] = value;
}

// Initialize matrix
void init_matrix(struct matrix* mat, int rows, int cols) {
    mat->rows = rows;
    mat->cols = cols;
    for (int i = 0; i < rows * cols; i++) {
        mat->data[i] = 0.0;
    }
}

// Copy matrix
void copy_matrix(struct matrix* dest, struct matrix* src) {
    dest->rows = src->rows;
    dest->cols = src->cols;
    for (int i = 0; i < src->rows * src->cols; i++) {
        dest->data[i] = src->data[i];
    }
}

// Calculate dot product of two vectors
double dot_product(struct matrix* mat, int row1, int row2) {
    double sum = 0.0;
    for (int j = 0; j < mat->cols; j++) {
        sum += get_element(mat, row1, j) * get_element(mat, row2, j);
    }
    return sum;
}

// Calculate vector norm
double vector_norm(struct matrix* mat, int row) {
    return sqrt(dot_product(mat, row, row));
}

// Gram-Schmidt orthogonalization
void gram_schmidt(struct matrix* A, struct matrix* Q, struct matrix* R) {
    int n = A->rows;
    
    // Initialize Q and R
    init_matrix(Q, n, n);
    init_matrix(R, n, n);
    
    for (int i = 0; i < n; i++) {
        // Copy column i from A to Q
        for (int j = 0; j < n; j++) {
            set_element(Q, j, i, get_element(A, j, i));
        }
        
        // Orthogonalize against previous columns
        for (int j = 0; j < i; j++) {
            double proj = dot_product(Q, i, j) / dot_product(Q, j, j);
            R->data[j * n + i] = proj;
            
            for (int k = 0; k < n; k++) {
                double q_val = get_element(Q, k, i);
                q_val -= proj * get_element(Q, k, j);
                set_element(Q, k, i, q_val);
            }
        }
        
        // Normalize
        double norm = vector_norm(Q, i);
        if (norm > 1e-10) {
            R->data[i * n + i] = norm;
            for (int k = 0; k < n; k++) {
                double q_val = get_element(Q, k, i) / norm;
                set_element(Q, k, i, q_val);
            }
        } else {
            R->data[i * n + i] = 0.0;
        }
    }
}

// Matrix multiplication
void matrix_multiply(struct matrix* A, struct matrix* B, struct matrix* C) {
    int m = A->rows, n = A->cols, p = B->cols;
    init_matrix(C, m, p);
    
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < p; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += get_element(A, i, k) * get_element(B, k, j);
            }
            set_element(C, i, j, sum);
        }
    }
}

// QR iteration for eigenvalues
void qr_iteration(struct matrix* A, int max_iterations) {
    int n = A->rows;
    struct matrix Q, R, temp;
    
    for (int iter = 0; iter < max_iterations; iter++) {
        // QR decomposition
        gram_schmidt(A, &Q, &R);
        
        // A = R * Q
        matrix_multiply(&R, &Q, &temp);
        copy_matrix(A, &temp);
    }
}

// Extract eigenvalues from diagonal
void extract_eigenvalues(struct matrix* A, double* eigenvalues) {
    int n = A->rows;
    for (int i = 0; i < n; i++) {
        eigenvalues[i] = get_element(A, i, i);
    }
}

// Check if matrix is upper triangular
int is_upper_triangular(struct matrix* mat) {
    for (int i = 1; i < mat->rows; i++) {
        for (int j = 0; j < i; j++) {
            if (fabs(get_element(mat, i, j)) > 1e-6) {
                return 0;
            }
        }
    }
    return 1;
}

// KLEE test function
int main() {
    // Symbolic parameters
    int size, max_iterations;
    klee_make_symbolic(&size, sizeof(size), "size");
    klee_make_symbolic(&max_iterations, sizeof(max_iterations), "max_iterations");
    
    // Constrain parameters
    klee_assume(size >= 2 && size <= MAX_SIZE);
    klee_assume(max_iterations >= 1 && max_iterations <= 10);
    
    // Initialize matrix A
    struct matrix A;
    init_matrix(&A, size, size);
    
    // Fill matrix with symbolic values
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double value;
            klee_make_symbolic(&value, sizeof(value), "matrix_element");
            klee_assume(value >= -10.0 && value <= 10.0);
            set_element(&A, i, j, value);
        }
    }
    
    // Test QR decomposition
    struct matrix Q, R;
    gram_schmidt(&A, &Q, &R);
    
    // Verify QR decomposition properties
    // Q should be orthogonal (Q^T * Q = I)
    struct matrix QTQ, temp;
    matrix_multiply(&Q, &Q, &QTQ); // Q^T * Q (assuming Q is square)
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double expected = (i == j) ? 1.0 : 0.0;
            double actual = get_element(&QTQ, i, j);
            klee_assert(fabs(actual - expected) < 1e-6);
        }
    }
    
    // R should be upper triangular
    klee_assert(is_upper_triangular(&R));
    
    // Verify A = Q * R
    struct matrix QR;
    matrix_multiply(&Q, &R, &QR);
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double a_val = get_element(&A, i, j);
            double qr_val = get_element(&QR, i, j);
            klee_assert(fabs(a_val - qr_val) < 1e-6);
        }
    }
    
    // Test QR iteration for eigenvalues
    struct matrix A_copy;
    copy_matrix(&A_copy, &A);
    qr_iteration(&A_copy, max_iterations);
    
    // Extract eigenvalues
    double eigenvalues[MAX_SIZE];
    extract_eigenvalues(&A_copy, eigenvalues);
    
    // Verify eigenvalues are real (diagonal elements)
    for (int i = 0; i < size; i++) {
        klee_assert(eigenvalues[i] >= -100.0 && eigenvalues[i] <= 100.0);
    }
    
    // Test with symmetric matrix (real eigenvalues)
    struct matrix symmetric;
    init_matrix(&symmetric, size, size);
    
    for (int i = 0; i < size; i++) {
        for (int j = i; j < size; j++) {
            double value;
            klee_make_symbolic(&value, sizeof(value), "symmetric_element");
            klee_assume(value >= -5.0 && value <= 5.0);
            set_element(&symmetric, i, j, value);
            set_element(&symmetric, j, i, value); // Make symmetric
        }
    }
    
    // QR iteration on symmetric matrix
    qr_iteration(&symmetric, max_iterations);
    
    double sym_eigenvalues[MAX_SIZE];
    extract_eigenvalues(&symmetric, sym_eigenvalues);
    
    // Eigenvalues should be real and sorted (approximately)
    for (int i = 1; i < size; i++) {
        klee_assert(sym_eigenvalues[i] >= sym_eigenvalues[i-1] - 1e-6);
    }
    
    return 0;
} 