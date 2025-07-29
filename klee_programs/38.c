#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

#define MAX_VERTICES 20
#define MAX_QUEUE_SIZE 100

typedef struct {
    int data[MAX_QUEUE_SIZE];
    int front, rear;
} Queue;

typedef struct {
    int vertices;
    int adjacency_matrix[MAX_VERTICES][MAX_VERTICES];
} Graph;

// Initialize queue
void init_queue(Queue* queue) {
    queue->front = queue->rear = -1;
}

// Check if queue is empty
int is_queue_empty(Queue* queue) {
    return queue->front == -1;
}

// Check if queue is full
int is_queue_full(Queue* queue) {
    return (queue->rear + 1) % MAX_QUEUE_SIZE == queue->front;
}

// Enqueue element
void enqueue(Queue* queue, int value) {
    if (is_queue_full(queue)) {
        return;
    }
    
    if (is_queue_empty(queue)) {
        queue->front = queue->rear = 0;
    } else {
        queue->rear = (queue->rear + 1) % MAX_QUEUE_SIZE;
    }
    
    queue->data[queue->rear] = value;
}

// Dequeue element
int dequeue(Queue* queue) {
    if (is_queue_empty(queue)) {
        return -1;
    }
    
    int value = queue->data[queue->front];
    
    if (queue->front == queue->rear) {
        queue->front = queue->rear = -1;
    } else {
        queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
    }
    
    return value;
}

// Initialize graph
void init_graph(Graph* graph, int vertices) {
    graph->vertices = vertices;
    for (int i = 0; i < vertices; i++) {
        for (int j = 0; j < vertices; j++) {
            graph->adjacency_matrix[i][j] = 0;
        }
    }
}

// Add edge to graph
void add_edge(Graph* graph, int src, int dest) {
    if (src >= 0 && src < graph->vertices && 
        dest >= 0 && dest < graph->vertices) {
        graph->adjacency_matrix[src][dest] = 1;
        graph->adjacency_matrix[dest][src] = 1; // Undirected graph
    }
}

// BFS traversal
void bfs(Graph* graph, int start_vertex, int visited[], int distance[]) {
    Queue queue;
    init_queue(&queue);
    
    // Initialize visited and distance arrays
    for (int i = 0; i < graph->vertices; i++) {
        visited[i] = 0;
        distance[i] = -1;
    }
    
    // Mark start vertex as visited and enqueue
    visited[start_vertex] = 1;
    distance[start_vertex] = 0;
    enqueue(&queue, start_vertex);
    
    while (!is_queue_empty(&queue)) {
        int current_vertex = dequeue(&queue);
        
        // Visit all adjacent vertices
        for (int i = 0; i < graph->vertices; i++) {
            if (graph->adjacency_matrix[current_vertex][i] == 1 && !visited[i]) {
                visited[i] = 1;
                distance[i] = distance[current_vertex] + 1;
                enqueue(&queue, i);
            }
        }
    }
}

// Check if graph is connected
int is_connected(Graph* graph) {
    int visited[MAX_VERTICES];
    int distance[MAX_VERTICES];
    
    // Start BFS from vertex 0
    bfs(graph, 0, visited, distance);
    
    // Check if all vertices are visited
    for (int i = 0; i < graph->vertices; i++) {
        if (!visited[i]) {
            return 0;
        }
    }
    
    return 1;
}

// Count connected components
int count_connected_components(Graph* graph) {
    int visited[MAX_VERTICES] = {0};
    int distance[MAX_VERTICES];
    int components = 0;
    
    for (int i = 0; i < graph->vertices; i++) {
        if (!visited[i]) {
            bfs(graph, i, visited, distance);
            components++;
        }
    }
    
    return components;
}

// Find shortest path between two vertices
int shortest_path(Graph* graph, int src, int dest) {
    int visited[MAX_VERTICES];
    int distance[MAX_VERTICES];
    
    bfs(graph, src, visited, distance);
    
    return distance[dest];
}

// Check if path exists between two vertices
int path_exists(Graph* graph, int src, int dest) {
    int visited[MAX_VERTICES];
    int distance[MAX_VERTICES];
    
    bfs(graph, src, visited, distance);
    
    return visited[dest];
}

// Count edges in graph
int count_edges(Graph* graph) {
    int count = 0;
    for (int i = 0; i < graph->vertices; i++) {
        for (int j = i + 1; j < graph->vertices; j++) {
            if (graph->adjacency_matrix[i][j] == 1) {
                count++;
            }
        }
    }
    return count;
}

// Check if vertex has any neighbors
int has_neighbors(Graph* graph, int vertex) {
    for (int i = 0; i < graph->vertices; i++) {
        if (graph->adjacency_matrix[vertex][i] == 1) {
            return 1;
        }
    }
    return 0;
}

// Count neighbors of a vertex
int count_neighbors(Graph* graph, int vertex) {
    int count = 0;
    for (int i = 0; i < graph->vertices; i++) {
        if (graph->adjacency_matrix[vertex][i] == 1) {
            count++;
        }
    }
    return count;
}

// Check if graph is a tree (connected with n-1 edges)
int is_tree(Graph* graph) {
    int edges = count_edges(graph);
    int components = count_connected_components(graph);
    
    return components == 1 && edges == graph->vertices - 1;
}

// KLEE test function
int main() {
    Graph graph;
    int vertices, edges;
    int edge_list[MAX_VERTICES * MAX_VERTICES][2];
    
    // Make inputs symbolic
    klee_make_symbolic(&vertices, sizeof(int), "vertices");
    klee_assume(vertices >= 1 && vertices <= MAX_VERTICES);
    
    klee_make_symbolic(&edges, sizeof(int), "edges");
    klee_assume(edges >= 0 && edges <= vertices * (vertices - 1) / 2);
    
    for (int i = 0; i < edges; i++) {
        klee_make_symbolic(&edge_list[i][0], sizeof(int), "edge_src");
        klee_make_symbolic(&edge_list[i][1], sizeof(int), "edge_dest");
        klee_assume(edge_list[i][0] >= 0 && edge_list[i][0] < vertices);
        klee_assume(edge_list[i][1] >= 0 && edge_list[i][1] < vertices);
        klee_assume(edge_list[i][0] != edge_list[i][1]); // No self-loops
    }
    
    // Initialize graph
    init_graph(&graph, vertices);
    
    // Add edges
    for (int i = 0; i < edges; i++) {
        add_edge(&graph, edge_list[i][0], edge_list[i][1]);
    }
    
    // Test BFS
    int visited[MAX_VERTICES];
    int distance[MAX_VERTICES];
    
    if (vertices > 0) {
        bfs(&graph, 0, visited, distance);
        
        // Verify BFS properties
        klee_assert(visited[0] == 1); // Start vertex should be visited
        klee_assert(distance[0] == 0); // Distance to start should be 0
        
        // All reachable vertices should have non-negative distance
        for (int i = 0; i < vertices; i++) {
            if (visited[i]) {
                klee_assert(distance[i] >= 0);
            }
        }
    }
    
    // Test with known graph
    Graph test_graph;
    init_graph(&test_graph, 4);
    add_edge(&test_graph, 0, 1);
    add_edge(&test_graph, 0, 2);
    add_edge(&test_graph, 1, 2);
    add_edge(&test_graph, 2, 3);
    
    int test_visited[MAX_VERTICES];
    int test_distance[MAX_VERTICES];
    bfs(&test_graph, 0, test_visited, test_distance);
    
    // Verify BFS results
    klee_assert(test_visited[0] == 1);
    klee_assert(test_visited[1] == 1);
    klee_assert(test_visited[2] == 1);
    klee_assert(test_visited[3] == 1);
    klee_assert(test_distance[0] == 0);
    klee_assert(test_distance[1] == 1);
    klee_assert(test_distance[2] == 1);
    klee_assert(test_distance[3] == 2);
    
    // Test connectivity
    klee_assert(is_connected(&test_graph));
    klee_assert(count_connected_components(&test_graph) == 1);
    
    // Test shortest path
    klee_assert(shortest_path(&test_graph, 0, 3) == 2);
    klee_assert(shortest_path(&test_graph, 0, 1) == 1);
    klee_assert(shortest_path(&test_graph, 0, 0) == 0);
    
    // Test path existence
    klee_assert(path_exists(&test_graph, 0, 3));
    klee_assert(path_exists(&test_graph, 1, 3));
    klee_assert(path_exists(&test_graph, 0, 0));
    
    // Test edge counting
    klee_assert(count_edges(&test_graph) == 4);
    
    // Test with symbolic graph
    if (vertices > 0) {
        int components = count_connected_components(&graph);
        klee_assert(components >= 1);
        klee_assert(components <= vertices);
        
        int total_edges = count_edges(&graph);
        klee_assert(total_edges >= 0);
        klee_assert(total_edges <= vertices * (vertices - 1) / 2);
        
        // Test BFS from each vertex
        for (int start = 0; start < vertices; start++) {
            int local_visited[MAX_VERTICES];
            int local_distance[MAX_VERTICES];
            
            bfs(&graph, start, local_visited, local_distance);
            
            klee_assert(local_visited[start] == 1);
            klee_assert(local_distance[start] == 0);
            
            // Check that distances are consistent
            for (int i = 0; i < vertices; i++) {
                if (local_visited[i]) {
                    klee_assert(local_distance[i] >= 0);
                }
            }
        }
    }
    
    // Test edge cases
    // Empty graph
    Graph empty_graph;
    init_graph(&empty_graph, 3);
    klee_assert(count_edges(&empty_graph) == 0);
    klee_assert(count_connected_components(&empty_graph) == 3);
    
    // Single vertex
    Graph single_graph;
    init_graph(&single_graph, 1);
    klee_assert(count_edges(&single_graph) == 0);
    klee_assert(count_connected_components(&single_graph) == 1);
    
    // Complete graph (all vertices connected)
    Graph complete_graph;
    init_graph(&complete_graph, 3);
    add_edge(&complete_graph, 0, 1);
    add_edge(&complete_graph, 0, 2);
    add_edge(&complete_graph, 1, 2);
    klee_assert(is_connected(&complete_graph));
    klee_assert(count_edges(&complete_graph) == 3);
    
    // Test tree property
    if (vertices > 1 && is_connected(&graph)) {
        int edge_count = count_edges(&graph);
        if (edge_count == vertices - 1) {
            klee_assert(is_tree(&graph));
        }
    }
    
    return 0;
} 