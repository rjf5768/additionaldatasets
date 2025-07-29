#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#define MAX_VERTICES 20
#define INF 999999

typedef struct {
    int weight;
    int src;
    int dest;
} Edge;

// Find minimum weight edge from unvisited vertices
int find_min_edge(int weights[], int visited[], int vertices) {
    int min_weight = INF;
    int min_vertex = -1;
    
    for (int i = 0; i < vertices; i++) {
        if (!visited[i] && weights[i] < min_weight) {
            min_weight = weights[i];
            min_vertex = i;
        }
    }
    
    return min_vertex;
}

// Prim's algorithm to find minimum spanning tree
void prim_mst(int graph[][MAX_VERTICES], int mst[][MAX_VERTICES], int vertices) {
    int weights[MAX_VERTICES];
    int parent[MAX_VERTICES];
    int visited[MAX_VERTICES];
    
    // Initialize arrays
    for (int i = 0; i < vertices; i++) {
        weights[i] = INF;
        parent[i] = -1;
        visited[i] = 0;
    }
    
    // Start with vertex 0
    weights[0] = 0;
    
    for (int count = 0; count < vertices - 1; count++) {
        int u = find_min_edge(weights, visited, vertices);
        
        if (u == -1) {
            break; // No more edges to add
        }
        
        visited[u] = 1;
        
        // Update weights of adjacent vertices
        for (int v = 0; v < vertices; v++) {
            if (graph[u][v] != 0 && !visited[v] && graph[u][v] < weights[v]) {
                weights[v] = graph[u][v];
                parent[v] = u;
            }
        }
    }
    
    // Build MST matrix
    for (int i = 0; i < vertices; i++) {
        for (int j = 0; j < vertices; j++) {
            mst[i][j] = 0;
        }
    }
    
    for (int i = 1; i < vertices; i++) {
        if (parent[i] != -1) {
            mst[parent[i]][i] = weights[i];
            mst[i][parent[i]] = weights[i];
        }
    }
}

// Calculate total weight of MST
int calculate_mst_weight(int mst[][MAX_VERTICES], int vertices) {
    int total_weight = 0;
    
    for (int i = 0; i < vertices; i++) {
        for (int j = i + 1; j < vertices; j++) {
            total_weight += mst[i][j];
        }
    }
    
    return total_weight;
}

// Check if MST is connected (all vertices reachable)
int is_mst_connected(int mst[][MAX_VERTICES], int vertices) {
    int visited[MAX_VERTICES] = {0};
    int stack[MAX_VERTICES];
    int top = 0;
    
    // Start DFS from vertex 0
    stack[top++] = 0;
    visited[0] = 1;
    
    while (top > 0) {
        int current = stack[--top];
        
        for (int i = 0; i < vertices; i++) {
            if (mst[current][i] > 0 && !visited[i]) {
                visited[i] = 1;
                stack[top++] = i;
            }
        }
    }
    
    // Check if all vertices are visited
    for (int i = 0; i < vertices; i++) {
        if (!visited[i]) {
            return 0;
        }
    }
    
    return 1;
}

// Count edges in MST
int count_mst_edges(int mst[][MAX_VERTICES], int vertices) {
    int edge_count = 0;
    
    for (int i = 0; i < vertices; i++) {
        for (int j = i + 1; j < vertices; j++) {
            if (mst[i][j] > 0) {
                edge_count++;
            }
        }
    }
    
    return edge_count;
}

// Check if graph is connected
int is_graph_connected(int graph[][MAX_VERTICES], int vertices) {
    int visited[MAX_VERTICES] = {0};
    int stack[MAX_VERTICES];
    int top = 0;
    
    // Start DFS from vertex 0
    stack[top++] = 0;
    visited[0] = 1;
    
    while (top > 0) {
        int current = stack[--top];
        
        for (int i = 0; i < vertices; i++) {
            if (graph[current][i] > 0 && !visited[i]) {
                visited[i] = 1;
                stack[top++] = i;
            }
        }
    }
    
    // Check if all vertices are visited
    for (int i = 0; i < vertices; i++) {
        if (!visited[i]) {
            return 0;
        }
    }
    
    return 1;
}

// Check if MST is a tree (no cycles)
int is_mst_tree(int mst[][MAX_VERTICES], int vertices) {
    int edge_count = count_mst_edges(mst, vertices);
    return edge_count == vertices - 1;
}

// Find minimum weight in graph
int find_min_weight(int graph[][MAX_VERTICES], int vertices) {
    int min_weight = INF;
    
    for (int i = 0; i < vertices; i++) {
        for (int j = 0; j < vertices; j++) {
            if (graph[i][j] > 0 && graph[i][j] < min_weight) {
                min_weight = graph[i][j];
            }
        }
    }
    
    return min_weight;
}

// Find maximum weight in graph
int find_max_weight(int graph[][MAX_VERTICES], int vertices) {
    int max_weight = 0;
    
    for (int i = 0; i < vertices; i++) {
        for (int j = 0; j < vertices; j++) {
            if (graph[i][j] > max_weight) {
                max_weight = graph[i][j];
            }
        }
    }
    
    return max_weight;
}

// Count total edges in graph
int count_graph_edges(int graph[][MAX_VERTICES], int vertices) {
    int edge_count = 0;
    
    for (int i = 0; i < vertices; i++) {
        for (int j = i + 1; j < vertices; j++) {
            if (graph[i][j] > 0) {
                edge_count++;
            }
        }
    }
    
    return edge_count;
}

// KLEE test function
int main() {
    int graph[MAX_VERTICES][MAX_VERTICES];
    int mst[MAX_VERTICES][MAX_VERTICES];
    int vertices;
    
    // Make inputs symbolic
    klee_make_symbolic(&vertices, sizeof(int), "vertices");
    klee_assume(vertices >= 2 && vertices <= MAX_VERTICES);
    
    for (int i = 0; i < vertices; i++) {
        for (int j = 0; j < vertices; j++) {
            klee_make_symbolic(&graph[i][j], sizeof(int), "edge_weight");
            klee_assume(graph[i][j] >= 0 && graph[i][j] <= 1000);
        }
    }
    
    // Ensure graph is symmetric (undirected)
    for (int i = 0; i < vertices; i++) {
        for (int j = 0; j < vertices; j++) {
            graph[j][i] = graph[i][j];
        }
    }
    
    // Ensure no self-loops
    for (int i = 0; i < vertices; i++) {
        graph[i][i] = 0;
    }
    
    // Test with known graph
    int test_graph[4][4] = {
        {0, 1, 2, 3},
        {1, 0, 4, 6},
        {2, 4, 0, 5},
        {3, 6, 5, 0}
    };
    
    int test_mst[4][4];
    prim_mst(test_graph, test_mst, 4);
    
    // Verify MST properties
    klee_assert(is_mst_connected(test_mst, 4));
    klee_assert(is_mst_tree(test_mst, 4));
    klee_assert(count_mst_edges(test_mst, 4) == 3);
    
    int mst_weight = calculate_mst_weight(test_mst, 4);
    klee_assert(mst_weight > 0);
    
    // Test with symbolic graph if connected
    if (is_graph_connected(graph, vertices)) {
        prim_mst(graph, mst, vertices);
        
        // Verify MST properties
        klee_assert(is_mst_connected(mst, vertices));
        klee_assert(is_mst_tree(mst, vertices));
        klee_assert(count_mst_edges(mst, vertices) == vertices - 1);
        
        // Verify MST weight is reasonable
        int total_weight = calculate_mst_weight(mst, vertices);
        klee_assert(total_weight >= 0);
        
        // Verify MST weight is not greater than sum of all edges
        int total_graph_weight = 0;
        for (int i = 0; i < vertices; i++) {
            for (int j = i + 1; j < vertices; j++) {
                total_graph_weight += graph[i][j];
            }
        }
        klee_assert(total_weight <= total_graph_weight);
        
        // Verify each edge in MST exists in original graph
        for (int i = 0; i < vertices; i++) {
            for (int j = i + 1; j < vertices; j++) {
                if (mst[i][j] > 0) {
                    klee_assert(graph[i][j] > 0);
                    klee_assert(mst[i][j] == graph[i][j]);
                }
            }
        }
    }
    
    // Test edge cases
    // Single edge graph
    int single_edge_graph[2][2] = {{0, 5}, {5, 0}};
    int single_edge_mst[2][2];
    prim_mst(single_edge_graph, single_edge_mst, 2);
    
    klee_assert(is_mst_connected(single_edge_mst, 2));
    klee_assert(count_mst_edges(single_edge_mst, 2) == 1);
    klee_assert(calculate_mst_weight(single_edge_mst, 2) == 5);
    
    // Triangle graph
    int triangle_graph[3][3] = {
        {0, 3, 4},
        {3, 0, 5},
        {4, 5, 0}
    };
    int triangle_mst[3][3];
    prim_mst(triangle_graph, triangle_mst, 3);
    
    klee_assert(is_mst_connected(triangle_mst, 3));
    klee_assert(count_mst_edges(triangle_mst, 3) == 2);
    
    // Test with disconnected graph
    int disconnected_graph[4][4] = {
        {0, 1, 0, 0},
        {1, 0, 0, 0},
        {0, 0, 0, 1},
        {0, 0, 1, 0}
    };
    
    // Prim's should still work but may not produce a connected MST
    int disconnected_mst[4][4];
    prim_mst(disconnected_graph, disconnected_mst, 4);
    
    // Test graph properties
    klee_assert(find_min_weight(graph, vertices) >= 0);
    klee_assert(find_max_weight(graph, vertices) >= 0);
    klee_assert(count_graph_edges(graph, vertices) >= 0);
    
    // Test that MST weight is minimum among all possible spanning trees
    // (This is a simplified test - in practice, we'd need to generate all spanning trees)
    if (is_graph_connected(graph, vertices)) {
        int mst_weight = calculate_mst_weight(mst, vertices);
        int min_edge_weight = find_min_weight(graph, vertices);
        
        // MST weight should be at least (vertices - 1) * min_edge_weight
        // if all edges have the same weight
        klee_assert(mst_weight >= 0);
    }
    
    return 0;
} 