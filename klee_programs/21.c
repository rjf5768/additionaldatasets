#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <klee/klee.h>

#define MAX_POINTS 100
#define PI 3.14159265359

typedef struct Point {
    double x, y;
} Point;

typedef struct Spirograph {
    double R;  // Fixed circle radius
    double r;  // Moving circle radius
    double d;  // Distance from center of moving circle
    Point points[MAX_POINTS];
    int num_points;
} Spirograph;

// Initialize spirograph
void init_spirograph(Spirograph* spiro, double R, double r, double d) {
    spiro->R = R;
    spiro->r = r;
    spiro->d = d;
    spiro->num_points = 0;
}

// Calculate spirograph point
Point calculate_point(Spirograph* spiro, double t) {
    Point p;
    
    // Parametric equations for spirograph
    double x = (spiro->R - spiro->r) * cos(t) + spiro->d * cos((spiro->R - spiro->r) * t / spiro->r);
    double y = (spiro->R - spiro->r) * sin(t) - spiro->d * sin((spiro->R - spiro->r) * t / spiro->r);
    
    p.x = x;
    p.y = y;
    return p;
}

// Generate spirograph curve
void generate_curve(Spirograph* spiro, int num_points) {
    if (num_points > MAX_POINTS) num_points = MAX_POINTS;
    
    spiro->num_points = num_points;
    
    for (int i = 0; i < num_points; i++) {
        double t = 2 * PI * i / num_points;
        spiro->points[i] = calculate_point(spiro, t);
    }
}

// Calculate curve length (approximation)
double calculate_curve_length(Spirograph* spiro) {
    if (spiro->num_points < 2) return 0.0;
    
    double length = 0.0;
    for (int i = 1; i < spiro->num_points; i++) {
        double dx = spiro->points[i].x - spiro->points[i-1].x;
        double dy = spiro->points[i].y - spiro->points[i-1].y;
        length += sqrt(dx*dx + dy*dy);
    }
    
    return length;
}

// Calculate bounding box
void calculate_bounding_box(Spirograph* spiro, double* min_x, double* max_x, double* min_y, double* max_y) {
    if (spiro->num_points == 0) {
        *min_x = *max_x = *min_y = *max_y = 0.0;
        return;
    }
    
    *min_x = *max_x = spiro->points[0].x;
    *min_y = *max_y = spiro->points[0].y;
    
    for (int i = 1; i < spiro->num_points; i++) {
        if (spiro->points[i].x < *min_x) *min_x = spiro->points[i].x;
        if (spiro->points[i].x > *max_x) *max_x = spiro->points[i].x;
        if (spiro->points[i].y < *min_y) *min_y = spiro->points[i].y;
        if (spiro->points[i].y > *max_y) *max_y = spiro->points[i].y;
    }
}

// Check if point is on curve (approximate)
int is_point_on_curve(Spirograph* spiro, Point p, double tolerance) {
    for (int i = 0; i < spiro->num_points; i++) {
        double dx = spiro->points[i].x - p.x;
        double dy = spiro->points[i].y - p.y;
        double distance = sqrt(dx*dx + dy*dy);
        if (distance <= tolerance) {
            return 1;
        }
    }
    return 0;
}

// Calculate area enclosed by curve (approximation using shoelace formula)
double calculate_area(Spirograph* spiro) {
    if (spiro->num_points < 3) return 0.0;
    
    double area = 0.0;
    for (int i = 0; i < spiro->num_points; i++) {
        int j = (i + 1) % spiro->num_points;
        area += spiro->points[i].x * spiro->points[j].y;
        area -= spiro->points[j].x * spiro->points[i].y;
    }
    
    return fabs(area) / 2.0;
}

// Check if parameters are valid
int are_valid_parameters(double R, double r, double d) {
    return (R > 0 && r > 0 && d >= 0 && r <= R);
}

// KLEE test function
int main() {
    Spirograph spiro;
    
    // Symbolic inputs
    double R, r, d;
    int num_points;
    double test_x, test_y;
    
    klee_make_symbolic(&R, sizeof(R), "R");
    klee_make_symbolic(&r, sizeof(r), "r");
    klee_make_symbolic(&d, sizeof(d), "d");
    klee_make_symbolic(&num_points, sizeof(num_points), "num_points");
    klee_make_symbolic(&test_x, sizeof(test_x), "test_x");
    klee_make_symbolic(&test_y, sizeof(test_y), "test_y");
    
    // Constrain parameters
    klee_assume(R > 0.1 && R <= 10.0);
    klee_assume(r > 0.1 && r <= 5.0);
    klee_assume(d >= 0.0 && d <= 3.0);
    klee_assume(r <= R);  // Moving circle must fit inside fixed circle
    klee_assume(num_points >= 10 && num_points <= 50);
    klee_assume(test_x >= -20.0 && test_x <= 20.0);
    klee_assume(test_y >= -20.0 && test_y <= 20.0);
    
    // Initialize spirograph
    init_spirograph(&spiro, R, r, d);
    klee_assert(are_valid_parameters(R, r, d));
    
    // Generate curve
    generate_curve(&spiro, num_points);
    klee_assert(spiro.num_points > 0);
    klee_assert(spiro.num_points <= MAX_POINTS);
    
    // Test point calculation
    Point test_point = calculate_point(&spiro, 0.0);
    klee_assert(test_point.x >= -20.0 && test_point.x <= 20.0);
    klee_assert(test_point.y >= -20.0 && test_point.y <= 20.0);
    
    // Test curve properties
    double length = calculate_curve_length(&spiro);
    klee_assert(length >= 0.0);
    klee_assert(length <= 1000.0); // Reasonable upper bound
    
    // Test bounding box
    double min_x, max_x, min_y, max_y;
    calculate_bounding_box(&spiro, &min_x, &max_x, &min_y, &max_y);
    klee_assert(min_x <= max_x);
    klee_assert(min_y <= max_y);
    klee_assert(max_x - min_x <= 50.0); // Reasonable bounds
    klee_assert(max_y - min_y <= 50.0);
    
    // Test area calculation
    double area = calculate_area(&spiro);
    klee_assert(area >= 0.0);
    klee_assert(area <= 1000.0); // Reasonable upper bound
    
    // Test point on curve
    Point test_p = {test_x, test_y};
    int on_curve = is_point_on_curve(&spiro, test_p, 0.1);
    klee_assert(on_curve >= 0 && on_curve <= 1);
    
    // Test curve closure (first and last points should be close)
    if (spiro.num_points > 1) {
        double dx = spiro.points[0].x - spiro.points[spiro.num_points-1].x;
        double dy = spiro.points[0].y - spiro.points[spiro.num_points-1].y;
        double closure_distance = sqrt(dx*dx + dy*dy);
        klee_assert(closure_distance <= 10.0); // Should be reasonably closed
    }
    
    // Test parameter relationships
    klee_assert(spiro.R > spiro.r); // Fixed circle must be larger
    klee_assert(spiro.d >= 0.0);    // Distance must be non-negative
    
    // Test curve smoothness (consecutive points should be reasonably close)
    for (int i = 1; i < spiro.num_points; i++) {
        double dx = spiro.points[i].x - spiro.points[i-1].x;
        double dy = spiro.points[i].y - spiro.points[i-1].y;
        double step_distance = sqrt(dx*dx + dy*dy);
        klee_assert(step_distance <= 5.0); // Steps should be reasonably small
    }
    
    // Test with different parameter combinations
    if (d == 0.0) {
        // When d=0, curve should be a circle
        klee_assert(length > 0.0);
    }
    
    if (r == R) {
        // When r=R, curve should be a point or line
        klee_assert(length >= 0.0);
    }
    
    return 0;
} 