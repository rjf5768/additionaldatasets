#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

typedef struct AVLNode {
    int key;
    struct AVLNode *left;
    struct AVLNode *right;
    int height;
} AVLNode;

// Get height of node
int getHeight(AVLNode *node) {
    if (node == NULL) return -1;
    return node->height;
}

// Get balance factor
int getBalance(AVLNode *node) {
    if (node == NULL) return 0;
    return getHeight(node->left) - getHeight(node->right);
}

// Create new node
AVLNode* createNode(int key) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    node->height = 0;
    return node;
}

// Right rotate
AVLNode* rightRotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    
    x->right = y;
    y->left = T2;
    
    y->height = 1 + (getHeight(y->left) > getHeight(y->right) ? 
                     getHeight(y->left) : getHeight(y->right));
    x->height = 1 + (getHeight(x->left) > getHeight(x->right) ? 
                     getHeight(x->left) : getHeight(x->right));
    
    return x;
}

// Left rotate
AVLNode* leftRotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    
    y->left = x;
    x->right = T2;
    
    x->height = 1 + (getHeight(x->left) > getHeight(x->right) ? 
                     getHeight(x->left) : getHeight(x->right));
    y->height = 1 + (getHeight(y->left) > getHeight(y->right) ? 
                     getHeight(y->left) : getHeight(y->right));
    
    return y;
}

// Insert node
AVLNode* insert(AVLNode* node, int key) {
    if (node == NULL) {
        return createNode(key);
    }
    
    if (key < node->key) {
        node->left = insert(node->left, key);
    } else if (key > node->key) {
        node->right = insert(node->right, key);
    } else {
        return node; // Duplicate keys not allowed
    }
    
    // Update height
    node->height = 1 + (getHeight(node->left) > getHeight(node->right) ? 
                        getHeight(node->left) : getHeight(node->right));
    
    // Get balance factor
    int balance = getBalance(node);
    
    // Left Left Case
    if (balance > 1 && key < node->left->key) {
        return rightRotate(node);
    }
    
    // Right Right Case
    if (balance < -1 && key > node->right->key) {
        return leftRotate(node);
    }
    
    // Left Right Case
    if (balance > 1 && key > node->left->key) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    
    // Right Left Case
    if (balance < -1 && key < node->right->key) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }
    
    return node;
}

// Search for a key
AVLNode* search(AVLNode* root, int key) {
    if (root == NULL || root->key == key) {
        return root;
    }
    
    if (key < root->key) {
        return search(root->left, key);
    }
    
    return search(root->right, key);
}

// Count nodes
int countNodes(AVLNode* root) {
    if (root == NULL) return 0;
    return 1 + countNodes(root->left) + countNodes(root->right);
}

// Check if tree is balanced
int isBalanced(AVLNode* root) {
    if (root == NULL) return 1;
    
    int balance = getBalance(root);
    if (balance > 1 || balance < -1) return 0;
    
    return isBalanced(root->left) && isBalanced(root->right);
}

// Check if tree is valid BST
int isValidBST(AVLNode* root, int min, int max) {
    if (root == NULL) return 1;
    
    if (root->key <= min || root->key >= max) return 0;
    
    return isValidBST(root->left, min, root->key) && 
           isValidBST(root->right, root->key, max);
}

// KLEE test function
int main() {
    AVLNode* root = NULL;
    
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
    root = insert(root, val1);
    klee_assert(root != NULL);
    klee_assert(countNodes(root) == 1);
    klee_assert(isBalanced(root));
    klee_assert(isValidBST(root, -1, 101));
    
    root = insert(root, val2);
    klee_assert(countNodes(root) == 2);
    klee_assert(isBalanced(root));
    klee_assert(isValidBST(root, -1, 101));
    
    root = insert(root, val3);
    klee_assert(countNodes(root) == 3);
    klee_assert(isBalanced(root));
    klee_assert(isValidBST(root, -1, 101));
    
    root = insert(root, val4);
    klee_assert(countNodes(root) == 4);
    klee_assert(isBalanced(root));
    klee_assert(isValidBST(root, -1, 101));
    
    // Test search
    AVLNode* found = search(root, search_val);
    if (search_val == val1 || search_val == val2 || 
        search_val == val3 || search_val == val4) {
        klee_assert(found != NULL);
        klee_assert(found->key == search_val);
    } else {
        klee_assert(found == NULL);
    }
    
    // Test AVL properties
    for (AVLNode* node = root; node != NULL; node = node->left) {
        int balance = getBalance(node);
        klee_assert(balance >= -1 && balance <= 1);
    }
    
    // Test height consistency
    for (AVLNode* node = root; node != NULL; node = node->right) {
        int left_height = getHeight(node->left);
        int right_height = getHeight(node->right);
        int expected_height = 1 + (left_height > right_height ? left_height : right_height);
        klee_assert(node->height == expected_height);
    }
    
    return 0;
} 