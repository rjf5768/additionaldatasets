#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

#define MAX_SIZE 20

typedef struct PriorityQueue {
    int data[MAX_SIZE];
    int priorities[MAX_SIZE];
    int size;
} PriorityQueue;

// Initialize priority queue
void init_queue(PriorityQueue* pq) {
    pq->size = 0;
}

// Check if queue is empty
int is_empty(PriorityQueue* pq) {
    return pq->size == 0;
}

// Check if queue is full
int is_full(PriorityQueue* pq) {
    return pq->size >= MAX_SIZE;
}

// Insert element with priority
int enqueue(PriorityQueue* pq, int data, int priority) {
    if (is_full(pq)) {
        return 0; // Queue full
    }
    
    // Find position to insert (ascending priority)
    int pos = pq->size;
    for (int i = 0; i < pq->size; i++) {
        if (priority < pq->priorities[i]) {
            pos = i;
            break;
        }
    }
    
    // Shift elements to make space
    for (int i = pq->size; i > pos; i--) {
        pq->data[i] = pq->data[i-1];
        pq->priorities[i] = pq->priorities[i-1];
    }
    
    // Insert element
    pq->data[pos] = data;
    pq->priorities[pos] = priority;
    pq->size++;
    
    return 1;
}

// Remove and return highest priority element
int dequeue(PriorityQueue* pq) {
    if (is_empty(pq)) {
        return -1; // Queue empty
    }
    
    int data = pq->data[0];
    
    // Shift elements
    for (int i = 0; i < pq->size - 1; i++) {
        pq->data[i] = pq->data[i+1];
        pq->priorities[i] = pq->priorities[i+1];
    }
    
    pq->size--;
    return data;
}

// Peek at highest priority element
int peek(PriorityQueue* pq) {
    if (is_empty(pq)) {
        return -1;
    }
    return pq->data[0];
}

// Get highest priority value
int get_highest_priority(PriorityQueue* pq) {
    if (is_empty(pq)) {
        return -1;
    }
    return pq->priorities[0];
}

// Check if queue is properly ordered
int is_properly_ordered(PriorityQueue* pq) {
    for (int i = 1; i < pq->size; i++) {
        if (pq->priorities[i] < pq->priorities[i-1]) {
            return 0;
        }
    }
    return 1;
}

// Search for element
int search(PriorityQueue* pq, int data) {
    for (int i = 0; i < pq->size; i++) {
        if (pq->data[i] == data) {
            return i;
        }
    }
    return -1;
}

// Get element at index
int get_at_index(PriorityQueue* pq, int index) {
    if (index < 0 || index >= pq->size) {
        return -1;
    }
    return pq->data[index];
}

// Get priority at index
int get_priority_at_index(PriorityQueue* pq, int index) {
    if (index < 0 || index >= pq->size) {
        return -1;
    }
    return pq->priorities[index];
}

// Update priority of element
int update_priority(PriorityQueue* pq, int data, int new_priority) {
    int index = search(pq, data);
    if (index == -1) {
        return 0; // Element not found
    }
    
    // Remove element
    for (int i = index; i < pq->size - 1; i++) {
        pq->data[i] = pq->data[i+1];
        pq->priorities[i] = pq->priorities[i+1];
    }
    pq->size--;
    
    // Re-insert with new priority
    return enqueue(pq, data, new_priority);
}

// KLEE test function
int main() {
    PriorityQueue pq;
    init_queue(&pq);
    
    // Symbolic inputs
    int data1, priority1, data2, priority2, data3, priority3, data4, priority4;
    int search_data, update_data, new_priority, index;
    
    klee_make_symbolic(&data1, sizeof(data1), "data1");
    klee_make_symbolic(&priority1, sizeof(priority1), "priority1");
    klee_make_symbolic(&data2, sizeof(data2), "data2");
    klee_make_symbolic(&priority2, sizeof(priority2), "priority2");
    klee_make_symbolic(&data3, sizeof(data3), "data3");
    klee_make_symbolic(&priority3, sizeof(priority3), "priority3");
    klee_make_symbolic(&data4, sizeof(data4), "data4");
    klee_make_symbolic(&priority4, sizeof(priority4), "priority4");
    klee_make_symbolic(&search_data, sizeof(search_data), "search_data");
    klee_make_symbolic(&update_data, sizeof(update_data), "update_data");
    klee_make_symbolic(&new_priority, sizeof(new_priority), "new_priority");
    klee_make_symbolic(&index, sizeof(index), "index");
    
    // Constrain inputs
    klee_assume(data1 >= 0 && data1 <= 100);
    klee_assume(priority1 >= 0 && priority1 <= 10);
    klee_assume(data2 >= 0 && data2 <= 100);
    klee_assume(priority2 >= 0 && priority2 <= 10);
    klee_assume(data3 >= 0 && data3 <= 100);
    klee_assume(priority3 >= 0 && priority3 <= 10);
    klee_assume(data4 >= 0 && data4 <= 100);
    klee_assume(priority4 >= 0 && priority4 <= 10);
    klee_assume(search_data >= 0 && search_data <= 100);
    klee_assume(update_data >= 0 && update_data <= 100);
    klee_assume(new_priority >= 0 && new_priority <= 10);
    klee_assume(index >= 0 && index <= 10);
    
    // Test initial state
    klee_assert(is_empty(&pq));
    klee_assert(!is_full(&pq));
    klee_assert(pq.size == 0);
    
    // Insert elements
    int result1 = enqueue(&pq, data1, priority1);
    klee_assert(result1 == 1);
    klee_assert(pq.size == 1);
    klee_assert(!is_empty(&pq));
    klee_assert(is_properly_ordered(&pq));
    
    int result2 = enqueue(&pq, data2, priority2);
    klee_assert(result2 == 1);
    klee_assert(pq.size == 2);
    klee_assert(is_properly_ordered(&pq));
    
    int result3 = enqueue(&pq, data3, priority3);
    klee_assert(result3 == 1);
    klee_assert(pq.size == 3);
    klee_assert(is_properly_ordered(&pq));
    
    int result4 = enqueue(&pq, data4, priority4);
    klee_assert(result4 == 1);
    klee_assert(pq.size == 4);
    klee_assert(is_properly_ordered(&pq));
    
    // Test peek
    int peek_data = peek(&pq);
    klee_assert(peek_data >= 0);
    klee_assert(peek_data == pq.data[0]);
    
    // Test highest priority
    int highest_priority = get_highest_priority(&pq);
    klee_assert(highest_priority >= 0);
    klee_assert(highest_priority == pq.priorities[0]);
    
    // Test search
    int search_result = search(&pq, search_data);
    if (search_data == data1 || search_data == data2 || 
        search_data == data3 || search_data == data4) {
        klee_assert(search_result >= 0 && search_result < pq.size);
        klee_assert(pq.data[search_result] == search_data);
    } else {
        klee_assert(search_result == -1);
    }
    
    // Test get at index
    int data_at_index = get_at_index(&pq, index);
    if (index < pq.size) {
        klee_assert(data_at_index >= 0);
        klee_assert(data_at_index == pq.data[index]);
    } else {
        klee_assert(data_at_index == -1);
    }
    
    // Test priority ordering
    for (int i = 1; i < pq.size; i++) {
        klee_assert(pq.priorities[i] >= pq.priorities[i-1]);
    }
    
    // Test dequeue
    int dequeued = dequeue(&pq);
    klee_assert(dequeued >= 0);
    klee_assert(pq.size == 3);
    klee_assert(is_properly_ordered(&pq));
    
    // Test update priority
    if (update_data == data1 || update_data == data2 || 
        update_data == data3 || update_data == data4) {
        int update_result = update_priority(&pq, update_data, new_priority);
        if (update_result) {
            klee_assert(is_properly_ordered(&pq));
        }
    }
    
    // Test queue properties
    klee_assert(pq.size >= 0 && pq.size <= MAX_SIZE);
    
    // Test that priorities are non-negative
    for (int i = 0; i < pq.size; i++) {
        klee_assert(pq.priorities[i] >= 0);
    }
    
    // Test that data values are valid
    for (int i = 0; i < pq.size; i++) {
        klee_assert(pq.data[i] >= 0);
    }
    
    // Test multiple dequeues
    while (!is_empty(&pq)) {
        int data = dequeue(&pq);
        klee_assert(data >= 0);
    }
    
    klee_assert(is_empty(&pq));
    klee_assert(pq.size == 0);
    
    // Test edge cases
    PriorityQueue pq2;
    init_queue(&pq2);
    
    // Test dequeue on empty queue
    int empty_dequeue = dequeue(&pq2);
    klee_assert(empty_dequeue == -1);
    
    // Test peek on empty queue
    int empty_peek = peek(&pq2);
    klee_assert(empty_peek == -1);
    
    // Test get_highest_priority on empty queue
    int empty_priority = get_highest_priority(&pq2);
    klee_assert(empty_priority == -1);
    
    // Test with single element
    enqueue(&pq2, 50, 5);
    klee_assert(pq2.size == 1);
    klee_assert(pq2.data[0] == 50);
    klee_assert(pq2.priorities[0] == 5);
    
    // Test priority ordering with equal priorities
    PriorityQueue pq3;
    init_queue(&pq3);
    enqueue(&pq3, 10, 5);
    enqueue(&pq3, 20, 5);
    enqueue(&pq3, 30, 5);
    klee_assert(pq3.size == 3);
    klee_assert(is_properly_ordered(&pq3));
    
    return 0;
} 