#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

typedef struct node {
    int val;
    struct node *par;
    struct node *left;
    struct node *right;
    int color;  // 0 = black, 1 = red
} Node;

// Create a new node
Node *newNode(int val, Node *par) {
    Node *create = (Node *)(malloc(sizeof(Node)));
    create->val = val;
    create->par = par;
    create->left = NULL;
    create->right = NULL;
    create->color = 1;  // Red by default
    return create;
}

// Check if node is leaf
int isLeaf(Node *n) {
    return (n->left == NULL && n->right == NULL);
}

// Left rotate
Node *leftRotate(Node *node) {
    Node *parent = node->par;
    Node *grandParent = parent->par;

    parent->right = node->left;
    if (node->left != NULL) {
        node->left->par = parent;
    }
    node->par = grandParent;
    parent->par = node;
    node->left = parent;
    if (grandParent != NULL) {
        if (grandParent->right == parent) {
            grandParent->right = node;
        } else {
            grandParent->left = node;
        }
    }
    return node;
}

// Right rotate
Node *rightRotate(Node *node) {
    Node *parent = node->par;
    Node *grandParent = parent->par;

    parent->left = node->right;
    if (node->right != NULL) {
        node->right->par = parent;
    }
    node->par = grandParent;
    parent->par = node;
    node->right = parent;
    if (grandParent != NULL) {
        if (grandParent->right == parent) {
            grandParent->right = node;
        } else {
            grandParent->left = node;
        }
    }
    return node;
}

// Insert node into tree
void insertNode(int val, Node **root) {
    Node *new = newNode(val, NULL);
    
    if (*root == NULL) {
        new->color = 0;  // Root is always black
        *root = new;
        return;
    }
    
    Node *current = *root;
    Node *parent = NULL;
    
    // Find insertion position
    while (current != NULL) {
        parent = current;
        if (val < current->val) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    
    new->par = parent;
    if (val < parent->val) {
        parent->left = new;
    } else {
        parent->right = new;
    }
    
    // Fix red-black properties (simplified)
    if (parent->color == 1) {
        new->color = 0;  // Make new node black if parent is red
    }
}

// Search for a value
Node *search(Node *root, int val) {
    if (root == NULL || root->val == val) {
        return root;
    }
    
    if (val < root->val) {
        return search(root->left, val);
    }
    return search(root->right, val);
}

// Count nodes in tree
int countNodes(Node *root) {
    if (root == NULL) {
        return 0;
    }
    return 1 + countNodes(root->left) + countNodes(root->right);
}

// Check if tree is valid BST
int isValidBST(Node *root, int min, int max) {
    if (root == NULL) {
        return 1;
    }
    
    if (root->val <= min || root->val >= max) {
        return 0;
    }
    
    return isValidBST(root->left, min, root->val) && 
           isValidBST(root->right, root->val, max);
}

// KLEE test function
int main() {
    Node *root = NULL;
    
    // Symbolic inputs for tree operations
    int val1, val2, val3, search_val;
    klee_make_symbolic(&val1, sizeof(val1), "val1");
    klee_make_symbolic(&val2, sizeof(val2), "val2");
    klee_make_symbolic(&val3, sizeof(val3), "val3");
    klee_make_symbolic(&search_val, sizeof(search_val), "search_val");
    
    // Constrain inputs to reasonable range
    klee_assume(val1 >= 0 && val1 <= 100);
    klee_assume(val2 >= 0 && val2 <= 100);
    klee_assume(val3 >= 0 && val3 <= 100);
    klee_assume(search_val >= 0 && search_val <= 100);
    
    // Ensure different values
    klee_assume(val1 != val2 && val2 != val3 && val1 != val3);
    
    // Insert nodes
    insertNode(val1, &root);
    klee_assert(root != NULL);
    klee_assert(countNodes(root) == 1);
    
    insertNode(val2, &root);
    klee_assert(countNodes(root) == 2);
    
    insertNode(val3, &root);
    klee_assert(countNodes(root) == 3);
    
    // Test BST property
    klee_assert(isValidBST(root, -1, 101));
    
    // Test search
    Node *found = search(root, search_val);
    if (search_val == val1 || search_val == val2 || search_val == val3) {
        klee_assert(found != NULL);
    } else {
        klee_assert(found == NULL);
    }
    
    // Test tree structure
    if (root->left != NULL) {
        klee_assert(root->left->val < root->val);
    }
    if (root->right != NULL) {
        klee_assert(root->right->val > root->val);
    }
    
    return 0;
} 