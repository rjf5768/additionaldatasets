#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

typedef struct BSTNode {
    int key;
    struct BSTNode *left;
    struct BSTNode *right;
    int height;
} BSTNode;

// Create new node
BSTNode* createNode(int key) {
    BSTNode* node = (BSTNode*)malloc(sizeof(BSTNode));
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    return node;
}

// Get height of node
int getHeight(BSTNode* node) {
    if (node == NULL) return 0;
    return node->height;
}

// Get balance factor
int getBalance(BSTNode* node) {
    if (node == NULL) return 0;
    return getHeight(node->left) - getHeight(node->right);
}

// Right rotate
BSTNode* rightRotate(BSTNode* y) {
    BSTNode* x = y->left;
    BSTNode* T2 = x->right;
    
    x->right = y;
    y->left = T2;
    
    y->height = 1 + (getHeight(y->left) > getHeight(y->right) ? 
                     getHeight(y->left) : getHeight(y->right));
    x->height = 1 + (getHeight(x->left) > getHeight(x->right) ? 
                     getHeight(x->left) : getHeight(x->right));
    
    return x;
}

// Left rotate
BSTNode* leftRotate(BSTNode* x) {
    BSTNode* y = x->right;
    BSTNode* T2 = y->left;
    
    y->left = x;
    x->right = T2;
    
    x->height = 1 + (getHeight(x->left) > getHeight(x->right) ? 
                     getHeight(x->left) : getHeight(x->right));
    y->height = 1 + (getHeight(y->left) > getHeight(y->right) ? 
                     getHeight(y->left) : getHeight(y->right));
    
    return y;
}

// Insert node
BSTNode* insert(BSTNode* node, int key) {
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
BSTNode* search(BSTNode* root, int key) {
    if (root == NULL || root->key == key) {
        return root;
    }
    
    if (key < root->key) {
        return search(root->left, key);
    }
    
    return search(root->right, key);
}

// Find minimum value node
BSTNode* findMin(BSTNode* node) {
    BSTNode* current = node;
    while (current && current->left != NULL) {
        current = current->left;
    }
    return current;
}

// Delete node
BSTNode* deleteNode(BSTNode* root, int key) {
    if (root == NULL) {
        return root;
    }
    
    if (key < root->key) {
        root->left = deleteNode(root->left, key);
    } else if (key > root->key) {
        root->right = deleteNode(root->right, key);
    } else {
        // Node with only one child or no child
        if (root->left == NULL) {
            BSTNode* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            BSTNode* temp = root->left;
            free(root);
            return temp;
        }
        
        // Node with two children
        BSTNode* temp = findMin(root->right);
        root->key = temp->key;
        root->right = deleteNode(root->right, temp->key);
    }
    
    if (root == NULL) {
        return root;
    }
    
    // Update height
    root->height = 1 + (getHeight(root->left) > getHeight(root->right) ? 
                        getHeight(root->left) : getHeight(root->right));
    
    // Get balance factor
    int balance = getBalance(root);
    
    // Left Left Case
    if (balance > 1 && getBalance(root->left) >= 0) {
        return rightRotate(root);
    }
    
    // Left Right Case
    if (balance > 1 && getBalance(root->left) < 0) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }
    
    // Right Right Case
    if (balance < -1 && getBalance(root->right) <= 0) {
        return leftRotate(root);
    }
    
    // Right Left Case
    if (balance < -1 && getBalance(root->right) > 0) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }
    
    return root;
}

// Count nodes
int countNodes(BSTNode* root) {
    if (root == NULL) return 0;
    return 1 + countNodes(root->left) + countNodes(root->right);
}

// Check if tree is valid BST
int isValidBST(BSTNode* root, int min, int max) {
    if (root == NULL) return 1;
    
    if (root->key <= min || root->key >= max) return 0;
    
    return isValidBST(root->left, min, root->key) && 
           isValidBST(root->right, root->key, max);
}

// Check if tree is balanced
int isBalanced(BSTNode* root) {
    if (root == NULL) return 1;
    
    int balance = getBalance(root);
    if (balance > 1 || balance < -1) return 0;
    
    return isBalanced(root->left) && isBalanced(root->right);
}

// Inorder traversal
void inorderTraversal(BSTNode* root, int* arr, int* index) {
    if (root != NULL) {
        inorderTraversal(root->left, arr, index);
        arr[*index] = root->key;
        (*index)++;
        inorderTraversal(root->right, arr, index);
    }
}

// KLEE test function
int main() {
    BSTNode* root = NULL;
    
    // Symbolic inputs
    int val1, val2, val3, val4, search_val, delete_val;
    klee_make_symbolic(&val1, sizeof(val1), "val1");
    klee_make_symbolic(&val2, sizeof(val2), "val2");
    klee_make_symbolic(&val3, sizeof(val3), "val3");
    klee_make_symbolic(&val4, sizeof(val4), "val4");
    klee_make_symbolic(&search_val, sizeof(search_val), "search_val");
    klee_make_symbolic(&delete_val, sizeof(delete_val), "delete_val");
    
    // Constrain inputs
    klee_assume(val1 >= 0 && val1 <= 100);
    klee_assume(val2 >= 0 && val2 <= 100);
    klee_assume(val3 >= 0 && val3 <= 100);
    klee_assume(val4 >= 0 && val4 <= 100);
    klee_assume(search_val >= 0 && search_val <= 100);
    klee_assume(delete_val >= 0 && delete_val <= 100);
    
    // Ensure different values
    klee_assume(val1 != val2 && val2 != val3 && val3 != val4);
    klee_assume(val1 != val3 && val1 != val4 && val2 != val4);
    
    // Insert nodes
    root = insert(root, val1);
    klee_assert(root != NULL);
    klee_assert(countNodes(root) == 1);
    klee_assert(isValidBST(root, -1, 101));
    klee_assert(isBalanced(root));
    
    root = insert(root, val2);
    klee_assert(countNodes(root) == 2);
    klee_assert(isValidBST(root, -1, 101));
    klee_assert(isBalanced(root));
    
    root = insert(root, val3);
    klee_assert(countNodes(root) == 3);
    klee_assert(isValidBST(root, -1, 101));
    klee_assert(isBalanced(root));
    
    root = insert(root, val4);
    klee_assert(countNodes(root) == 4);
    klee_assert(isValidBST(root, -1, 101));
    klee_assert(isBalanced(root));
    
    // Test search
    BSTNode* found = search(root, search_val);
    if (search_val == val1 || search_val == val2 || 
        search_val == val3 || search_val == val4) {
        klee_assert(found != NULL);
        klee_assert(found->key == search_val);
    } else {
        klee_assert(found == NULL);
    }
    
    // Test inorder traversal (should be sorted)
    int traversal[4];
    int index = 0;
    inorderTraversal(root, traversal, &index);
    klee_assert(index == 4);
    
    // Verify traversal is sorted
    for (int i = 1; i < 4; i++) {
        klee_assert(traversal[i] >= traversal[i-1]);
    }
    
    // Test deletion
    if (delete_val == val1 || delete_val == val2 || 
        delete_val == val3 || delete_val == val4) {
        int nodes_before = countNodes(root);
        root = deleteNode(root, delete_val);
        int nodes_after = countNodes(root);
        klee_assert(nodes_after == nodes_before - 1);
        klee_assert(isValidBST(root, -1, 101));
        klee_assert(isBalanced(root));
    }
    
    // Test tree properties after operations
    klee_assert(root != NULL);
    klee_assert(countNodes(root) >= 1);
    klee_assert(isValidBST(root, -1, 101));
    klee_assert(isBalanced(root));
    
    // Test height consistency
    for (BSTNode* node = root; node != NULL; node = node->left) {
        int left_height = getHeight(node->left);
        int right_height = getHeight(node->right);
        int expected_height = 1 + (left_height > right_height ? left_height : right_height);
        klee_assert(node->height == expected_height);
    }
    
    return 0;
} 