#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

typedef struct node {
    struct node *prev, *next;
    int value;
} ListNode;

// Create a list node
ListNode* create_node(int data) {
    ListNode* new_list = (ListNode*)malloc(sizeof(ListNode));
    new_list->value = data;
    new_list->next = new_list;
    new_list->prev = new_list;
    return new_list;
}

// Insert a node at start of list
ListNode* insert_at_head(ListNode* head, int data) {
    if (head == NULL) {
        head = create_node(data);
        return head;
    } else {
        ListNode* temp;
        ListNode* new_node = create_node(data);
        temp = head->prev;
        new_node->next = head;
        head->prev = new_node;
        new_node->prev = temp;
        temp->next = new_node;
        head = new_node;
        return head;
    }
}

// Insert a node at end of list
ListNode* insert_at_tail(ListNode* head, int data) {
    if (head == NULL) {
        head = create_node(data);
        return head;
    } else {
        ListNode* temp1, *temp2;
        ListNode* new_node = create_node(data);
        temp1 = head;
        temp2 = head->prev;
        new_node->prev = temp2;
        new_node->next = temp1;
        temp1->prev = new_node;
        temp2->next = new_node;
        return head;
    }
}

// Delete first node
ListNode* delete_first(ListNode* head) {
    if (head == NULL) {
        return NULL;
    }
    
    if (head->next == head) {
        free(head);
        return NULL;
    }
    
    ListNode* temp = head;
    ListNode* last = head->prev;
    head = head->next;
    head->prev = last;
    last->next = head;
    free(temp);
    return head;
}

// Delete last node
ListNode* delete_last(ListNode* head) {
    if (head == NULL) {
        return NULL;
    }
    
    if (head->next == head) {
        free(head);
        return NULL;
    }
    
    ListNode* last = head->prev;
    ListNode* second_last = last->prev;
    second_last->next = head;
    head->prev = second_last;
    free(last);
    return head;
}

// Get list size
int get_size(ListNode* head) {
    if (head == NULL) {
        return 0;
    }
    
    int count = 0;
    ListNode* current = head;
    do {
        count++;
        current = current->next;
    } while (current != head);
    
    return count;
}

// Get value at index
int get(ListNode* list, int index) {
    if (list == NULL || index < 0) {
        return -1;
    }
    
    ListNode* current = list;
    for (int i = 0; i < index; i++) {
        current = current->next;
        if (current == list) {
            return -1; // Index out of bounds
        }
    }
    return current->value;
}

// Check if list is circular
int is_circular(ListNode* head) {
    if (head == NULL) {
        return 1;
    }
    
    ListNode* current = head;
    int count = 0;
    int max_iterations = 100; // Prevent infinite loop
    
    do {
        current = current->next;
        count++;
        if (count > max_iterations) {
            return 0; // Not circular
        }
    } while (current != head);
    
    return 1;
}

// Check if list is doubly linked
int is_doubly_linked(ListNode* head) {
    if (head == NULL) {
        return 1;
    }
    
    ListNode* current = head;
    do {
        if (current->next->prev != current) {
            return 0;
        }
        current = current->next;
    } while (current != head);
    
    return 1;
}

// Search for a value
ListNode* search(ListNode* head, int value) {
    if (head == NULL) {
        return NULL;
    }
    
    ListNode* current = head;
    do {
        if (current->value == value) {
            return current;
        }
        current = current->next;
    } while (current != head);
    
    return NULL;
}

// KLEE test function
int main() {
    ListNode* head = NULL;
    
    // Symbolic inputs
    int val1, val2, val3, val4, search_val, delete_choice;
    klee_make_symbolic(&val1, sizeof(val1), "val1");
    klee_make_symbolic(&val2, sizeof(val2), "val2");
    klee_make_symbolic(&val3, sizeof(val3), "val3");
    klee_make_symbolic(&val4, sizeof(val4), "val4");
    klee_make_symbolic(&search_val, sizeof(search_val), "search_val");
    klee_make_symbolic(&delete_choice, sizeof(delete_choice), "delete_choice");
    
    // Constrain inputs
    klee_assume(val1 >= 0 && val1 <= 100);
    klee_assume(val2 >= 0 && val2 <= 100);
    klee_assume(val3 >= 0 && val3 <= 100);
    klee_assume(val4 >= 0 && val4 <= 100);
    klee_assume(search_val >= 0 && search_val <= 100);
    klee_assume(delete_choice >= 0 && delete_choice <= 1);
    
    // Insert nodes
    head = insert_at_head(head, val1);
    klee_assert(head != NULL);
    klee_assert(get_size(head) == 1);
    klee_assert(is_circular(head));
    klee_assert(is_doubly_linked(head));
    
    head = insert_at_tail(head, val2);
    klee_assert(get_size(head) == 2);
    klee_assert(is_circular(head));
    klee_assert(is_doubly_linked(head));
    
    head = insert_at_head(head, val3);
    klee_assert(get_size(head) == 3);
    klee_assert(is_circular(head));
    klee_assert(is_doubly_linked(head));
    
    head = insert_at_tail(head, val4);
    klee_assert(get_size(head) == 4);
    klee_assert(is_circular(head));
    klee_assert(is_doubly_linked(head));
    
    // Test get function
    int first_val = get(head, 0);
    int last_val = get(head, 3);
    klee_assert(first_val >= 0);
    klee_assert(last_val >= 0);
    
    // Test search
    ListNode* found = search(head, search_val);
    if (search_val == val1 || search_val == val2 || 
        search_val == val3 || search_val == val4) {
        klee_assert(found != NULL);
        klee_assert(found->value == search_val);
    } else {
        klee_assert(found == NULL);
    }
    
    // Test deletion
    if (delete_choice == 0) {
        int size_before = get_size(head);
        head = delete_first(head);
        int size_after = get_size(head);
        klee_assert(size_after == size_before - 1);
        klee_assert(is_circular(head));
        klee_assert(is_doubly_linked(head));
    } else {
        int size_before = get_size(head);
        head = delete_last(head);
        int size_after = get_size(head);
        klee_assert(size_after == size_before - 1);
        klee_assert(is_circular(head));
        klee_assert(is_doubly_linked(head));
    }
    
    // Test circular properties
    if (head != NULL) {
        klee_assert(head->next->prev == head);
        klee_assert(head->prev->next == head);
        
        // Traverse full circle
        ListNode* current = head;
        int count = 0;
        do {
            count++;
            current = current->next;
            klee_assert(count <= 10); // Prevent infinite loop
        } while (current != head);
        
        klee_assert(count == get_size(head));
    }
    
    return 0;
} 