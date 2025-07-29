#include <klee/klee.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 10  // Reduced for KLEE testing
#define INT_MIN 0

struct node
{
    int vertex;
    struct node *next;
};

struct Graph
{
    int numVertices;
    int *visited;
    struct node **adjLists;
};

struct Stack
{
    int arr[MAX_SIZE];
    int top;
};

struct node *createNode(int v)
{
    struct node *newNode = malloc(sizeof(struct node));
    newNode->vertex = v;
    newNode->next = NULL;
    return newNode;
}

struct Graph *createGraph(int vertices)
{
    struct Graph *graph = malloc(sizeof(struct Graph));
    graph->numVertices = vertices;
    graph->adjLists = malloc(vertices * sizeof(struct node *));
    graph->visited = malloc(vertices * sizeof(int));

    int i;
    for (i = 0; i < vertices; i++)
    {
        graph->adjLists[i] = NULL;
        graph->visited[i] = 0;
    }
    return graph;
}

void addEdge(struct Graph *graph, int src, int dest)
{
    struct node *newNode = createNode(dest);
    newNode->next = graph->adjLists[src];
    graph->adjLists[src] = newNode;
}

struct Stack *createStack()
{
    struct Stack *stack = malloc(sizeof(struct Stack));
    stack->top = -1;
    return stack;
}

void push(struct Stack *stack, int element)
{
    stack->arr[++stack->top] = element;
}

int pop(struct Stack *stack)
{
    if (stack->top == -1)
        return INT_MIN;
    else
        return stack->arr[stack->top--];
}

void topologicalSortHelper(int vertex, struct Graph *graph, struct Stack *stack)
{
    graph->visited[vertex] = 1;
    struct node *adjList = graph->adjLists[vertex];
    struct node *temp = adjList;
    
    while (temp != NULL)
    {
        int connectedVertex = temp->vertex;
        if (graph->visited[connectedVertex] == 0)
        {
            topologicalSortHelper(connectedVertex, graph, stack);
        }
        temp = temp->next;
    }
    push(stack, vertex);
}

void topologicalSort(struct Graph *graph)
{
    struct Stack *stack = createStack();
    int i = 0;
    for (i = 0; i < graph->numVertices; i++)
    {
        if (graph->visited[i] == 0)
        {
            topologicalSortHelper(i, graph, stack);
        }
    }
    
    // Verify that all vertices are visited
    for (i = 0; i < graph->numVertices; i++) {
        klee_assert(graph->visited[i] == 1);
    }
    
    // Verify stack contains all vertices
    klee_assert(stack->top == graph->numVertices - 1);
}

int main()
{
    int vertices = 5;
    int edges = 4;
    
    struct Graph *graph = createGraph(vertices);
    
    // Make edge inputs symbolic
    int src[4], dst[4];
    klee_make_symbolic(src, sizeof(src), "src");
    klee_make_symbolic(dst, sizeof(dst), "dst");
    
    // Constrain edge inputs to valid vertex indices
    for (int i = 0; i < edges; i++) {
        klee_assume(src[i] >= 0 && src[i] < vertices);
        klee_assume(dst[i] >= 0 && dst[i] < vertices);
        klee_assume(src[i] != dst[i]); // No self-loops
    }
    
    // Add edges
    for (int i = 0; i < edges; i++) {
        addEdge(graph, src[i], dst[i]);
    }
    
    // Perform topological sort
    topologicalSort(graph);
    
    // Verify that the graph is acyclic (no cycles)
    // This is a simplified check - in practice, you'd need cycle detection
    for (int i = 0; i < vertices; i++) {
        klee_assert(graph->visited[i] == 1);
    }
    
    return 0;
} 