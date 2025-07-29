#include <klee/klee.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define order 2 /**< number of dependent variables in ::problem */

/**
 * @brief Problem statement for a system with first-order differential
 * equations. Updates the system differential variables.
 */
void problem(const double *x, double *y, double *dy)
{
    const double omega = 1.F;       // some const for the problem
    dy[0] = y[1];                   // x dot
    dy[1] = -omega * omega * y[0];  // y dot
}

/**
 * @brief Exact solution of the problem. Used for solution comparison.
 */
void exact_solution(const double *x, double *y)
{
    y[0] = cos(x[0]);
    y[1] = -sin(x[0]);
}

/**
 * @brief Compute next step approximation using the forward-Euler
 * method.
 */
void forward_euler_step(const double dx, const double *x, double *y, double *dy)
{
    int o;
    problem(x, y, dy);
    for (o = 0; o < order; o++) y[o] += dx * dy[o];
}

/**
 * @brief Compute approximation using the forward-Euler
 * method in the given limits.
 */
double forward_euler(double dx, double x0, double x_max, double *y)
{
    double dy[order];
    double x = x0;
    int steps = 0;
    
    do  // iterate for each step of independent variable
    {
        forward_euler_step(dx, &x, y, dy);  // perform integration
        x += dx;                            // update step
        steps++;
    } while (x <= x_max && steps < 1000);  // limit steps to prevent infinite loops
    
    return (double)steps;
}

int main()
{
    double step_size;
    klee_make_symbolic(&step_size, sizeof(step_size), "step_size");
    
    // Constrain step size to reasonable values
    klee_assume(step_size > 0.01 && step_size < 1.0);
    
    double X0 = 0.f;          /* initial value of x0 */
    double X_MAX = 5.F;       /* upper limit of integration */
    double Y0[] = {1.f, 0.f}; /* initial value Y = y(x = x_0) */

    // get approximate solution
    double total_steps = forward_euler(step_size, X0, X_MAX, Y0);
    
    // Verify that we get a reasonable number of steps
    klee_assert(total_steps > 0 && total_steps < 1000);
    
    // Verify that the solution is bounded (for this harmonic oscillator)
    klee_assert(Y0[0] >= -2.0 && Y0[0] <= 2.0);
    klee_assert(Y0[1] >= -2.0 && Y0[1] <= 2.0);
    
    return 0;
} 