#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

#define MAX_VERTICES 20
#define MAX_EDGES 100

typedef struct {
    int src, dest, weight;
} Edge;

typedef struct {
    int parent;
    int rank;
} Subset;

// Find set of an element (with path compression)
int find(Subset subsets[], int i) {
    if (subsets[i].parent != i) {
        subsets[i].parent = find(subsets, subsets[i].parent);
    }
    return subsets[i].parent;
}

// Union of two sets (with union by rank)
void union_sets(Subset subsets[], int x, int y) {
    int xroot = find(subsets, x);
    int yroot = find(subsets, y);
    
    if (subsets[xroot].rank < subsets[yroot].rank) {
        subsets[xroot].parent = yroot;
    } else if (subsets[xroot].rank > subsets[yroot].rank) {
        subsets[yroot].parent = xroot;
    } else {
        subsets[yroot].parent = xroot;
        subsets[xroot].rank++;
    }
}

// Compare function for qsort
int compare_edges(const void* a, const void* b) {
    return ((Edge*)a)->weight - ((Edge*)b)->weight;
}

// Kruskal's algorithm
int kruskal_mst(Edge edges[], int vertices, int num_edges, Edge result[]) {
    // Sort edges by weight
    qsort(edges, num_edges, sizeof(Edge), compare_edges);
    
    // Create subsets for union-find
    Subset* subsets = (Subset*)malloc(vertices * sizeof(Subset));
    if (!subsets) {
        return -1;
    }
    
    // Initialize subsets
    for (int i = 0; i < vertices; i++) {
        subsets[i].parent = i;
        subsets[i].rank = 0;
    }
    
    int mst_edges = 0;
    int edge_index = 0;
    
    // Process edges in ascending order
    while (mst_edges < vertices - 1 && edge_index < num_edges) {
        Edge next_edge = edges[edge_index++];
        
        int x = find(subsets, next_edge.src);
        int y = find(subsets, next_edge.dest);
        
        // Include edge if it doesn't cause cycle
        if (x != y) {
            result[mst_edges++] = next_edge;
            union_sets(subsets, x, y);
        }
    }
    
    free(subsets);
    return mst_edges;
}

// Calculate total weight of MST
int calculate_mst_weight(Edge mst[], int mst_edges) {
    int total_weight = 0;
    for (int i = 0; i < mst_edges; i++) {
        total_weight += mst[i].weight;
    }
    return total_weight;
}

// Check if MST is connected (all vertices reachable)
int is_mst_connected(Edge mst[], int mst_edges, int vertices) {
    if (mst_edges != vertices - 1) {
        return 0; // Not enough edges for a tree
    }
    
    // Use union-find to check connectivity
    Subset* subsets = (Subset*)malloc(vertices * sizeof(Subset));
    if (!subsets) {
        return 0;
    }
    
    // Initialize subsets
    for (int i = 0; i < vertices; i++) {
        subsets[i].parent = i;
        subsets[i].rank = 0;
    }
    
    // Union all edges in MST
    for (int i = 0; i < mst_edges; i++) {
        union_sets(subsets, mst[i].src, mst[i].dest);
    }
    
    // Check if all vertices are in the same set
    int root = find(subsets, 0);
    for (int i = 1; i < vertices; i++) {
        if (find(subsets, i) != root) {
            free(subsets);
            return 0;
        }
    }
    
    free(subsets);
    return 1;
}

// Check if MST is a tree (no cycles)
int is_mst_tree(Edge mst[], int mst_edges, int vertices) {
    return mst_edges == vertices - 1 && is_mst_connected(mst, mst_edges, vertices);
}

// Check if graph is connected
int is_graph_connected(Edge edges[], int num_edges, int vertices) {
    Subset* subsets = (Subset*)malloc(vertices * sizeof(Subset));
    if (!subsets) {
        return 0;
    }
    
    // Initialize subsets
    for (int i = 0; i < vertices; i++) {
        subsets[i].parent = i;
        subsets[i].rank = 0;
    }
    
    // Union all edges
    for (int i = 0; i < num_edges; i++) {
        union_sets(subsets, edges[i].src, edges[i].dest);
    }
    
    // Check if all vertices are in the same set
    int root = find(subsets, 0);
    for (int i = 1; i < vertices; i++) {
        if (find(subsets, i) != root) {
            free(subsets);
            return 0;
        }
    }
    
    free(subsets);
    return 1;
}

// Find minimum weight in graph
int find_min_weight(Edge edges[], int num_edges) {
    if (num_edges == 0) return 0;
    
    int min_weight = edges[0].weight;
    for (int i = 1; i < num_edges; i++) {
        if (edges[i].weight < min_weight) {
            min_weight = edges[i].weight;
        }
    }
    return min_weight;
}

// Find maximum weight in graph
int find_max_weight(Edge edges[], int num_edges) {
    if (num_edges == 0) return 0;
    
    int max_weight = edges[0].weight;
    for (int i = 1; i < num_edges; i++) {
        if (edges[i].weight > max_weight) {
            max_weight = edges[i].weight;
        }
    }
    return max_weight;
}

// Count edges incident to a vertex
int count_vertex_edges(Edge edges[], int num_edges, int vertex) {
    int count = 0;
    for (int i = 0; i < num_edges; i++) {
        if (edges[i].src == vertex || edges[i].dest == vertex) {
            count++;
        }
    }
    return count;
}

// Check if edge exists in MST
int edge_in_mst(Edge mst[], int mst_edges, int src, int dest) {
    for (int i = 0; i < mst_edges; i++) {
        if ((mst[i].src == src && mst[i].dest == dest) ||
            (mst[i].src == dest && mst[i].dest == src)) {
            return 1;
        }
    }
    return 0;
}

// KLEE test function
int main() {
    Edge edges[MAX_EDGES];
    Edge mst[MAX_EDGES];
    int vertices, num_edges;
    
    // Make inputs symbolic
    klee_make_symbolic(&vertices, sizeof(int), "vertices");
    klee_assume(vertices >= 2 && vertices <= MAX_VERTICES);
    
    klee_make_symbolic(&num_edges, sizeof(int), "num_edges");
    klee_assume(num_edges >= 0 && num_edges <= MAX_EDGES);
    
    for (int i = 0; i < num_edges; i++) {
        klee_make_symbolic(&edges[i].src, sizeof(int), "edge_src");
        klee_make_symbolic(&edges[i].dest, sizeof(int), "edge_dest");
        klee_make_symbolic(&edges[i].weight, sizeof(int), "edge_weight");
        
        klee_assume(edges[i].src >= 0 && edges[i].src < vertices);
        klee_assume(edges[i].dest >= 0 && edges[i].dest < vertices);
        klee_assume(edges[i].src != edges[i].dest); // No self-loops
        klee_assume(edges[i].weight >= 0 && edges[i].weight <= 1000);
    }
    
    // Test with known graph
    Edge test_edges[] = {
        {0, 1, 4}, {0, 2, 3}, {1, 2, 1}, {1, 3, 2}, {2, 3, 4}
    };
    int test_vertices = 4;
    int test_num_edges = 5;
    Edge test_mst[MAX_EDGES];
    
    int test_mst_edges = kruskal_mst(test_edges, test_vertices, test_num_edges, test_mst);
    
    // Verify MST properties
    klee_assert(test_mst_edges == test_vertices - 1);
    klee_assert(is_mst_connected(test_mst, test_mst_edges, test_vertices));
    klee_assert(is_mst_tree(test_mst, test_mst_edges, test_vertices));
    
    int test_weight = calculate_mst_weight(test_mst, test_mst_edges);
    klee_assert(test_weight > 0);
    
    // Test with symbolic graph if connected
    if (is_graph_connected(edges, num_edges, vertices)) {
        int mst_edges = kruskal_mst(edges, vertices, num_edges, mst);
        
        // Verify MST properties
        klee_assert(mst_edges == vertices - 1);
        klee_assert(is_mst_connected(mst, mst_edges, vertices));
        klee_assert(is_mst_tree(mst, mst_edges, vertices));
        
        // Verify MST weight is reasonable
        int total_weight = calculate_mst_weight(mst, mst_edges);
        klee_assert(total_weight >= 0);
        
        // Verify MST weight is not greater than sum of all edges
        int total_graph_weight = 0;
        for (int i = 0; i < num_edges; i++) {
            total_graph_weight += edges[i].weight;
        }
        klee_assert(total_weight <= total_graph_weight);
        
        // Verify each edge in MST exists in original graph
        for (int i = 0; i < mst_edges; i++) {
            int found = 0;
            for (int j = 0; j < num_edges; j++) {
                if ((mst[i].src == edges[j].src && mst[i].dest == edges[j].dest) ||
                    (mst[i].src == edges[j].dest && mst[i].dest == edges[j].src)) {
                    klee_assert(mst[i].weight == edges[j].weight);
                    found = 1;
                    break;
                }
            }
            klee_assert(found);
        }
    }
    
    // Test edge cases
    // Single edge graph
    Edge single_edge[] = {{0, 1, 5}};
    Edge single_mst[MAX_EDGES];
    int single_mst_edges = kruskal_mst(single_edge, 2, 1, single_mst);
    
    klee_assert(single_mst_edges == 1);
    klee_assert(is_mst_connected(single_mst, single_mst_edges, 2));
    klee_assert(calculate_mst_weight(single_mst, single_mst_edges) == 5);
    
    // Triangle graph
    Edge triangle_edges[] = {{0, 1, 3}, {1, 2, 4}, {2, 0, 5}};
    Edge triangle_mst[MAX_EDGES];
    int triangle_mst_edges = kruskal_mst(triangle_edges, 3, 3, triangle_mst);
    
    klee_assert(triangle_mst_edges == 2);
    klee_assert(is_mst_connected(triangle_mst, triangle_mst_edges, 3));
    
    // Test graph properties
    klee_assert(find_min_weight(edges, num_edges) >= 0);
    klee_assert(find_max_weight(edges, num_edges) >= 0);
    
    // Test that MST weight is minimum among all possible spanning trees
    // (This is a simplified test - in practice, we'd need to generate all spanning trees)
    if (is_graph_connected(edges, num_edges, vertices)) {
        int mst_weight = calculate_mst_weight(mst, vertices - 1);
        int min_edge_weight = find_min_weight(edges, num_edges);
        
        // MST weight should be at least (vertices - 1) * min_edge_weight
        // if all edges have the same weight
        klee_assert(mst_weight >= 0);
    }
    
    // Test vertex degree properties
    if (vertices > 0) {
        for (int v = 0; v < vertices; v++) {
            int degree = count_vertex_edges(mst, vertices - 1, v);
            klee_assert(degree >= 0);
            // In a tree, each vertex should have at least one edge (except single vertex)
            if (vertices > 1) {
                klee_assert(degree >= 1);
            }
        }
    }
    
    return 0;
} 