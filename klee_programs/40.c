#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <klee/klee.h>

#define MAX_STEPS 1000
#define MAX_SYSTEM_SIZE 10

// Function pointer type for ODE system
typedef double (*ode_function)(double t, double y, double* params);

// ODE system structure
typedef struct {
    int size;
    double initial_conditions[MAX_SYSTEM_SIZE];
    double parameters[MAX_SYSTEM_SIZE];
    ode_function* functions;
} ODESystem;

// Simple exponential decay: dy/dt = -k*y
double exponential_decay(double t, double y, double* params) {
    double k = params[0];
    return -k * y;
}

// Simple growth: dy/dt = k*y
double exponential_growth(double t, double y, double* params) {
    double k = params[0];
    return k * y;
}

// Logistic growth: dy/dt = k*y*(1-y/M)
double logistic_growth(double t, double y, double* params) {
    double k = params[0];
    double M = params[1];
    return k * y * (1 - y / M);
}

// Harmonic oscillator: d²y/dt² + ω²y = 0
// Converted to system: dy₁/dt = y₂, dy₂/dt = -ω²y₁
double harmonic_oscillator_y1(double t, double y, double* params) {
    double omega = params[0];
    return -omega * omega * y;
}

double harmonic_oscillator_y2(double t, double y, double* params) {
    return y; // This is the velocity component
}

// Midpoint Euler method for single ODE
double midpoint_euler_single(ode_function f, double t0, double y0, 
                           double h, double* params, int steps) {
    double t = t0;
    double y = y0;
    
    for (int i = 0; i < steps; i++) {
        // Calculate midpoint
        double k1 = f(t, y, params);
        double y_mid = y + (h / 2) * k1;
        double t_mid = t + h / 2;
        
        // Use midpoint to calculate next step
        double k2 = f(t_mid, y_mid, params);
        y = y + h * k2;
        t += h;
    }
    
    return y;
}

// Midpoint Euler method for ODE system
void midpoint_euler_system(ODESystem* system, double t0, double h, 
                          int steps, double* solution) {
    double t = t0;
    
    // Copy initial conditions
    for (int i = 0; i < system->size; i++) {
        solution[i] = system->initial_conditions[i];
    }
    
    for (int step = 0; step < steps; step++) {
        double k1[MAX_SYSTEM_SIZE], k2[MAX_SYSTEM_SIZE];
        double y_mid[MAX_SYSTEM_SIZE];
        
        // Calculate k1 values
        for (int i = 0; i < system->size; i++) {
            k1[i] = system->functions[i](t, solution[i], system->parameters);
        }
        
        // Calculate midpoint values
        for (int i = 0; i < system->size; i++) {
            y_mid[i] = solution[i] + (h / 2) * k1[i];
        }
        
        // Calculate k2 values at midpoint
        for (int i = 0; i < system->size; i++) {
            k2[i] = system->functions[i](t + h / 2, y_mid[i], system->parameters);
        }
        
        // Update solution
        for (int i = 0; i < system->size; i++) {
            solution[i] = solution[i] + h * k2[i];
        }
        
        t += h;
    }
}

// Calculate exact solution for exponential decay
double exact_exponential_decay(double t, double y0, double k) {
    return y0 * exp(-k * t);
}

// Calculate exact solution for exponential growth
double exact_exponential_growth(double t, double y0, double k) {
    return y0 * exp(k * t);
}

// Calculate exact solution for logistic growth
double exact_logistic_growth(double t, double y0, double k, double M) {
    return M / (1 + (M / y0 - 1) * exp(-k * t));
}

// Check if solution is reasonable (not NaN or infinite)
int is_solution_valid(double* solution, int size) {
    for (int i = 0; i < size; i++) {
        if (isnan(solution[i]) || isinf(solution[i])) {
            return 0;
        }
    }
    return 1;
}

// Calculate relative error
double relative_error(double exact, double approximate) {
    if (fabs(exact) < 1e-10) {
        return fabs(approximate - exact);
    }
    return fabs((approximate - exact) / exact);
}

// Check if solution is bounded
int is_solution_bounded(double* solution, int size, double bound) {
    for (int i = 0; i < size; i++) {
        if (fabs(solution[i]) > bound) {
            return 0;
        }
    }
    return 1;
}

// Check if solution is monotonic (for appropriate problems)
int is_solution_monotonic(double* solution, int size) {
    if (size < 2) return 1;
    
    int increasing = 1, decreasing = 1;
    for (int i = 1; i < size; i++) {
        if (solution[i] <= solution[i-1]) increasing = 0;
        if (solution[i] >= solution[i-1]) decreasing = 0;
    }
    
    return increasing || decreasing;
}

// KLEE test function
int main() {
    double t0, h, k, M, y0;
    int steps;
    
    // Make inputs symbolic
    klee_make_symbolic(&t0, sizeof(double), "t0");
    klee_make_symbolic(&h, sizeof(double), "h");
    klee_make_symbolic(&k, sizeof(double), "k");
    klee_make_symbolic(&M, sizeof(double), "M");
    klee_make_symbolic(&y0, sizeof(double), "y0");
    klee_make_symbolic(&steps, sizeof(int), "steps");
    
    // Constrain inputs
    klee_assume(t0 >= 0 && t0 <= 100);
    klee_assume(h > 0 && h <= 1);
    klee_assume(k > 0 && k <= 10);
    klee_assume(M > 0 && M <= 100);
    klee_assume(y0 > 0 && y0 <= 100);
    klee_assume(steps >= 1 && steps <= MAX_STEPS);
    
    // Test exponential decay
    double params_decay[1] = {k};
    double solution_decay = midpoint_euler_single(exponential_decay, t0, y0, h, params_decay, steps);
    
    klee_assert(is_solution_valid(&solution_decay, 1));
    klee_assert(solution_decay >= 0); // Solution should be non-negative
    
    // Test exponential growth
    double params_growth[1] = {k};
    double solution_growth = midpoint_euler_single(exponential_growth, t0, y0, h, params_growth, steps);
    
    klee_assert(is_solution_valid(&solution_growth, 1));
    klee_assert(solution_growth > 0); // Solution should be positive
    
    // Test logistic growth
    double params_logistic[2] = {k, M};
    double solution_logistic = midpoint_euler_single(logistic_growth, t0, y0, h, params_logistic, steps);
    
    klee_assert(is_solution_valid(&solution_logistic, 1));
    klee_assert(solution_logistic >= 0); // Solution should be non-negative
    klee_assert(solution_logistic <= M * 1.1); // Should not exceed carrying capacity by much
    
    // Test with known values
    double test_t0 = 0.0;
    double test_h = 0.1;
    double test_k = 1.0;
    double test_y0 = 1.0;
    int test_steps = 10;
    
    double test_params[1] = {test_k};
    double test_solution = midpoint_euler_single(exponential_decay, test_t0, test_y0, test_h, test_params, test_steps);
    
    // Calculate exact solution
    double exact_solution = exact_exponential_decay(test_t0 + test_steps * test_h, test_y0, test_k);
    
    // Check that numerical solution is reasonable
    klee_assert(test_solution > 0);
    klee_assert(test_solution <= test_y0);
    
    // Test ODE system (harmonic oscillator)
    ODESystem harmonic_system;
    harmonic_system.size = 2;
    harmonic_system.initial_conditions[0] = 1.0; // Initial position
    harmonic_system.initial_conditions[1] = 0.0; // Initial velocity
    harmonic_system.parameters[0] = 1.0; // omega
    
    ode_function harmonic_functions[2] = {harmonic_oscillator_y2, harmonic_oscillator_y1};
    harmonic_system.functions = harmonic_functions;
    
    double harmonic_solution[2];
    midpoint_euler_system(&harmonic_system, 0.0, 0.1, 10, harmonic_solution);
    
    klee_assert(is_solution_valid(harmonic_solution, 2));
    
    // Test system properties
    // For harmonic oscillator, energy should be approximately conserved
    double energy = harmonic_solution[0] * harmonic_system.parameters[0] * harmonic_solution[0] + 
                   harmonic_solution[1] * harmonic_solution[1];
    double initial_energy = harmonic_system.initial_conditions[0] * harmonic_system.parameters[0] * 
                           harmonic_system.initial_conditions[0] + 
                           harmonic_system.initial_conditions[1] * harmonic_system.initial_conditions[1];
    
    // Energy should not grow too much (numerical error)
    klee_assert(energy <= initial_energy * 1.5);
    
    // Test with symbolic inputs
    if (steps > 0 && h > 0) {
        // Test that solution is bounded for reasonable parameters
        klee_assert(is_solution_bounded(&solution_decay, 1, y0 * 2));
        klee_assert(is_solution_bounded(&solution_growth, 1, y0 * exp(k * steps * h) * 2));
        
        // Test that exponential decay is decreasing
        if (steps > 1) {
            double solution1 = midpoint_euler_single(exponential_decay, t0, y0, h, params_decay, steps/2);
            double solution2 = midpoint_euler_single(exponential_decay, t0, y0, h, params_decay, steps);
            klee_assert(solution2 <= solution1);
        }
        
        // Test that exponential growth is increasing
        if (steps > 1) {
            double solution1 = midpoint_euler_single(exponential_growth, t0, y0, h, params_growth, steps/2);
            double solution2 = midpoint_euler_single(exponential_growth, t0, y0, h, params_growth, steps);
            klee_assert(solution2 >= solution1);
        }
    }
    
    // Test edge cases
    // Very small step size
    double small_h = 0.001;
    double small_solution = midpoint_euler_single(exponential_decay, 0, 1, small_h, params_decay, 100);
    klee_assert(is_solution_valid(&small_solution, 1));
    
    // Very large step size
    double large_h = 0.5;
    double large_solution = midpoint_euler_single(exponential_decay, 0, 1, large_h, params_decay, 5);
    klee_assert(is_solution_valid(&large_solution, 1));
    
    // Single step
    double single_solution = midpoint_euler_single(exponential_decay, 0, 1, 0.1, params_decay, 1);
    klee_assert(is_solution_valid(&single_solution, 1));
    
    return 0;
} 