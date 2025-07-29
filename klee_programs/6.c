#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <klee/klee.h>

#define MAX_DIM 4
#define MAX_FEATURES 2

// Simplified SOM structure for KLEE
struct som_network {
    double weights[MAX_DIM * MAX_DIM * MAX_FEATURES];
    int dim_x, dim_y, features;
    double learning_rate;
    double neighborhood_size;
};

// Get weight from 3D array
double* get_weight(struct som_network* som, int x, int y, int f) {
    int offset = (x * som->dim_y * som->features) + (y * som->features) + f;
    return &som->weights[offset];
}

// Calculate Euclidean distance
double euclidean_distance(const double* a, const double* b, int dim) {
    double sum = 0.0;
    for (int i = 0; i < dim; i++) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

// Find best matching unit
void find_bmu(struct som_network* som, const double* input, int* bmu_x, int* bmu_y) {
    double min_dist = INFINITY;
    *bmu_x = 0;
    *bmu_y = 0;
    
    for (int i = 0; i < som->dim_x; i++) {
        for (int j = 0; j < som->dim_y; j++) {
            double dist = 0.0;
            for (int f = 0; f < som->features; f++) {
                double* weight = get_weight(som, i, j, f);
                double diff = input[f] - *weight;
                dist += diff * diff;
            }
            dist = sqrt(dist);
            
            if (dist < min_dist) {
                min_dist = dist;
                *bmu_x = i;
                *bmu_y = j;
            }
        }
    }
}

// Update weights using neighborhood function
void update_weights(struct som_network* som, const double* input, int bmu_x, int bmu_y) {
    for (int i = 0; i < som->dim_x; i++) {
        for (int j = 0; j < som->dim_y; j++) {
            // Calculate distance from BMU
            double dist = sqrt((i - bmu_x) * (i - bmu_x) + (j - bmu_y) * (j - bmu_y));
            
            // Neighborhood function (Gaussian)
            double neighborhood = exp(-(dist * dist) / (2 * som->neighborhood_size * som->neighborhood_size));
            
            // Update weights
            for (int f = 0; f < som->features; f++) {
                double* weight = get_weight(som, i, j, f);
                *weight += som->learning_rate * neighborhood * (input[f] - *weight);
            }
        }
    }
}

// Initialize network
void init_som(struct som_network* som, int dim_x, int dim_y, int features) {
    som->dim_x = dim_x;
    som->dim_y = dim_y;
    som->features = features;
    som->learning_rate = 0.1;
    som->neighborhood_size = 2.0;
    
    // Initialize weights randomly
    for (int i = 0; i < dim_x * dim_y * features; i++) {
        som->weights[i] = (double)(rand() % 100) / 100.0;
    }
}

// Train SOM with tracing
void train_som_trace(struct som_network* som, double** data, int num_samples, int max_iterations) {
    for (int iter = 0; iter < max_iterations; iter++) {
        // Update learning parameters
        som->learning_rate = 0.1 * exp(-(double)iter / max_iterations);
        som->neighborhood_size = 2.0 * exp(-(double)iter / max_iterations);
        
        for (int sample = 0; sample < num_samples; sample++) {
            int bmu_x, bmu_y;
            find_bmu(som, data[sample], &bmu_x, &bmu_y);
            update_weights(som, data[sample], bmu_x, bmu_y);
        }
    }
}

// Trace network state
void trace_network(struct som_network* som, const char* label) {
    // This would normally write to a file, but for KLEE we just validate
    for (int i = 0; i < som->dim_x; i++) {
        for (int j = 0; j < som->dim_y; j++) {
            for (int f = 0; f < som->features; f++) {
                double* weight = get_weight(som, i, j, f);
                klee_assert(*weight >= 0.0 && *weight <= 1.0);
            }
        }
    }
}

// KLEE test function
int main() {
    // Symbolic parameters
    int dim_x, dim_y, features, num_samples, max_iterations;
    klee_make_symbolic(&dim_x, sizeof(dim_x), "dim_x");
    klee_make_symbolic(&dim_y, sizeof(dim_y), "dim_y");
    klee_make_symbolic(&features, sizeof(features), "features");
    klee_make_symbolic(&num_samples, sizeof(num_samples), "num_samples");
    klee_make_symbolic(&max_iterations, sizeof(max_iterations), "max_iterations");
    
    // Constrain parameters
    klee_assume(dim_x >= 2 && dim_x <= 4);
    klee_assume(dim_y >= 2 && dim_y <= 4);
    klee_assume(features >= 1 && features <= 2);
    klee_assume(num_samples >= 1 && num_samples <= 3);
    klee_assume(max_iterations >= 1 && max_iterations <= 5);
    
    // Initialize SOM
    struct som_network som;
    init_som(&som, dim_x, dim_y, features);
    
    // Allocate training data
    double** data = malloc(num_samples * sizeof(double*));
    for (int i = 0; i < num_samples; i++) {
        data[i] = malloc(features * sizeof(double));
        for (int j = 0; j < features; j++) {
            klee_make_symbolic(&data[i][j], sizeof(double), "data");
            klee_assume(data[i][j] >= 0.0 && data[i][j] <= 1.0);
        }
    }
    
    // Trace initial state
    trace_network(&som, "initial");
    
    // Train network
    train_som_trace(&som, data, num_samples, max_iterations);
    
    // Trace final state
    trace_network(&som, "final");
    
    // Test with symbolic input
    double test_input[MAX_FEATURES];
    klee_make_symbolic(test_input, sizeof(test_input), "test_input");
    for (int i = 0; i < features; i++) {
        klee_assume(test_input[i] >= 0.0 && test_input[i] <= 1.0);
    }
    
    // Find BMU for test input
    int bmu_x, bmu_y;
    find_bmu(&som, test_input, &bmu_x, &bmu_y);
    
    // Add assertions
    klee_assert(bmu_x >= 0 && bmu_x < dim_x);
    klee_assert(bmu_y >= 0 && bmu_y < dim_y);
    
    // Test weight convergence
    double total_weight_change = 0.0;
    for (int i = 0; i < dim_x * dim_y * features; i++) {
        total_weight_change += som.weights[i];
    }
    klee_assert(total_weight_change >= 0.0);
    
    // Cleanup
    for (int i = 0; i < num_samples; i++) {
        free(data[i]);
    }
    free(data);
    
    return 0;
} 