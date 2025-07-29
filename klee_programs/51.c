#include <klee/klee.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
    int len;
    int current;
    int* contents;
} Vector;

void init(Vector* vec, int val) {
    vec->contents = (int*)malloc(sizeof(int));
    vec->contents[0] = val;
    vec->current = 0;
    vec->len = 1;
}

void delete(Vector* vec) {
    free(vec->contents);    
}

void clear(Vector* vec) {
    delete(vec);
    init(vec, 0);
}

int len(Vector* vec) {
    return vec->len;    
}

void push(Vector* vec, int val) {
    vec->contents = realloc(vec->contents, (sizeof(int) * (vec->len + 1)));
    vec->contents[vec->len] = val;
    vec->len++;
}

int get(Vector* vec, int index) {
    if(index < vec->len) {
        return vec->contents[index];
    }
    return -1;
}

void set(Vector* vec, int index, int val) {
    if(index < vec->len) {
        vec->contents[index] = val;
    }
}

int next(Vector* vec) {
    if(vec->current == vec->len) {
        vec->current = 0;
    }
    int current_val = vec->contents[vec->current];
    vec->current++;
    return current_val;
}

void* begin(Vector* vec) {
    return (void*)vec->contents;
}

int main() {
    Vector vec;
    int value, index;
    
    // Make inputs symbolic
    klee_make_symbolic(&value, sizeof(value), "value");
    klee_make_symbolic(&index, sizeof(index), "index");
    
    // Constrain inputs to reasonable values
    klee_assume(value >= 0 && value <= 100);
    klee_assume(index >= 0 && index <= 10);
    
    // Initialize vector
    init(&vec, 10);
    
    // Verify initial state
    klee_assert(len(&vec) == 1);
    klee_assert(get(&vec, 0) == 10);
    
    // Push symbolic value
    push(&vec, value);
    
    // Verify vector grew
    klee_assert(len(&vec) == 2);
    klee_assert(get(&vec, 1) == value);
    
    // Test set operation
    set(&vec, index, value);
    
    // Verify set operation (if index is valid)
    if (index < len(&vec)) {
        klee_assert(get(&vec, index) == value);
    }
    
    // Test next operation
    int next_val = next(&vec);
    klee_assert(next_val >= 0);
    
    // Verify vector properties
    klee_assert(len(&vec) > 0);
    klee_assert(vec.contents != NULL);
    
    // Test boundary conditions
    klee_assert(get(&vec, -1) == -1);  // Invalid index
    klee_assert(get(&vec, len(&vec)) == -1);  // Out of bounds
    
    delete(&vec);
    
    return 0;
} 