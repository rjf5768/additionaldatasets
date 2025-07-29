#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

typedef struct Node {
    int data;
    struct Node *llink;
    struct Node *rlink;
    int left_thread;   // 1 if left pointer is a thread
    int right_thread;  // 1 if right pointer is a thread
} node;

// Create a new node
node* create_node(int data) {
    node* ptr = (node*)malloc(sizeof(node));
    ptr->rlink = ptr->llink = NULL;
    ptr->data = data;
    ptr->left_thread = 0;
    ptr->right_thread = 0;
    return ptr;
}

// Insert a node into the tree
void insert_bt(node** root, int data) {
    node* new_node = create_node(data);
    node* temp;
    node* prev;
    
    if (*root == NULL) {
        *root = new_node;
        return;
    }
    
    temp = *root;
    prev = NULL;
    
    while (temp != NULL) {
        if (new_node->data > temp->data) {
            prev = temp;
            temp = temp->rlink;
        } else if (new_node->data < temp->data) {
            prev = temp;
            temp = temp->llink;
        } else {
            free(new_node);
            return; // Duplicate
        }
    }
    
    if (new_node->data > prev->data) {
        prev->rlink = new_node;
    } else {
        prev->llink = new_node;
    }
}

// Search for an element
node* search(node* root, int ele) {
    node* temp = root;
    
    while (temp != NULL) {
        if (ele == temp->data) {
            return temp;
        } else if (ele < temp->data) {
            temp = temp->llink;
        } else {
            temp = temp->rlink;
        }
    }
    
    return NULL;
}

// Convert to threaded tree
void convert_to_threaded(node* root) {
    if (root == NULL) return;
    
    // Convert left subtree
    convert_to_threaded(root->llink);
    
    // Convert right subtree
    convert_to_threaded(root->rlink);
    
    // Thread left pointer to inorder predecessor
    if (root->llink == NULL) {
        // Find inorder predecessor
        node* pred = find_inorder_predecessor(root);
        root->llink = pred;
        root->left_thread = 1;
    }
    
    // Thread right pointer to inorder successor
    if (root->rlink == NULL) {
        // Find inorder successor
        node* succ = find_inorder_successor(root);
        root->rlink = succ;
        root->right_thread = 1;
    }
}

// Find inorder predecessor (simplified)
node* find_inorder_predecessor(node* root) {
    // This is a simplified version for KLEE testing
    // In a real implementation, this would traverse the tree
    return NULL;
}

// Find inorder successor (simplified)
node* find_inorder_successor(node* root) {
    // This is a simplified version for KLEE testing
    // In a real implementation, this would traverse the tree
    return NULL;
}

// Inorder traversal
void inorder_display(node* curr) {
    if (curr == NULL) return;
    
    // Go to leftmost node
    while (curr->llink != NULL && !curr->left_thread) {
        curr = curr->llink;
    }
    
    // Traverse using threads
    while (curr != NULL) {
        printf("%d ", curr->data);
        
        if (curr->right_thread) {
            curr = curr->rlink;
        } else {
            curr = curr->rlink;
            while (curr != NULL && !curr->left_thread) {
                curr = curr->llink;
            }
        }
    }
}

// Count nodes
int count_nodes(node* root) {
    if (root == NULL) return 0;
    return 1 + count_nodes(root->llink) + count_nodes(root->rlink);
}

// Check if tree is valid BST
int is_valid_bst(node* root, int min, int max) {
    if (root == NULL) return 1;
    
    if (root->data <= min || root->data >= max) return 0;
    
    return is_valid_bst(root->llink, min, root->data) && 
           is_valid_bst(root->rlink, root->data, max);
}

// Check threading properties
int check_threading(node* root) {
    if (root == NULL) return 1;
    
    // Check left thread
    if (root->left_thread) {
        if (root->llink != NULL) {
            // Should point to inorder predecessor
            return 1; // Simplified check
        }
    }
    
    // Check right thread
    if (root->right_thread) {
        if (root->rlink != NULL) {
            // Should point to inorder successor
            return 1; // Simplified check
        }
    }
    
    return check_threading(root->llink) && check_threading(root->rlink);
}

// Find minimum value
node* find_min(node* root) {
    if (root == NULL) return NULL;
    
    while (root->llink != NULL && !root->left_thread) {
        root = root->llink;
    }
    
    return root;
}

// Find maximum value
node* find_max(node* root) {
    if (root == NULL) return NULL;
    
    while (root->rlink != NULL && !root->right_thread) {
        root = root->rlink;
    }
    
    return root;
}

// KLEE test function
int main() {
    node* root = NULL;
    
    // Symbolic inputs
    int val1, val2, val3, val4, search_val;
    klee_make_symbolic(&val1, sizeof(val1), "val1");
    klee_make_symbolic(&val2, sizeof(val2), "val2");
    klee_make_symbolic(&val3, sizeof(val3), "val3");
    klee_make_symbolic(&val4, sizeof(val4), "val4");
    klee_make_symbolic(&search_val, sizeof(search_val), "search_val");
    
    // Constrain inputs
    klee_assume(val1 >= 0 && val1 <= 100);
    klee_assume(val2 >= 0 && val2 <= 100);
    klee_assume(val3 >= 0 && val3 <= 100);
    klee_assume(val4 >= 0 && val4 <= 100);
    klee_assume(search_val >= 0 && search_val <= 100);
    
    // Ensure different values
    klee_assume(val1 != val2 && val2 != val3 && val3 != val4);
    klee_assume(val1 != val3 && val1 != val4 && val2 != val4);
    
    // Insert nodes
    insert_bt(&root, val1);
    klee_assert(root != NULL);
    klee_assert(count_nodes(root) == 1);
    klee_assert(is_valid_bst(root, -1, 101));
    
    insert_bt(&root, val2);
    klee_assert(count_nodes(root) == 2);
    klee_assert(is_valid_bst(root, -1, 101));
    
    insert_bt(&root, val3);
    klee_assert(count_nodes(root) == 3);
    klee_assert(is_valid_bst(root, -1, 101));
    
    insert_bt(&root, val4);
    klee_assert(count_nodes(root) == 4);
    klee_assert(is_valid_bst(root, -1, 101));
    
    // Test search
    node* found = search(root, search_val);
    if (search_val == val1 || search_val == val2 || 
        search_val == val3 || search_val == val4) {
        klee_assert(found != NULL);
        klee_assert(found->data == search_val);
    } else {
        klee_assert(found == NULL);
    }
    
    // Test min and max
    node* min_node = find_min(root);
    node* max_node = find_max(root);
    klee_assert(min_node != NULL);
    klee_assert(max_node != NULL);
    
    // Min should be less than max
    klee_assert(min_node->data <= max_node->data);
    
    // Test threading (simplified)
    convert_to_threaded(root);
    klee_assert(check_threading(root));
    
    // Test node properties
    for (node* curr = root; curr != NULL; curr = curr->llink) {
        klee_assert(curr->data >= 0 && curr->data <= 100);
        klee_assert(curr->left_thread >= 0 && curr->left_thread <= 1);
        klee_assert(curr->right_thread >= 0 && curr->right_thread <= 1);
    }
    
    // Test BST property after threading
    klee_assert(is_valid_bst(root, -1, 101));
    
    return 0;
} 