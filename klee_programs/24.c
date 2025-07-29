#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <klee/klee.h>

typedef struct {
    float x, y, z;
} Vector3D;

// Vector addition
Vector3D vector_add(Vector3D a, Vector3D b) {
    Vector3D result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

// Vector subtraction
Vector3D vector_sub(Vector3D a, Vector3D b) {
    Vector3D result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

// Dot product
float dot_product(Vector3D a, Vector3D b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Cross product
Vector3D cross_product(Vector3D a, Vector3D b) {
    Vector3D result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

// Vector magnitude
float magnitude(Vector3D v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

// Normalize vector
Vector3D normalize(Vector3D v) {
    float mag = magnitude(v);
    Vector3D result;
    if (mag > 0) {
        result.x = v.x / mag;
        result.y = v.y / mag;
        result.z = v.z / mag;
    } else {
        result.x = result.y = result.z = 0;
    }
    return result;
}

// Scalar multiplication
Vector3D scalar_multiply(Vector3D v, float scalar) {
    Vector3D result;
    result.x = v.x * scalar;
    result.y = v.y * scalar;
    result.z = v.z * scalar;
    return result;
}

// Check if two vectors are equal
int vectors_equal(Vector3D a, Vector3D b) {
    return (a.x == b.x && a.y == b.y && a.z == b.z);
}

// Check if vector is zero
int is_zero_vector(Vector3D v) {
    return (v.x == 0 && v.y == 0 && v.z == 0);
}

// Angle between two vectors (in radians)
float angle_between(Vector3D a, Vector3D b) {
    float dot = dot_product(a, b);
    float mag_a = magnitude(a);
    float mag_b = magnitude(b);
    
    if (mag_a == 0 || mag_b == 0) {
        return 0;
    }
    
    float cos_angle = dot / (mag_a * mag_b);
    if (cos_angle > 1) cos_angle = 1;
    if (cos_angle < -1) cos_angle = -1;
    
    return acos(cos_angle);
}

// Distance between two points
float distance(Vector3D a, Vector3D b) {
    Vector3D diff = vector_sub(a, b);
    return magnitude(diff);
}

// KLEE test function
int main() {
    Vector3D v1, v2, v3;
    
    // Make vectors symbolic
    klee_make_symbolic(&v1.x, sizeof(float), "v1_x");
    klee_make_symbolic(&v1.y, sizeof(float), "v1_y");
    klee_make_symbolic(&v1.z, sizeof(float), "v1_z");
    
    klee_make_symbolic(&v2.x, sizeof(float), "v2_x");
    klee_make_symbolic(&v2.y, sizeof(float), "v2_y");
    klee_make_symbolic(&v2.z, sizeof(float), "v2_z");
    
    klee_make_symbolic(&v3.x, sizeof(float), "v3_x");
    klee_make_symbolic(&v3.y, sizeof(float), "v3_y");
    klee_make_symbolic(&v3.z, sizeof(float), "v3_z");
    
    // Constrain inputs to reasonable ranges
    klee_assume(v1.x >= -100 && v1.x <= 100);
    klee_assume(v1.y >= -100 && v1.y <= 100);
    klee_assume(v1.z >= -100 && v1.z <= 100);
    
    klee_assume(v2.x >= -100 && v2.x <= 100);
    klee_assume(v2.y >= -100 && v2.y <= 100);
    klee_assume(v2.z >= -100 && v2.z <= 100);
    
    klee_assume(v3.x >= -100 && v3.x <= 100);
    klee_assume(v3.y >= -100 && v3.y <= 100);
    klee_assume(v3.z >= -100 && v3.z <= 100);
    
    // Test vector addition
    Vector3D sum = vector_add(v1, v2);
    klee_assert(sum.x == v1.x + v2.x);
    klee_assert(sum.y == v1.y + v2.y);
    klee_assert(sum.z == v1.z + v2.z);
    
    // Test vector subtraction
    Vector3D diff = vector_sub(v1, v2);
    klee_assert(diff.x == v1.x - v2.x);
    klee_assert(diff.y == v1.y - v2.y);
    klee_assert(diff.z == v1.z - v2.z);
    
    // Test dot product
    float dot = dot_product(v1, v2);
    klee_assert(dot == v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
    
    // Test cross product
    Vector3D cross = cross_product(v1, v2);
    klee_assert(cross.x == v1.y * v2.z - v1.z * v2.y);
    klee_assert(cross.y == v1.z * v2.x - v1.x * v2.z);
    klee_assert(cross.z == v1.x * v2.y - v1.y * v2.x);
    
    // Test magnitude
    float mag1 = magnitude(v1);
    klee_assert(mag1 >= 0);
    
    // Test scalar multiplication
    float scalar = 2.5;
    Vector3D scaled = scalar_multiply(v1, scalar);
    klee_assert(scaled.x == v1.x * scalar);
    klee_assert(scaled.y == v1.y * scalar);
    klee_assert(scaled.z == v1.z * scalar);
    
    // Test vector properties
    // Commutativity of addition
    Vector3D sum1 = vector_add(v1, v2);
    Vector3D sum2 = vector_add(v2, v1);
    klee_assert(vectors_equal(sum1, sum2));
    
    // Associativity of addition
    Vector3D sum3 = vector_add(vector_add(v1, v2), v3);
    Vector3D sum4 = vector_add(v1, vector_add(v2, v3));
    klee_assert(vectors_equal(sum3, sum4));
    
    // Distributivity of scalar multiplication
    Vector3D scaled_sum = scalar_multiply(vector_add(v1, v2), scalar);
    Vector3D sum_scaled = vector_add(scalar_multiply(v1, scalar), scalar_multiply(v2, scalar));
    klee_assert(vectors_equal(scaled_sum, sum_scaled));
    
    // Cross product properties
    // v1 × v2 = -(v2 × v1)
    Vector3D cross1 = cross_product(v1, v2);
    Vector3D cross2 = cross_product(v2, v1);
    Vector3D neg_cross2 = scalar_multiply(cross2, -1);
    klee_assert(vectors_equal(cross1, neg_cross2));
    
    // v1 × v1 = 0
    Vector3D self_cross = cross_product(v1, v1);
    klee_assert(is_zero_vector(self_cross));
    
    // Test normalization
    if (magnitude(v1) > 0) {
        Vector3D normalized = normalize(v1);
        float norm_mag = magnitude(normalized);
        klee_assert(norm_mag > 0.99 && norm_mag < 1.01); // Allow small floating point errors
    }
    
    // Test angle calculation
    if (magnitude(v1) > 0 && magnitude(v2) > 0) {
        float angle = angle_between(v1, v2);
        klee_assert(angle >= 0 && angle <= 3.14159); // 0 to π
    }
    
    // Test distance
    float dist = distance(v1, v2);
    klee_assert(dist >= 0);
    
    // Test edge cases
    Vector3D zero = {0, 0, 0};
    klee_assert(is_zero_vector(zero));
    klee_assert(magnitude(zero) == 0);
    
    // Test with zero vector
    Vector3D sum_with_zero = vector_add(v1, zero);
    klee_assert(vectors_equal(sum_with_zero, v1));
    
    Vector3D diff_with_zero = vector_sub(v1, zero);
    klee_assert(vectors_equal(diff_with_zero, v1));
    
    float dot_with_zero = dot_product(v1, zero);
    klee_assert(dot_with_zero == 0);
    
    Vector3D cross_with_zero = cross_product(v1, zero);
    klee_assert(is_zero_vector(cross_with_zero));
    
    return 0;
} 