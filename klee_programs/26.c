#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

#define INITIAL_CAPACITY 4
#define MAX_CAPACITY 1000

typedef struct {
    int* data;
    int top;
    int capacity;
} DynamicStack;

// Create a new dynamic stack
DynamicStack* create_stack() {
    DynamicStack* stack = (DynamicStack*)malloc(sizeof(DynamicStack));
    stack->data = (int*)malloc(INITIAL_CAPACITY * sizeof(int));
    stack->top = -1;
    stack->capacity = INITIAL_CAPACITY;
    return stack;
}

// Check if stack is empty
int is_empty(DynamicStack* stack) {
    return stack->top == -1;
}

// Check if stack is full
int is_full(DynamicStack* stack) {
    return stack->top == stack->capacity - 1;
}

// Get stack size
int stack_size(DynamicStack* stack) {
    return stack->top + 1;
}

// Get stack capacity
int get_capacity(DynamicStack* stack) {
    return stack->capacity;
}

// Expand stack capacity
void expand_stack(DynamicStack* stack) {
    int new_capacity = stack->capacity * 2;
    if (new_capacity > MAX_CAPACITY) {
        new_capacity = MAX_CAPACITY;
    }
    
    int* new_data = (int*)malloc(new_capacity * sizeof(int));
    for (int i = 0; i <= stack->top; i++) {
        new_data[i] = stack->data[i];
    }
    
    free(stack->data);
    stack->data = new_data;
    stack->capacity = new_capacity;
}

// Shrink stack capacity
void shrink_stack(DynamicStack* stack) {
    if (stack->capacity <= INITIAL_CAPACITY) {
        return;
    }
    
    int new_capacity = stack->capacity / 2;
    if (new_capacity < INITIAL_CAPACITY) {
        new_capacity = INITIAL_CAPACITY;
    }
    
    // Only shrink if usage is less than 25% of capacity
    if (stack_size(stack) > new_capacity / 4) {
        return;
    }
    
    int* new_data = (int*)malloc(new_capacity * sizeof(int));
    for (int i = 0; i <= stack->top; i++) {
        new_data[i] = stack->data[i];
    }
    
    free(stack->data);
    stack->data = new_data;
    stack->capacity = new_capacity;
}

// Push element onto stack
int push(DynamicStack* stack, int value) {
    if (is_full(stack)) {
        expand_stack(stack);
    }
    
    stack->top++;
    stack->data[stack->top] = value;
    return stack->top;
}

// Pop element from stack
int pop(DynamicStack* stack) {
    if (is_empty(stack)) {
        return -1; // Error value
    }
    
    int value = stack->data[stack->top];
    stack->top--;
    
    // Shrink if necessary
    shrink_stack(stack);
    
    return value;
}

// Peek at top element
int peek(DynamicStack* stack) {
    if (is_empty(stack)) {
        return -1; // Error value
    }
    return stack->data[stack->top];
}

// Get element at specific index
int get_at_index(DynamicStack* stack, int index) {
    if (index < 0 || index > stack->top) {
        return -1; // Error value
    }
    return stack->data[index];
}

// Check if stack is properly ordered (LIFO property)
int is_properly_ordered(DynamicStack* stack) {
    for (int i = 0; i < stack->top; i++) {
        if (stack->data[i] > stack->data[i + 1]) {
            return 0; // Not properly ordered
        }
    }
    return 1;
}

// Clear stack
void clear_stack(DynamicStack* stack) {
    stack->top = -1;
    shrink_stack(stack);
}

// Destroy stack
void destroy_stack(DynamicStack* stack) {
    free(stack->data);
    free(stack);
}

// Copy stack
DynamicStack* copy_stack(DynamicStack* original) {
    DynamicStack* copy = create_stack();
    for (int i = 0; i <= original->top; i++) {
        push(copy, original->data[i]);
    }
    return copy;
}

// KLEE test function
int main() {
    DynamicStack* stack = create_stack();
    
    // Symbolic inputs
    int values[10];
    int num_operations;
    
    // Make inputs symbolic
    for (int i = 0; i < 10; i++) {
        klee_make_symbolic(&values[i], sizeof(int), "value");
        klee_assume(values[i] >= -100 && values[i] <= 100);
    }
    
    klee_make_symbolic(&num_operations, sizeof(int), "num_ops");
    klee_assume(num_operations >= 0 && num_operations <= 10);
    
    // Test initial state
    klee_assert(is_empty(stack));
    klee_assert(stack_size(stack) == 0);
    klee_assert(get_capacity(stack) == INITIAL_CAPACITY);
    
    // Test push operations
    for (int i = 0; i < num_operations; i++) {
        int old_size = stack_size(stack);
        int old_capacity = get_capacity(stack);
        
        int result = push(stack, values[i]);
        
        klee_assert(result == i);
        klee_assert(stack_size(stack) == old_size + 1);
        klee_assert(!is_empty(stack));
        klee_assert(peek(stack) == values[i]);
        klee_assert(get_at_index(stack, i) == values[i]);
        
        // Check capacity expansion
        if (old_size >= old_capacity) {
            klee_assert(get_capacity(stack) > old_capacity);
        }
    }
    
    // Test stack properties after pushes
    klee_assert(stack_size(stack) == num_operations);
    klee_assert(get_capacity(stack) >= stack_size(stack));
    
    // Test pop operations
    for (int i = num_operations - 1; i >= 0; i--) {
        int old_size = stack_size(stack);
        int old_capacity = get_capacity(stack);
        
        int popped = pop(stack);
        
        klee_assert(popped == values[i]);
        klee_assert(stack_size(stack) == old_size - 1);
        
        if (i > 0) {
            klee_assert(peek(stack) == values[i - 1]);
        } else {
            klee_assert(is_empty(stack));
        }
    }
    
    // Test empty stack operations
    klee_assert(is_empty(stack));
    klee_assert(stack_size(stack) == 0);
    klee_assert(pop(stack) == -1);
    klee_assert(peek(stack) == -1);
    
    // Test with specific values
    push(stack, 10);
    push(stack, 20);
    push(stack, 30);
    
    klee_assert(stack_size(stack) == 3);
    klee_assert(peek(stack) == 30);
    klee_assert(get_at_index(stack, 0) == 10);
    klee_assert(get_at_index(stack, 1) == 20);
    klee_assert(get_at_index(stack, 2) == 30);
    
    // Test pop order (LIFO)
    klee_assert(pop(stack) == 30);
    klee_assert(pop(stack) == 20);
    klee_assert(pop(stack) == 10);
    klee_assert(is_empty(stack));
    
    // Test capacity management
    for (int i = 0; i < 20; i++) {
        push(stack, i);
    }
    
    klee_assert(stack_size(stack) == 20);
    klee_assert(get_capacity(stack) >= 20);
    
    // Test clear operation
    clear_stack(stack);
    klee_assert(is_empty(stack));
    klee_assert(stack_size(stack) == 0);
    
    // Test copy operation
    push(stack, 100);
    push(stack, 200);
    push(stack, 300);
    
    DynamicStack* copy = copy_stack(stack);
    klee_assert(stack_size(copy) == stack_size(stack));
    klee_assert(peek(copy) == peek(stack));
    
    // Verify copy is independent
    pop(copy);
    klee_assert(stack_size(copy) != stack_size(stack));
    
    // Clean up
    destroy_stack(stack);
    destroy_stack(copy);
    
    // Test edge cases
    DynamicStack* edge_stack = create_stack();
    
    // Test with maximum values
    for (int i = 0; i < 100; i++) {
        push(edge_stack, 1000);
    }
    
    klee_assert(stack_size(edge_stack) == 100);
    klee_assert(get_capacity(edge_stack) >= 100);
    
    // Test with negative values
    clear_stack(edge_stack);
    push(edge_stack, -50);
    push(edge_stack, -100);
    
    klee_assert(peek(edge_stack) == -100);
    klee_assert(pop(edge_stack) == -100);
    klee_assert(pop(edge_stack) == -50);
    
    destroy_stack(edge_stack);
    
    return 0;
} 