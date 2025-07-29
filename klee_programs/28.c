#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#define MAX_SIZE 100
#define MAX_QUERIES 20

typedef struct {
    int data[MAX_SIZE * 4]; // 4x size for segment tree
    int size;
    int n;
} SegmentTree;

// Initialize segment tree
void init_segment_tree(SegmentTree* st, int arr[], int n) {
    st->n = n;
    st->size = 4 * n;
    
    // Initialize all elements to 0
    for (int i = 0; i < st->size; i++) {
        st->data[i] = 0;
    }
}

// Build segment tree
void build_segment_tree(SegmentTree* st, int arr[], int node, int start, int end) {
    if (start == end) {
        st->data[node] = arr[start];
        return;
    }
    
    int mid = (start + end) / 2;
    build_segment_tree(st, arr, 2 * node + 1, start, mid);
    build_segment_tree(st, arr, 2 * node + 2, mid + 1, end);
    
    st->data[node] = st->data[2 * node + 1] + st->data[2 * node + 2];
}

// Update element at index
void update_segment_tree(SegmentTree* st, int node, int start, int end, int index, int value) {
    if (start == end) {
        st->data[node] = value;
        return;
    }
    
    int mid = (start + end) / 2;
    if (index <= mid) {
        update_segment_tree(st, 2 * node + 1, start, mid, index, value);
    } else {
        update_segment_tree(st, 2 * node + 2, mid + 1, end, index, value);
    }
    
    st->data[node] = st->data[2 * node + 1] + st->data[2 * node + 2];
}

// Range sum query
int range_sum_query(SegmentTree* st, int node, int start, int end, int left, int right) {
    if (right < start || left > end) {
        return 0;
    }
    
    if (left <= start && right >= end) {
        return st->data[node];
    }
    
    int mid = (start + end) / 2;
    int left_sum = range_sum_query(st, 2 * node + 1, start, mid, left, right);
    int right_sum = range_sum_query(st, 2 * node + 2, mid + 1, end, left, right);
    
    return left_sum + right_sum;
}

// Range minimum query
int range_min_query(SegmentTree* st, int node, int start, int end, int left, int right) {
    if (right < start || left > end) {
        return INT_MAX;
    }
    
    if (left <= start && right >= end) {
        return st->data[node];
    }
    
    int mid = (start + end) / 2;
    int left_min = range_min_query(st, 2 * node + 1, start, mid, left, right);
    int right_min = range_min_query(st, 2 * node + 2, mid + 1, end, left, right);
    
    return (left_min < right_min) ? left_min : right_min;
}

// Range maximum query
int range_max_query(SegmentTree* st, int node, int start, int end, int left, int right) {
    if (right < start || left > end) {
        return INT_MIN;
    }
    
    if (left <= start && right >= end) {
        return st->data[node];
    }
    
    int mid = (start + end) / 2;
    int left_max = range_max_query(st, 2 * node + 1, start, mid, left, right);
    int right_max = range_max_query(st, 2 * node + 2, mid + 1, end, left, right);
    
    return (left_max > right_max) ? left_max : right_max;
}

// Get element at index
int get_element(SegmentTree* st, int index) {
    if (index < 0 || index >= st->n) {
        return -1; // Error value
    }
    return range_sum_query(st, 0, 0, st->n - 1, index, index);
}

// Check if range is valid
int is_valid_range(int left, int right, int n) {
    return left >= 0 && right < n && left <= right;
}

// Check if index is valid
int is_valid_index(int index, int n) {
    return index >= 0 && index < n;
}

// Count elements in range
int count_elements_in_range(int left, int right) {
    return right - left + 1;
}

// Check if segment tree is properly built
int is_properly_built(SegmentTree* st) {
    for (int i = 0; i < st->size; i++) {
        if (st->data[i] < 0) {
            return 0; // Negative values not allowed
        }
    }
    return 1;
}

// KLEE test function
int main() {
    int arr[MAX_SIZE];
    int n, queries[MAX_QUERIES][3]; // [left, right, operation_type]
    
    // Make inputs symbolic
    klee_make_symbolic(&n, sizeof(int), "n");
    klee_assume(n >= 1 && n <= MAX_SIZE);
    
    for (int i = 0; i < n; i++) {
        klee_make_symbolic(&arr[i], sizeof(int), "arr_element");
        klee_assume(arr[i] >= 0 && arr[i] <= 1000); // Non-negative values
    }
    
    for (int i = 0; i < MAX_QUERIES; i++) {
        klee_make_symbolic(&queries[i][0], sizeof(int), "query_left");
        klee_make_symbolic(&queries[i][1], sizeof(int), "query_right");
        klee_make_symbolic(&queries[i][2], sizeof(int), "query_type");
        klee_assume(queries[i][0] >= 0 && queries[i][0] < n);
        klee_assume(queries[i][1] >= 0 && queries[i][1] < n);
        klee_assume(queries[i][2] >= 0 && queries[i][2] <= 2); // 0=sum, 1=min, 2=max
    }
    
    // Initialize and build segment tree
    SegmentTree st;
    init_segment_tree(&st, arr, n);
    build_segment_tree(&st, arr, 0, 0, n - 1);
    
    // Test initial state
    klee_assert(is_properly_built(&st));
    klee_assert(st.n == n);
    klee_assert(st.size == 4 * n);
    
    // Test range queries
    for (int i = 0; i < MAX_QUERIES; i++) {
        int left = queries[i][0];
        int right = queries[i][1];
        int type = queries[i][2];
        
        if (is_valid_range(left, right, n)) {
            int result;
            
            switch (type) {
                case 0: // Sum query
                    result = range_sum_query(&st, 0, 0, n - 1, left, right);
                    klee_assert(result >= 0);
                    
                    // Verify with brute force
                    int expected_sum = 0;
                    for (int j = left; j <= right; j++) {
                        expected_sum += arr[j];
                    }
                    klee_assert(result == expected_sum);
                    break;
                    
                case 1: // Min query
                    result = range_min_query(&st, 0, 0, n - 1, left, right);
                    klee_assert(result >= 0);
                    
                    // Verify with brute force
                    int expected_min = arr[left];
                    for (int j = left + 1; j <= right; j++) {
                        if (arr[j] < expected_min) {
                            expected_min = arr[j];
                        }
                    }
                    klee_assert(result == expected_min);
                    break;
                    
                case 2: // Max query
                    result = range_max_query(&st, 0, 0, n - 1, left, right);
                    klee_assert(result >= 0);
                    
                    // Verify with brute force
                    int expected_max = arr[left];
                    for (int j = left + 1; j <= right; j++) {
                        if (arr[j] > expected_max) {
                            expected_max = arr[j];
                        }
                    }
                    klee_assert(result == expected_max);
                    break;
            }
        }
    }
    
    // Test single element queries
    for (int i = 0; i < n; i++) {
        int element = get_element(&st, i);
        klee_assert(element == arr[i]);
    }
    
    // Test invalid queries
    klee_assert(get_element(&st, -1) == -1);
    klee_assert(get_element(&st, n) == -1);
    
    // Test range properties
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            int sum = range_sum_query(&st, 0, 0, n - 1, i, j);
            int count = count_elements_in_range(i, j);
            klee_assert(sum >= 0);
            klee_assert(count == j - i + 1);
        }
    }
    
    // Test with specific values
    int test_arr[5] = {1, 2, 3, 4, 5};
    SegmentTree test_st;
    init_segment_tree(&test_st, test_arr, 5);
    build_segment_tree(&test_st, test_arr, 0, 0, 4);
    
    klee_assert(range_sum_query(&test_st, 0, 0, 4, 0, 4) == 15);
    klee_assert(range_sum_query(&test_st, 0, 0, 4, 1, 3) == 9);
    klee_assert(range_min_query(&test_st, 0, 0, 4, 0, 4) == 1);
    klee_assert(range_max_query(&test_st, 0, 0, 4, 0, 4) == 5);
    
    // Test updates
    update_segment_tree(&test_st, 0, 0, 4, 2, 10);
    klee_assert(range_sum_query(&test_st, 0, 0, 4, 0, 4) == 22);
    klee_assert(get_element(&test_st, 2) == 10);
    
    // Test edge cases
    // Single element array
    int single_arr[1] = {42};
    SegmentTree single_st;
    init_segment_tree(&single_st, single_arr, 1);
    build_segment_tree(&single_st, single_arr, 0, 0, 0);
    
    klee_assert(range_sum_query(&single_st, 0, 0, 0, 0, 0) == 42);
    klee_assert(range_min_query(&single_st, 0, 0, 0, 0, 0) == 42);
    klee_assert(range_max_query(&single_st, 0, 0, 0, 0, 0) == 42);
    
    // Test with all zeros
    int zero_arr[3] = {0, 0, 0};
    SegmentTree zero_st;
    init_segment_tree(&zero_st, zero_arr, 3);
    build_segment_tree(&zero_st, zero_arr, 0, 0, 2);
    
    klee_assert(range_sum_query(&zero_st, 0, 0, 2, 0, 2) == 0);
    klee_assert(range_min_query(&zero_st, 0, 0, 2, 0, 2) == 0);
    klee_assert(range_max_query(&zero_st, 0, 0, 2, 0, 2) == 0);
    
    return 0;
} 