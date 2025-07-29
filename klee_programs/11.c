#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <klee/klee.h>

#define MAX_POINTS 10
#define MAX_CLUSTERS 5
#define MAX_DIMENSIONS 3

// Point structure
struct point {
    double coords[MAX_DIMENSIONS];
    int cluster_id;
    int id;
};

// Cluster structure
struct cluster {
    double centroid[MAX_DIMENSIONS];
    int size;
    int id;
};

// Calculate Euclidean distance between two points
double distance(struct point* p1, struct point* p2, int dimensions) {
    double sum = 0.0;
    for (int i = 0; i < dimensions; i++) {
        double diff = p1->coords[i] - p2->coords[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

// Calculate distance between point and centroid
double distance_to_centroid(struct point* p, struct cluster* c, int dimensions) {
    double sum = 0.0;
    for (int i = 0; i < dimensions; i++) {
        double diff = p->coords[i] - c->centroid[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

// Assign points to nearest cluster
void assign_to_clusters(struct point* points, struct cluster* clusters, 
                       int num_points, int num_clusters, int dimensions) {
    for (int i = 0; i < num_points; i++) {
        double min_distance = INFINITY;
        int best_cluster = 0;
        
        for (int j = 0; j < num_clusters; j++) {
            double dist = distance_to_centroid(&points[i], &clusters[j], dimensions);
            if (dist < min_distance) {
                min_distance = dist;
                best_cluster = j;
            }
        }
        
        points[i].cluster_id = best_cluster;
    }
}

// Update cluster centroids
void update_centroids(struct point* points, struct cluster* clusters, 
                     int num_points, int num_clusters, int dimensions) {
    // Reset cluster sizes
    for (int i = 0; i < num_clusters; i++) {
        clusters[i].size = 0;
        for (int j = 0; j < dimensions; j++) {
            clusters[i].centroid[j] = 0.0;
        }
    }
    
    // Sum up points in each cluster
    for (int i = 0; i < num_points; i++) {
        int cluster_id = points[i].cluster_id;
        clusters[cluster_id].size++;
        for (int j = 0; j < dimensions; j++) {
            clusters[cluster_id].centroid[j] += points[i].coords[j];
        }
    }
    
    // Calculate centroids
    for (int i = 0; i < num_clusters; i++) {
        if (clusters[i].size > 0) {
            for (int j = 0; j < dimensions; j++) {
                clusters[i].centroid[j] /= clusters[i].size;
            }
        }
    }
}

// Initialize clusters with random centroids
void init_clusters(struct cluster* clusters, int num_clusters, int dimensions) {
    for (int i = 0; i < num_clusters; i++) {
        clusters[i].id = i;
        clusters[i].size = 0;
        for (int j = 0; j < dimensions; j++) {
            clusters[i].centroid[j] = (double)(rand() % 100) / 100.0;
        }
    }
}

// K-means clustering algorithm
void kmeans(struct point* points, struct cluster* clusters, 
           int num_points, int num_clusters, int dimensions, int max_iterations) {
    
    init_clusters(clusters, num_clusters, dimensions);
    
    for (int iter = 0; iter < max_iterations; iter++) {
        // Assign points to clusters
        assign_to_clusters(points, clusters, num_points, num_clusters, dimensions);
        
        // Update centroids
        update_centroids(points, clusters, num_points, num_clusters, dimensions);
    }
}

// Calculate total within-cluster sum of squares
double calculate_wcss(struct point* points, struct cluster* clusters, 
                     int num_points, int num_clusters, int dimensions) {
    double total_wcss = 0.0;
    
    for (int i = 0; i < num_points; i++) {
        int cluster_id = points[i].cluster_id;
        double dist = distance_to_centroid(&points[i], &clusters[cluster_id], dimensions);
        total_wcss += dist * dist;
    }
    
    return total_wcss;
}

// Check if all points are assigned to clusters
int all_points_assigned(struct point* points, int num_points) {
    for (int i = 0; i < num_points; i++) {
        if (points[i].cluster_id < 0) {
            return 0;
        }
    }
    return 1;
}

// KLEE test function
int main() {
    // Symbolic parameters
    int num_points, num_clusters, dimensions, max_iterations;
    klee_make_symbolic(&num_points, sizeof(num_points), "num_points");
    klee_make_symbolic(&num_clusters, sizeof(num_clusters), "num_clusters");
    klee_make_symbolic(&dimensions, sizeof(dimensions), "dimensions");
    klee_make_symbolic(&max_iterations, sizeof(max_iterations), "max_iterations");
    
    // Constrain parameters
    klee_assume(num_points >= 2 && num_points <= MAX_POINTS);
    klee_assume(num_clusters >= 1 && num_clusters <= MAX_CLUSTERS);
    klee_assume(dimensions >= 1 && dimensions <= MAX_DIMENSIONS);
    klee_assume(max_iterations >= 1 && max_iterations <= 10);
    klee_assume(num_clusters <= num_points); // Can't have more clusters than points
    
    // Allocate arrays
    struct point* points = malloc(num_points * sizeof(struct point));
    struct cluster* clusters = malloc(num_clusters * sizeof(struct cluster));
    
    // Initialize points with symbolic data
    for (int i = 0; i < num_points; i++) {
        points[i].id = i;
        points[i].cluster_id = -1; // Unassigned initially
        for (int j = 0; j < dimensions; j++) {
            klee_make_symbolic(&points[i].coords[j], sizeof(double), "coord");
            klee_assume(points[i].coords[j] >= 0.0 && points[i].coords[j] <= 1.0);
        }
    }
    
    // Run K-means
    kmeans(points, clusters, num_points, num_clusters, dimensions, max_iterations);
    
    // Verify clustering results
    klee_assert(all_points_assigned(points, num_points));
    
    // Check that all cluster IDs are valid
    for (int i = 0; i < num_points; i++) {
        klee_assert(points[i].cluster_id >= 0 && points[i].cluster_id < num_clusters);
    }
    
    // Check that clusters have valid centroids
    for (int i = 0; i < num_clusters; i++) {
        for (int j = 0; j < dimensions; j++) {
            klee_assert(clusters[i].centroid[j] >= 0.0 && clusters[i].centroid[j] <= 1.0);
        }
    }
    
    // Test cluster assignment consistency
    for (int i = 0; i < num_points; i++) {
        int assigned_cluster = points[i].cluster_id;
        double min_distance = distance_to_centroid(&points[i], &clusters[assigned_cluster], dimensions);
        
        // Check that no other cluster is closer
        for (int j = 0; j < num_clusters; j++) {
            if (j != assigned_cluster) {
                double dist = distance_to_centroid(&points[i], &clusters[j], dimensions);
                klee_assert(min_distance <= dist);
            }
        }
    }
    
    // Calculate and verify WCSS
    double wcss = calculate_wcss(points, clusters, num_points, num_clusters, dimensions);
    klee_assert(wcss >= 0.0);
    
    // Test with symbolic query point
    struct point query_point;
    klee_make_symbolic(query_point.coords, sizeof(query_point.coords), "query_coords");
    for (int j = 0; j < dimensions; j++) {
        klee_assume(query_point.coords[j] >= 0.0 && query_point.coords[j] <= 1.0);
    }
    
    // Find nearest cluster for query point
    double min_dist = INFINITY;
    int nearest_cluster = 0;
    for (int i = 0; i < num_clusters; i++) {
        double dist = distance_to_centroid(&query_point, &clusters[i], dimensions);
        if (dist < min_dist) {
            min_dist = dist;
            nearest_cluster = i;
        }
    }
    
    klee_assert(nearest_cluster >= 0 && nearest_cluster < num_clusters);
    klee_assert(min_dist >= 0.0);
    
    // Test cluster size consistency
    int total_assigned = 0;
    for (int i = 0; i < num_clusters; i++) {
        total_assigned += clusters[i].size;
    }
    klee_assert(total_assigned == num_points);
    
    free(points);
    free(clusters);
    return 0;
} 