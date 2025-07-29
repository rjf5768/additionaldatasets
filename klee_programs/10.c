#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

#define MAX_SIZE 10
#define MAX_KEYS 3

// Structure for multi-key data
struct multi_key_data {
    int keys[MAX_KEYS];
    int id;
};

// Compare function for multi-key sorting
int compare_multi_key(struct multi_key_data* a, struct multi_key_data* b, int key_index) {
    if (key_index >= MAX_KEYS) return 0;
    return a->keys[key_index] - b->keys[key_index];
}

// Partition function for quick sort
int partition(struct multi_key_data* arr, int low, int high, int key_index) {
    struct multi_key_data pivot = arr[high];
    int i = low - 1;
    
    for (int j = low; j < high; j++) {
        if (compare_multi_key(&arr[j], &pivot, key_index) <= 0) {
            i++;
            // Swap elements
            struct multi_key_data temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    
    // Swap pivot
    struct multi_key_data temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    
    return i + 1;
}

// Quick sort for single key
void quick_sort_single_key(struct multi_key_data* arr, int low, int high, int key_index) {
    if (low < high) {
        int pi = partition(arr, low, high, key_index);
        quick_sort_single_key(arr, low, pi - 1, key_index);
        quick_sort_single_key(arr, pi + 1, high, key_index);
    }
}

// Multi-key quick sort
void multi_key_quick_sort(struct multi_key_data* arr, int size, int num_keys) {
    // Sort by each key in reverse order (most significant key first)
    for (int key = num_keys - 1; key >= 0; key--) {
        quick_sort_single_key(arr, 0, size - 1, key);
    }
}

// Check if array is sorted by a specific key
int is_sorted_by_key(struct multi_key_data* arr, int size, int key_index) {
    for (int i = 1; i < size; i++) {
        if (compare_multi_key(&arr[i-1], &arr[i], key_index) > 0) {
            return 0;
        }
    }
    return 1;
}

// Check if array is sorted by all keys
int is_sorted_multi_key(struct multi_key_data* arr, int size, int num_keys) {
    for (int key = 0; key < num_keys; key++) {
        if (!is_sorted_by_key(arr, size, key)) {
            return 0;
        }
    }
    return 1;
}

// Generate test data
void generate_test_data(struct multi_key_data* arr, int size, int num_keys) {
    for (int i = 0; i < size; i++) {
        arr[i].id = i;
        for (int j = 0; j < num_keys; j++) {
            arr[i].keys[j] = rand() % 100; // Random values 0-99
        }
    }
}

// Print array (for debugging)
void print_array(struct multi_key_data* arr, int size, int num_keys) {
    for (int i = 0; i < size; i++) {
        printf("ID: %d, Keys: ", arr[i].id);
        for (int j = 0; j < num_keys; j++) {
            printf("%d ", arr[i].keys[j]);
        }
        printf("\n");
    }
}

// KLEE test function
int main() {
    // Symbolic parameters
    int size, num_keys;
    klee_make_symbolic(&size, sizeof(size), "size");
    klee_make_symbolic(&num_keys, sizeof(num_keys), "num_keys");
    
    // Constrain parameters
    klee_assume(size >= 2 && size <= MAX_SIZE);
    klee_assume(num_keys >= 1 && num_keys <= MAX_KEYS);
    
    // Allocate array
    struct multi_key_data* arr = malloc(size * sizeof(struct multi_key_data));
    
    // Initialize with symbolic data
    for (int i = 0; i < size; i++) {
        arr[i].id = i;
        for (int j = 0; j < num_keys; j++) {
            klee_make_symbolic(&arr[i].keys[j], sizeof(int), "key");
            klee_assume(arr[i].keys[j] >= 0 && arr[i].keys[j] <= 99);
        }
    }
    
    // Test individual key sorting
    for (int key = 0; key < num_keys; key++) {
        // Create a copy for testing
        struct multi_key_data* test_arr = malloc(size * sizeof(struct multi_key_data));
        for (int i = 0; i < size; i++) {
            test_arr[i] = arr[i];
        }
        
        // Sort by single key
        quick_sort_single_key(test_arr, 0, size - 1, key);
        
        // Verify sorting
        klee_assert(is_sorted_by_key(test_arr, size, key));
        
        // Test that all elements are preserved
        int found_ids[MAX_SIZE] = {0};
        for (int i = 0; i < size; i++) {
            found_ids[test_arr[i].id] = 1;
        }
        for (int i = 0; i < size; i++) {
            klee_assert(found_ids[i] == 1);
        }
        
        free(test_arr);
    }
    
    // Test multi-key sorting
    multi_key_quick_sort(arr, size, num_keys);
    
    // Verify multi-key sorting
    klee_assert(is_sorted_multi_key(arr, size, num_keys));
    
    // Test stability properties
    for (int i = 1; i < size; i++) {
        // If all keys are equal, IDs should maintain relative order
        int keys_equal = 1;
        for (int key = 0; key < num_keys; key++) {
            if (arr[i-1].keys[key] != arr[i].keys[key]) {
                keys_equal = 0;
                break;
            }
        }
        
        if (keys_equal) {
            // For equal keys, check if relative order is maintained
            klee_assert(arr[i-1].id <= arr[i].id);
        }
    }
    
    // Test edge cases
    if (size >= 3) {
        // Test with duplicate keys
        arr[0].keys[0] = arr[1].keys[0];
        quick_sort_single_key(arr, 0, size - 1, 0);
        klee_assert(is_sorted_by_key(arr, size, 0));
    }
    
    // Test partition function
    if (size >= 2) {
        int pivot_index = partition(arr, 0, size - 1, 0);
        klee_assert(pivot_index >= 0 && pivot_index < size);
        
        // Elements before pivot should be <= pivot
        for (int i = 0; i < pivot_index; i++) {
            klee_assert(compare_multi_key(&arr[i], &arr[pivot_index], 0) <= 0);
        }
        
        // Elements after pivot should be > pivot
        for (int i = pivot_index + 1; i < size; i++) {
            klee_assert(compare_multi_key(&arr[i], &arr[pivot_index], 0) > 0);
        }
    }
    
    free(arr);
    return 0;
} 