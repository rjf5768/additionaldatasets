#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#define MAX_VERTICES 20
#define MAX_EDGES 50

typedef struct Node {
    int vertex;
    struct Node* next;
} Node;

typedef struct {
    int numVertices;
    int numEdges;
    Node* adjLists[MAX_VERTICES];
    int visited[MAX_VERTICES];
    int finishTime[MAX_VERTICES];
    int component[MAX_VERTICES];
} Graph;

typedef struct {
    int data[MAX_VERTICES];
    int top;
} Stack;

// Create a new node
Node* create_node(int vertex) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->vertex = vertex;
    newNode->next = NULL;
    return newNode;
}

// Initialize graph
void init_graph(Graph* graph, int vertices) {
    graph->numVertices = vertices;
    graph->numEdges = 0;
    
    for (int i = 0; i < vertices; i++) {
        graph->adjLists[i] = NULL;
        graph->visited[i] = 0;
        graph->finishTime[i] = 0;
        graph->component[i] = -1;
    }
}

// Add edge to graph
void add_edge(Graph* graph, int src, int dest) {
    if (src < 0 || src >= graph->numVertices || 
        dest < 0 || dest >= graph->numVertices) {
        return; // Invalid vertices
    }
    
    Node* newNode = create_node(dest);
    newNode->next = graph->adjLists[src];
    graph->adjLists[src] = newNode;
    graph->numEdges++;
}

// Initialize stack
void init_stack(Stack* stack) {
    stack->top = -1;
}

// Check if stack is empty
int is_stack_empty(Stack* stack) {
    return stack->top == -1;
}

// Check if stack is full
int is_stack_full(Stack* stack) {
    return stack->top == MAX_VERTICES - 1;
}

// Push element onto stack
void push(Stack* stack, int value) {
    if (!is_stack_full(stack)) {
        stack->data[++stack->top] = value;
    }
}

// Pop element from stack
int pop(Stack* stack) {
    if (!is_stack_empty(stack)) {
        return stack->data[stack->top--];
    }
    return -1;
}

// First DFS to fill finish times
void dfs_fill_order(Graph* graph, int vertex, Stack* stack, int* time) {
    graph->visited[vertex] = 1;
    
    Node* current = graph->adjLists[vertex];
    while (current != NULL) {
        int neighbor = current->vertex;
        if (!graph->visited[neighbor]) {
            dfs_fill_order(graph, neighbor, stack, time);
        }
        current = current->next;
    }
    
    graph->finishTime[vertex] = (*time)++;
    push(stack, vertex);
}

// Second DFS to find SCCs
void dfs_scc(Graph* graph, int vertex, int component_id) {
    graph->visited[vertex] = 1;
    graph->component[vertex] = component_id;
    
    Node* current = graph->adjLists[vertex];
    while (current != NULL) {
        int neighbor = current->vertex;
        if (!graph->visited[neighbor]) {
            dfs_scc(graph, neighbor, component_id);
        }
        current = current->next;
    }
}

// Transpose graph (reverse all edges)
Graph* transpose_graph(Graph* original) {
    Graph* transposed = (Graph*)malloc(sizeof(Graph));
    init_graph(transposed, original->numVertices);
    
    for (int i = 0; i < original->numVertices; i++) {
        Node* current = original->adjLists[i];
        while (current != NULL) {
            add_edge(transposed, current->vertex, i);
            current = current->next;
        }
    }
    
    return transposed;
}

// Find strongly connected components using Kosaraju's algorithm
int find_sccs(Graph* graph) {
    Stack stack;
    init_stack(&stack);
    
    // Reset visited array
    for (int i = 0; i < graph->numVertices; i++) {
        graph->visited[i] = 0;
        graph->finishTime[i] = 0;
        graph->component[i] = -1;
    }
    
    // First DFS to fill finish times
    int time = 0;
    for (int i = 0; i < graph->numVertices; i++) {
        if (!graph->visited[i]) {
            dfs_fill_order(graph, i, &stack, &time);
        }
    }
    
    // Transpose the graph
    Graph* transposed = transpose_graph(graph);
    
    // Reset visited array for second DFS
    for (int i = 0; i < transposed->numVertices; i++) {
        transposed->visited[i] = 0;
    }
    
    // Second DFS to find SCCs
    int component_id = 0;
    while (!is_stack_empty(&stack)) {
        int vertex = pop(&stack);
        if (!transposed->visited[vertex]) {
            dfs_scc(transposed, vertex, component_id);
            component_id++;
        }
    }
    
    // Copy component information back to original graph
    for (int i = 0; i < graph->numVertices; i++) {
        graph->component[i] = transposed->component[i];
    }
    
    // Clean up
    free(transposed);
    
    return component_id;
}

// Check if two vertices are in the same component
int same_component(Graph* graph, int v1, int v2) {
    if (v1 < 0 || v1 >= graph->numVertices || 
        v2 < 0 || v2 >= graph->numVertices) {
        return 0;
    }
    return graph->component[v1] == graph->component[v2];
}

// Count vertices in a component
int count_vertices_in_component(Graph* graph, int component_id) {
    int count = 0;
    for (int i = 0; i < graph->numVertices; i++) {
        if (graph->component[i] == component_id) {
            count++;
        }
    }
    return count;
}

// Check if graph is strongly connected
int is_strongly_connected(Graph* graph) {
    if (graph->numVertices == 0) return 1;
    
    int num_components = find_sccs(graph);
    return num_components == 1;
}

// Check if vertex is reachable from another vertex
int is_reachable(Graph* graph, int src, int dest) {
    if (src < 0 || src >= graph->numVertices || 
        dest < 0 || dest >= graph->numVertices) {
        return 0;
    }
    
    // Reset visited array
    for (int i = 0; i < graph->numVertices; i++) {
        graph->visited[i] = 0;
    }
    
    // Simple DFS to check reachability
    Stack stack;
    init_stack(&stack);
    push(&stack, src);
    graph->visited[src] = 1;
    
    while (!is_stack_empty(&stack)) {
        int current = pop(&stack);
        if (current == dest) {
            return 1;
        }
        
        Node* neighbor = graph->adjLists[current];
        while (neighbor != NULL) {
            if (!graph->visited[neighbor->vertex]) {
                graph->visited[neighbor->vertex] = 1;
                push(&stack, neighbor->vertex);
            }
            neighbor = neighbor->next;
        }
    }
    
    return 0;
}

// Count total edges in graph
int count_edges(Graph* graph) {
    int count = 0;
    for (int i = 0; i < graph->numVertices; i++) {
        Node* current = graph->adjLists[i];
        while (current != NULL) {
            count++;
            current = current->next;
        }
    }
    return count;
}

// KLEE test function
int main() {
    int num_vertices, num_edges;
    int edges[MAX_EDGES][2]; // [src, dest]
    
    // Make inputs symbolic
    klee_make_symbolic(&num_vertices, sizeof(int), "num_vertices");
    klee_assume(num_vertices >= 1 && num_vertices <= MAX_VERTICES);
    
    klee_make_symbolic(&num_edges, sizeof(int), "num_edges");
    klee_assume(num_edges >= 0 && num_edges <= MAX_EDGES);
    
    for (int i = 0; i < num_edges; i++) {
        klee_make_symbolic(&edges[i][0], sizeof(int), "edge_src");
        klee_make_symbolic(&edges[i][1], sizeof(int), "edge_dest");
        klee_assume(edges[i][0] >= 0 && edges[i][0] < num_vertices);
        klee_assume(edges[i][1] >= 0 && edges[i][1] < num_vertices);
    }
    
    // Create and initialize graph
    Graph graph;
    init_graph(&graph, num_vertices);
    
    // Add edges
    for (int i = 0; i < num_edges; i++) {
        add_edge(&graph, edges[i][0], edges[i][1]);
    }
    
    // Test graph properties
    klee_assert(graph.numVertices == num_vertices);
    klee_assert(graph.numEdges == num_edges);
    klee_assert(count_edges(&graph) == num_edges);
    
    // Test stack operations
    Stack test_stack;
    init_stack(&test_stack);
    
    klee_assert(is_stack_empty(&test_stack));
    klee_assert(!is_stack_full(&test_stack));
    
    push(&test_stack, 5);
    klee_assert(!is_stack_empty(&test_stack));
    klee_assert(pop(&test_stack) == 5);
    klee_assert(is_stack_empty(&test_stack));
    
    // Test with simple graph
    Graph simple_graph;
    init_graph(&simple_graph, 3);
    add_edge(&simple_graph, 0, 1);
    add_edge(&simple_graph, 1, 2);
    add_edge(&simple_graph, 2, 0);
    
    int num_components = find_sccs(&simple_graph);
    klee_assert(num_components == 1); // Should be one SCC
    klee_assert(is_strongly_connected(&simple_graph));
    
    // Test reachability
    klee_assert(is_reachable(&simple_graph, 0, 1));
    klee_assert(is_reachable(&simple_graph, 0, 2));
    klee_assert(is_reachable(&simple_graph, 1, 2));
    klee_assert(is_reachable(&simple_graph, 2, 0));
    
    // Test with disconnected graph
    Graph disconnected_graph;
    init_graph(&disconnected_graph, 4);
    add_edge(&disconnected_graph, 0, 1);
    add_edge(&disconnected_graph, 1, 0);
    add_edge(&disconnected_graph, 2, 3);
    add_edge(&disconnected_graph, 3, 2);
    
    num_components = find_sccs(&disconnected_graph);
    klee_assert(num_components == 2); // Two separate SCCs
    
    klee_assert(same_component(&disconnected_graph, 0, 1));
    klee_assert(same_component(&disconnected_graph, 2, 3));
    klee_assert(!same_component(&disconnected_graph, 0, 2));
    
    // Test component counting
    klee_assert(count_vertices_in_component(&disconnected_graph, 0) == 2);
    klee_assert(count_vertices_in_component(&disconnected_graph, 1) == 2);
    
    // Test with symbolic graph
    if (num_vertices > 0) {
        num_components = find_sccs(&graph);
        klee_assert(num_components >= 1);
        klee_assert(num_components <= num_vertices);
        
        // Check that all vertices are assigned to components
        for (int i = 0; i < num_vertices; i++) {
            klee_assert(graph.component[i] >= 0);
            klee_assert(graph.component[i] < num_components);
        }
        
        // Check component properties
        for (int comp = 0; comp < num_components; comp++) {
            int count = count_vertices_in_component(&graph, comp);
            klee_assert(count >= 1);
            klee_assert(count <= num_vertices);
        }
    }
    
    // Test edge cases
    // Empty graph
    Graph empty_graph;
    init_graph(&empty_graph, 0);
    num_components = find_sccs(&empty_graph);
    klee_assert(num_components == 0);
    
    // Single vertex graph
    Graph single_graph;
    init_graph(&single_graph, 1);
    num_components = find_sccs(&single_graph);
    klee_assert(num_components == 1);
    klee_assert(single_graph.component[0] == 0);
    
    // Self-loop graph
    Graph self_loop_graph;
    init_graph(&self_loop_graph, 2);
    add_edge(&self_loop_graph, 0, 0);
    add_edge(&self_loop_graph, 1, 1);
    num_components = find_sccs(&self_loop_graph);
    klee_assert(num_components == 2);
    
    return 0;
} 