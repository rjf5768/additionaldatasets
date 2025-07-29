#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <klee/klee.h>

#define MAX_FEATURES 3
#define MAX_SAMPLES 5

// Adaline neuron structure
struct adaline_neuron {
    double weights[MAX_FEATURES];
    double bias;
    double learning_rate;
    int num_features;
};

// Initialize neuron
void init_neuron(struct adaline_neuron* neuron, int num_features, double learning_rate) {
    neuron->num_features = num_features;
    neuron->learning_rate = learning_rate;
    neuron->bias = 0.0;
    
    // Initialize weights randomly
    for (int i = 0; i < num_features; i++) {
        neuron->weights[i] = (double)(rand() % 100 - 50) / 100.0; // -0.5 to 0.5
    }
}

// Calculate weighted sum
double calculate_net(struct adaline_neuron* neuron, double* input) {
    double net = neuron->bias;
    for (int i = 0; i < neuron->num_features; i++) {
        net += neuron->weights[i] * input[i];
    }
    return net;
}

// Activation function (step function)
int activation_function(double net) {
    return (net >= 0) ? 1 : -1;
}

// Calculate error
double calculate_error(int target, double net) {
    return target - net;
}

// Update weights using gradient descent
void update_weights(struct adaline_neuron* neuron, double* input, double error) {
    // Update bias
    neuron->bias += neuron->learning_rate * error;
    
    // Update weights
    for (int i = 0; i < neuron->num_features; i++) {
        neuron->weights[i] += neuron->learning_rate * error * input[i];
    }
}

// Train neuron
void train_neuron(struct adaline_neuron* neuron, double** inputs, int* targets, 
                  int num_samples, int max_epochs) {
    for (int epoch = 0; epoch < max_epochs; epoch++) {
        double total_error = 0.0;
        
        for (int sample = 0; sample < num_samples; sample++) {
            // Calculate net input
            double net = calculate_net(neuron, inputs[sample]);
            
            // Calculate error
            double error = calculate_error(targets[sample], net);
            total_error += error * error; // Mean squared error
            
            // Update weights
            update_weights(neuron, inputs[sample], error);
        }
        
        // Check for convergence (simplified)
        if (total_error < 0.01) {
            break;
        }
    }
}

// Predict output
int predict(struct adaline_neuron* neuron, double* input) {
    double net = calculate_net(neuron, input);
    return activation_function(net);
}

// Calculate accuracy
double calculate_accuracy(struct adaline_neuron* neuron, double** inputs, 
                         int* targets, int num_samples) {
    int correct = 0;
    for (int i = 0; i < num_samples; i++) {
        int prediction = predict(neuron, inputs[i]);
        if (prediction == targets[i]) {
            correct++;
        }
    }
    return (double)correct / num_samples;
}

// KLEE test function
int main() {
    // Symbolic parameters
    int num_features, num_samples, max_epochs;
    double learning_rate;
    
    klee_make_symbolic(&num_features, sizeof(num_features), "num_features");
    klee_make_symbolic(&num_samples, sizeof(num_samples), "num_samples");
    klee_make_symbolic(&max_epochs, sizeof(max_epochs), "max_epochs");
    klee_make_symbolic(&learning_rate, sizeof(learning_rate), "learning_rate");
    
    // Constrain parameters
    klee_assume(num_features >= 1 && num_features <= 3);
    klee_assume(num_samples >= 1 && num_samples <= 5);
    klee_assume(max_epochs >= 1 && max_epochs <= 10);
    klee_assume(learning_rate > 0.0 && learning_rate <= 1.0);
    
    // Initialize neuron
    struct adaline_neuron neuron;
    init_neuron(&neuron, num_features, learning_rate);
    
    // Allocate training data
    double** inputs = malloc(num_samples * sizeof(double*));
    int* targets = malloc(num_samples * sizeof(int));
    
    for (int i = 0; i < num_samples; i++) {
        inputs[i] = malloc(num_features * sizeof(double));
        for (int j = 0; j < num_features; j++) {
            klee_make_symbolic(&inputs[i][j], sizeof(double), "input");
            klee_assume(inputs[i][j] >= -1.0 && inputs[i][j] <= 1.0);
        }
        klee_make_symbolic(&targets[i], sizeof(int), "target");
        klee_assume(targets[i] == 1 || targets[i] == -1);
    }
    
    // Train neuron
    train_neuron(&neuron, inputs, targets, num_samples, max_epochs);
    
    // Test predictions
    for (int i = 0; i < num_samples; i++) {
        int prediction = predict(&neuron, inputs[i]);
        klee_assert(prediction == 1 || prediction == -1);
        
        // Test net calculation
        double net = calculate_net(&neuron, inputs[i]);
        klee_assert(net >= -100.0 && net <= 100.0); // Reasonable bounds
    }
    
    // Test weight bounds
    for (int i = 0; i < num_features; i++) {
        klee_assert(neuron.weights[i] >= -10.0 && neuron.weights[i] <= 10.0);
    }
    klee_assert(neuron.bias >= -10.0 && neuron.bias <= 10.0);
    
    // Test with symbolic input
    double test_input[MAX_FEATURES];
    klee_make_symbolic(test_input, sizeof(test_input), "test_input");
    for (int i = 0; i < num_features; i++) {
        klee_assume(test_input[i] >= -1.0 && test_input[i] <= 1.0);
    }
    
    int test_prediction = predict(&neuron, test_input);
    klee_assert(test_prediction == 1 || test_prediction == -1);
    
    // Test error calculation
    double test_net = calculate_net(&neuron, test_input);
    double test_error = calculate_error(1, test_net);
    klee_assert(test_error >= -100.0 && test_error <= 100.0);
    
    // Cleanup
    for (int i = 0; i < num_samples; i++) {
        free(inputs[i]);
    }
    free(inputs);
    free(targets);
    
    return 0;
} 