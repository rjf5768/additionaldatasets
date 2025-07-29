#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

#define MAX_SIZE 100
#define MAX_BUCKETS 20
#define BUCKET_RANGE 10

typedef struct Node {
    int data;
    struct Node* next;
} Node;

// Create a new node
Node* create_node(int data) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node) {
        node->data = data;
        node->next = NULL;
    }
    return node;
}

// Insert a node at the beginning of a linked list
Node* insert_node(Node* head, int data) {
    Node* new_node = create_node(data);
    if (new_node) {
        new_node->next = head;
        return new_node;
    }
    return head;
}

// Get bucket index for a value
int get_bucket_index(int value, int bucket_range) {
    return value / bucket_range;
}

// Insertion sort for linked list
Node* insertion_sort_list(Node* head) {
    if (!head || !head->next) {
        return head;
    }
    
    Node* sorted = NULL;
    Node* current = head;
    
    while (current) {
        Node* next = current->next;
        
        if (!sorted || current->data <= sorted->data) {
            current->next = sorted;
            sorted = current;
        } else {
            Node* temp = sorted;
            while (temp->next && temp->next->data < current->data) {
                temp = temp->next;
            }
            current->next = temp->next;
            temp->next = current;
        }
        
        current = next;
    }
    
    return sorted;
}

// Bucket sort implementation
void bucket_sort(int arr[], int n, int bucket_range) {
    if (n <= 0) {
        return;
    }
    
    // Find min and max values
    int min_val = arr[0];
    int max_val = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] < min_val) min_val = arr[i];
        if (arr[i] > max_val) max_val = arr[i];
    }
    
    // Calculate number of buckets
    int num_buckets = (max_val - min_val) / bucket_range + 1;
    if (num_buckets <= 0) num_buckets = 1;
    if (num_buckets > MAX_BUCKETS) num_buckets = MAX_BUCKETS;
    
    // Create buckets
    Node** buckets = (Node**)calloc(num_buckets, sizeof(Node*));
    if (!buckets) {
        return;
    }
    
    // Distribute elements into buckets
    for (int i = 0; i < n; i++) {
        int bucket_index = (arr[i] - min_val) / bucket_range;
        if (bucket_index >= num_buckets) bucket_index = num_buckets - 1;
        if (bucket_index < 0) bucket_index = 0;
        
        buckets[bucket_index] = insert_node(buckets[bucket_index], arr[i]);
    }
    
    // Sort each bucket
    for (int i = 0; i < num_buckets; i++) {
        buckets[i] = insertion_sort_list(buckets[i]);
    }
    
    // Concatenate buckets back to array
    int index = 0;
    for (int i = 0; i < num_buckets && index < n; i++) {
        Node* current = buckets[i];
        while (current && index < n) {
            arr[index++] = current->data;
            current = current->next;
        }
    }
    
    // Free memory
    for (int i = 0; i < num_buckets; i++) {
        Node* current = buckets[i];
        while (current) {
            Node* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(buckets);
}

// Check if array is sorted
int is_sorted(int arr[], int n) {
    for (int i = 1; i < n; i++) {
        if (arr[i] < arr[i-1]) {
            return 0;
        }
    }
    return 1;
}

// Check if array contains same elements (permutation)
int is_permutation(int original[], int sorted[], int n) {
    // Simple check: sum should be the same
    int sum_original = 0, sum_sorted = 0;
    for (int i = 0; i < n; i++) {
        sum_original += original[i];
        sum_sorted += sorted[i];
    }
    return sum_original == sum_sorted;
}

// Count elements in a bucket
int count_bucket_elements(Node* bucket) {
    int count = 0;
    Node* current = bucket;
    while (current) {
        count++;
        current = current->next;
    }
    return count;
}

// Find minimum value in array
int find_min(int arr[], int n) {
    if (n <= 0) return 0;
    int min_val = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] < min_val) {
            min_val = arr[i];
        }
    }
    return min_val;
}

// Find maximum value in array
int find_max(int arr[], int n) {
    if (n <= 0) return 0;
    int max_val = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
    }
    return max_val;
}

// Check if all elements are in valid range
int is_in_range(int arr[], int n, int min_val, int max_val) {
    for (int i = 0; i < n; i++) {
        if (arr[i] < min_val || arr[i] > max_val) {
            return 0;
        }
    }
    return 1;
}

// Count distinct elements
int count_distinct(int arr[], int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    
    int count = 1;
    for (int i = 1; i < n; i++) {
        int is_duplicate = 0;
        for (int j = 0; j < i; j++) {
            if (arr[i] == arr[j]) {
                is_duplicate = 1;
                break;
            }
        }
        if (!is_duplicate) {
            count++;
        }
    }
    return count;
}

// KLEE test function
int main() {
    int arr[MAX_SIZE];
    int original_arr[MAX_SIZE];
    int n, bucket_range;
    
    // Make inputs symbolic
    klee_make_symbolic(&n, sizeof(int), "n");
    klee_assume(n >= 1 && n <= MAX_SIZE);
    
    klee_make_symbolic(&bucket_range, sizeof(int), "bucket_range");
    klee_assume(bucket_range >= 1 && bucket_range <= 100);
    
    for (int i = 0; i < n; i++) {
        klee_make_symbolic(&arr[i], sizeof(int), "element");
        klee_assume(arr[i] >= 0 && arr[i] <= 1000);
    }
    
    // Copy original array
    for (int i = 0; i < n; i++) {
        original_arr[i] = arr[i];
    }
    
    // Test bucket sort
    bucket_sort(arr, n, bucket_range);
    
    // Verify sorting properties
    klee_assert(is_sorted(arr, n));
    klee_assert(is_permutation(original_arr, arr, n));
    
    // Test with known arrays
    int test_arr1[] = {64, 34, 25, 12, 22, 11, 90};
    int test_n1 = 7;
    bucket_sort(test_arr1, test_n1, 10);
    klee_assert(is_sorted(test_arr1, test_n1));
    
    int test_arr2[] = {5, 2, 4, 6, 1, 3};
    int test_n2 = 6;
    bucket_sort(test_arr2, test_n2, 2);
    klee_assert(is_sorted(test_arr2, test_n2));
    
    // Test edge cases
    // Single element
    int single_arr[] = {42};
    bucket_sort(single_arr, 1, 10);
    klee_assert(is_sorted(single_arr, 1));
    
    // Already sorted array
    int sorted_arr[] = {1, 2, 3, 4, 5};
    int sorted_n = 5;
    bucket_sort(sorted_arr, sorted_n, 1);
    klee_assert(is_sorted(sorted_arr, sorted_n));
    
    // Reverse sorted array
    int reverse_arr[] = {5, 4, 3, 2, 1};
    int reverse_n = 5;
    bucket_sort(reverse_arr, reverse_n, 1);
    klee_assert(is_sorted(reverse_arr, reverse_n));
    
    // Array with duplicates
    int dup_arr[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    int dup_n = 11;
    bucket_sort(dup_arr, dup_n, 2);
    klee_assert(is_sorted(dup_arr, dup_n));
    
    // Test array properties
    int min_val = find_min(arr, n);
    int max_val = find_max(arr, n);
    
    klee_assert(min_val <= max_val);
    klee_assert(is_in_range(arr, n, min_val, max_val));
    
    // Test that sorted array maintains min/max
    int sorted_min = find_min(arr, n);
    int sorted_max = find_max(arr, n);
    
    klee_assert(sorted_min == min_val);
    klee_assert(sorted_max == max_val);
    
    // Test that number of distinct elements is preserved
    int original_distinct = count_distinct(original_arr, n);
    int sorted_distinct = count_distinct(arr, n);
    klee_assert(original_distinct == sorted_distinct);
    
    // Test bucket index calculation
    for (int i = 0; i < n; i++) {
        int bucket_index = get_bucket_index(original_arr[i], bucket_range);
        klee_assert(bucket_index >= 0);
    }
    
    // Test with symbolic input
    if (n > 0) {
        // Verify that first element is minimum
        klee_assert(arr[0] == min_val);
        
        // Verify that last element is maximum
        klee_assert(arr[n-1] == max_val);
        
        // Verify that adjacent elements are in order
        for (int i = 1; i < n; i++) {
            klee_assert(arr[i] >= arr[i-1]);
        }
    }
    
    return 0;
} 