#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <klee/klee.h>

#define MAX_DIM 5
#define MAX_FEATURES 3

// Simplified 3D array structure for KLEE
struct kohonen_array_3d {
    int dim1, dim2, dim3;
    double data[MAX_DIM * MAX_DIM * MAX_FEATURES];
};

// Get data from 3D array
double* kohonen_data_3d(const struct kohonen_array_3d *arr, int x, int y, int z) {
    int offset = (x * arr->dim2 * arr->dim3) + (y * arr->dim3) + z;
    return (double*)(arr->data + offset);
}

// Calculate Euclidean distance
double euclidean_distance(const double *a, const double *b, int dim) {
    double sum = 0.0;
    for (int i = 0; i < dim; i++) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

// Find best matching unit (BMU)
void find_bmu(const double *input, struct kohonen_array_3d *W, 
              int num_out, int num_features, int *bmu_x, int *bmu_y) {
    double min_dist = INFINITY;
    *bmu_x = 0;
    *bmu_y = 0;
    
    for (int i = 0; i < num_out; i++) {
        for (int j = 0; j < num_out; j++) {
            double *weights = kohonen_data_3d(W, i, j, 0);
            double dist = euclidean_distance(input, weights, num_features);
            
            if (dist < min_dist) {
                min_dist = dist;
                *bmu_x = i;
                *bmu_y = j;
            }
        }
    }
}

// Update weights using neighborhood function
void update_weights(const double *input, struct kohonen_array_3d *W, 
                   int bmu_x, int bmu_y, int num_out, int num_features, 
                   double alpha, double sigma) {
    
    for (int i = 0; i < num_out; i++) {
        for (int j = 0; j < num_out; j++) {
            // Calculate distance from BMU
            double dist = sqrt((i - bmu_x) * (i - bmu_x) + (j - bmu_y) * (j - bmu_y));
            
            // Neighborhood function (Gaussian)
            double neighborhood = exp(-(dist * dist) / (2 * sigma * sigma));
            
            // Update weights
            for (int k = 0; k < num_features; k++) {
                double *weight = kohonen_data_3d(W, i, j, k);
                *weight += alpha * neighborhood * (input[k] - *weight);
            }
        }
    }
}

// Initialize weights randomly
void init_weights(struct kohonen_array_3d *W, int num_out, int num_features) {
    for (int i = 0; i < num_out; i++) {
        for (int j = 0; j < num_out; j++) {
            for (int k = 0; k < num_features; k++) {
                double *weight = kohonen_data_3d(W, i, j, k);
                *weight = (double)(rand() % 100) / 100.0;  // Random between 0 and 1
            }
        }
    }
}

// Train SOM
void train_som(double **data, struct kohonen_array_3d *W, 
               int num_samples, int num_features, int num_out, 
               int max_iterations) {
    
    init_weights(W, num_out, num_features);
    
    for (int iter = 0; iter < max_iterations; iter++) {
        // Learning rate and neighborhood size decrease over time
        double alpha = 0.1 * exp(-(double)iter / max_iterations);
        double sigma = 2.0 * exp(-(double)iter / max_iterations);
        
        for (int sample = 0; sample < num_samples; sample++) {
            int bmu_x, bmu_y;
            find_bmu(data[sample], W, num_out, num_features, &bmu_x, &bmu_y);
            update_weights(data[sample], W, bmu_x, bmu_y, num_out, num_features, alpha, sigma);
        }
    }
}

// KLEE test function
int main() {
    // Symbolic parameters
    int num_samples, num_features, num_out, max_iterations;
    klee_make_symbolic(&num_samples, sizeof(num_samples), "num_samples");
    klee_make_symbolic(&num_features, sizeof(num_features), "num_features");
    klee_make_symbolic(&num_out, sizeof(num_out), "num_out");
    klee_make_symbolic(&max_iterations, sizeof(max_iterations), "max_iterations");
    
    // Constrain parameters to reasonable ranges
    klee_assume(num_samples >= 1 && num_samples <= 3);
    klee_assume(num_features >= 1 && num_features <= 3);
    klee_assume(num_out >= 2 && num_out <= 4);
    klee_assume(max_iterations >= 1 && max_iterations <= 10);
    
    // Allocate and initialize data
    double **data = malloc(num_samples * sizeof(double*));
    for (int i = 0; i < num_samples; i++) {
        data[i] = malloc(num_features * sizeof(double));
        for (int j = 0; j < num_features; j++) {
            klee_make_symbolic(&data[i][j], sizeof(double), "data");
            klee_assume(data[i][j] >= 0.0 && data[i][j] <= 1.0);
        }
    }
    
    // Initialize SOM weights
    struct kohonen_array_3d W;
    W.dim1 = num_out;
    W.dim2 = num_out;
    W.dim3 = num_features;
    
    // Train SOM
    train_som(data, &W, num_samples, num_features, num_out, max_iterations);
    
    // Test with symbolic input
    double test_input[MAX_FEATURES];
    klee_make_symbolic(test_input, sizeof(test_input), "test_input");
    for (int i = 0; i < num_features; i++) {
        klee_assume(test_input[i] >= 0.0 && test_input[i] <= 1.0);
    }
    
    // Find BMU for test input
    int bmu_x, bmu_y;
    find_bmu(test_input, &W, num_out, num_features, &bmu_x, &bmu_y);
    
    // Add assertions for KLEE exploration
    klee_assert(bmu_x >= 0 && bmu_x < num_out);
    klee_assert(bmu_y >= 0 && bmu_y < num_out);
    
    // Test weight updates
    double *weight = kohonen_data_3d(&W, bmu_x, bmu_y, 0);
    klee_assert(*weight >= 0.0 && *weight <= 1.0);
    
    // Test that weights are properly initialized
    for (int i = 0; i < num_out; i++) {
        for (int j = 0; j < num_out; j++) {
            for (int k = 0; k < num_features; k++) {
                double *w = kohonen_data_3d(&W, i, j, k);
                klee_assert(*w >= 0.0 && *w <= 1.0);
            }
        }
    }
    
    // Cleanup
    for (int i = 0; i < num_samples; i++) {
        free(data[i]);
    }
    free(data);
    
    return 0;
} 