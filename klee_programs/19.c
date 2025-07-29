#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

typedef struct Node {
    int data;
    struct Node* prev;
    struct Node* next;
} Node;

// Create a new node
Node* create_node(int data) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->data = data;
    new_node->prev = NULL;
    new_node->next = NULL;
    return new_node;
}

// Insert at beginning
Node* insert_at_beginning(Node* head, int data) {
    Node* new_node = create_node(data);
    
    if (head == NULL) {
        return new_node;
    }
    
    new_node->next = head;
    head->prev = new_node;
    return new_node;
}

// Insert at end
Node* insert_at_end(Node* head, int data) {
    Node* new_node = create_node(data);
    
    if (head == NULL) {
        return new_node;
    }
    
    Node* current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    
    current->next = new_node;
    new_node->prev = current;
    return head;
}

// Insert after a given node
void insert_after(Node* prev_node, int data) {
    if (prev_node == NULL) return;
    
    Node* new_node = create_node(data);
    new_node->next = prev_node->next;
    new_node->prev = prev_node;
    
    if (prev_node->next != NULL) {
        prev_node->next->prev = new_node;
    }
    
    prev_node->next = new_node;
}

// Delete first node
Node* delete_first(Node* head) {
    if (head == NULL) return NULL;
    
    Node* temp = head;
    head = head->next;
    
    if (head != NULL) {
        head->prev = NULL;
    }
    
    free(temp);
    return head;
}

// Delete last node
Node* delete_last(Node* head) {
    if (head == NULL) return NULL;
    
    if (head->next == NULL) {
        free(head);
        return NULL;
    }
    
    Node* current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    
    current->prev->next = NULL;
    free(current);
    return head;
}

// Delete node with given data
Node* delete_node(Node* head, int data) {
    if (head == NULL) return NULL;
    
    Node* current = head;
    
    // Find the node to delete
    while (current != NULL && current->data != data) {
        current = current->next;
    }
    
    if (current == NULL) return head; // Data not found
    
    // If node to be deleted is head
    if (current == head) {
        head = head->next;
        if (head != NULL) {
            head->prev = NULL;
        }
        free(current);
        return head;
    }
    
    // Update links
    current->prev->next = current->next;
    if (current->next != NULL) {
        current->next->prev = current->prev;
    }
    
    free(current);
    return head;
}

// Search for a value
Node* search(Node* head, int data) {
    Node* current = head;
    
    while (current != NULL) {
        if (current->data == data) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// Count nodes
int count_nodes(Node* head) {
    int count = 0;
    Node* current = head;
    
    while (current != NULL) {
        count++;
        current = current->next;
    }
    
    return count;
}

// Check if list is properly doubly linked
int is_properly_linked(Node* head) {
    if (head == NULL) return 1;
    
    Node* current = head;
    
    // Check forward links
    while (current->next != NULL) {
        if (current->next->prev != current) {
            return 0;
        }
        current = current->next;
    }
    
    // Check backward links
    while (current->prev != NULL) {
        if (current->prev->next != current) {
            return 0;
        }
        current = current->prev;
    }
    
    return 1;
}

// Reverse the list
Node* reverse_list(Node* head) {
    if (head == NULL || head->next == NULL) {
        return head;
    }
    
    Node* current = head;
    Node* temp = NULL;
    
    while (current != NULL) {
        temp = current->prev;
        current->prev = current->next;
        current->next = temp;
        current = current->prev;
    }
    
    if (temp != NULL) {
        head = temp->prev;
    }
    
    return head;
}

// Get value at index
int get_at_index(Node* head, int index) {
    if (head == NULL || index < 0) return -1;
    
    Node* current = head;
    int count = 0;
    
    while (current != NULL && count < index) {
        current = current->next;
        count++;
    }
    
    if (current == NULL) return -1;
    return current->data;
}

// KLEE test function
int main() {
    Node* head = NULL;
    
    // Symbolic inputs
    int val1, val2, val3, val4, search_val, delete_val, index;
    klee_make_symbolic(&val1, sizeof(val1), "val1");
    klee_make_symbolic(&val2, sizeof(val2), "val2");
    klee_make_symbolic(&val3, sizeof(val3), "val3");
    klee_make_symbolic(&val4, sizeof(val4), "val4");
    klee_make_symbolic(&search_val, sizeof(search_val), "search_val");
    klee_make_symbolic(&delete_val, sizeof(delete_val), "delete_val");
    klee_make_symbolic(&index, sizeof(index), "index");
    
    // Constrain inputs
    klee_assume(val1 >= 0 && val1 <= 100);
    klee_assume(val2 >= 0 && val2 <= 100);
    klee_assume(val3 >= 0 && val3 <= 100);
    klee_assume(val4 >= 0 && val4 <= 100);
    klee_assume(search_val >= 0 && search_val <= 100);
    klee_assume(delete_val >= 0 && delete_val <= 100);
    klee_assume(index >= 0 && index <= 10);
    
    // Insert nodes
    head = insert_at_beginning(head, val1);
    klee_assert(head != NULL);
    klee_assert(count_nodes(head) == 1);
    klee_assert(is_properly_linked(head));
    
    head = insert_at_end(head, val2);
    klee_assert(count_nodes(head) == 2);
    klee_assert(is_properly_linked(head));
    
    head = insert_at_beginning(head, val3);
    klee_assert(count_nodes(head) == 3);
    klee_assert(is_properly_linked(head));
    
    head = insert_at_end(head, val4);
    klee_assert(count_nodes(head) == 4);
    klee_assert(is_properly_linked(head));
    
    // Test search
    Node* found = search(head, search_val);
    if (search_val == val1 || search_val == val2 || 
        search_val == val3 || search_val == val4) {
        klee_assert(found != NULL);
        klee_assert(found->data == search_val);
    } else {
        klee_assert(found == NULL);
    }
    
    // Test get at index
    int value_at_index = get_at_index(head, index);
    if (index < 4) {
        klee_assert(value_at_index >= 0);
    } else {
        klee_assert(value_at_index == -1);
    }
    
    // Test deletion
    int size_before = count_nodes(head);
    head = delete_node(head, delete_val);
    int size_after = count_nodes(head);
    
    if (delete_val == val1 || delete_val == val2 || 
        delete_val == val3 || delete_val == val4) {
        klee_assert(size_after == size_before - 1);
    } else {
        klee_assert(size_after == size_before);
    }
    
    klee_assert(is_properly_linked(head));
    
    // Test reverse
    if (head != NULL) {
        Node* reversed = reverse_list(head);
        klee_assert(is_properly_linked(reversed));
        klee_assert(count_nodes(reversed) == count_nodes(head));
        
        // Test that first and last elements are swapped
        if (count_nodes(reversed) >= 2) {
            int first_val = get_at_index(reversed, 0);
            int last_val = get_at_index(reversed, count_nodes(reversed) - 1);
            klee_assert(first_val >= 0 && last_val >= 0);
        }
    }
    
    // Test doubly-linked properties
    if (head != NULL) {
        Node* current = head;
        
        // Test forward traversal
        int forward_count = 0;
        while (current != NULL) {
            forward_count++;
            current = current->next;
            klee_assert(forward_count <= 10); // Prevent infinite loop
        }
        
        // Test backward traversal
        current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        
        int backward_count = 0;
        while (current != NULL) {
            backward_count++;
            current = current->prev;
            klee_assert(backward_count <= 10); // Prevent infinite loop
        }
        
        klee_assert(forward_count == backward_count);
    }
    
    return 0;
} 