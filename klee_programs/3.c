#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

// Simplified AST node for KLEE testing
struct ASTNode {
    char content;
    struct ASTNode* left;
    struct ASTNode* right;
};

// Simplified NFA state
struct NFAState {
    int id;
    int isAccepting;
    char transition_char;
    struct NFAState* next;
};

// Simplified NFA
struct NFA {
    struct NFAState* start;
    struct NFAState* accept;
    int state_count;
};

// Create AST node
struct ASTNode* createNode(char content) {
    struct ASTNode* node = malloc(sizeof(struct ASTNode));
    node->content = content;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// Create NFA state
struct NFAState* createState(int id, int isAccepting) {
    struct NFAState* state = malloc(sizeof(struct NFAState));
    state->id = id;
    state->isAccepting = isAccepting;
    state->transition_char = '\0';
    state->next = NULL;
    return state;
}

// Create simple NFA for character
struct NFA* createCharNFA(char c) {
    struct NFA* nfa = malloc(sizeof(struct NFA));
    nfa->start = createState(0, 0);
    nfa->accept = createState(1, 1);
    nfa->start->transition_char = c;
    nfa->start->next = nfa->accept;
    nfa->state_count = 2;
    return nfa;
}

// Create concatenation NFA
struct NFA* createConcatNFA(struct NFA* nfa1, struct NFA* nfa2) {
    struct NFA* nfa = malloc(sizeof(struct NFA));
    nfa->start = nfa1->start;
    nfa->accept = nfa2->accept;
    
    // Connect nfa1's accept to nfa2's start
    nfa1->accept->isAccepting = 0;
    nfa1->accept->next = nfa2->start;
    
    nfa->state_count = nfa1->state_count + nfa2->state_count;
    return nfa;
}

// Create union NFA
struct NFA* createUnionNFA(struct NFA* nfa1, struct NFA* nfa2) {
    struct NFA* nfa = malloc(sizeof(struct NFA));
    nfa->start = createState(-1, 0);  // New start state
    nfa->accept = createState(-2, 1); // New accept state
    
    // Connect start to both NFAs
    nfa->start->next = nfa1->start;
    nfa->start->transition_char = 'e'; // epsilon transition
    
    // Connect both NFAs to accept
    nfa1->accept->next = nfa->accept;
    nfa2->accept->next = nfa->accept;
    
    nfa->state_count = nfa1->state_count + nfa2->state_count + 2;
    return nfa;
}

// Simulate NFA execution
int simulateNFA(struct NFA* nfa, const char* input) {
    struct NFAState* current = nfa->start;
    int input_len = strlen(input);
    
    for (int i = 0; i < input_len; i++) {
        if (current == NULL) return 0;
        
        if (current->transition_char == input[i] || current->transition_char == 'e') {
            current = current->next;
        } else {
            return 0;
        }
    }
    
    return (current != NULL && current->isAccepting);
}

// Build simple AST from string
struct ASTNode* buildSimpleAST(const char* input) {
    int len = strlen(input);
    if (len == 0) return NULL;
    
    if (len == 1) {
        return createNode(input[0]);
    }
    
    // Simple concatenation for KLEE testing
    struct ASTNode* root = createNode('.');
    root->left = createNode(input[0]);
    root->right = buildSimpleAST(input + 1);
    return root;
}

// Compile AST to NFA
struct NFA* compileAST(struct ASTNode* root) {
    if (root == NULL) return NULL;
    
    if (root->content != '.' && root->content != '|' && root->content != '*') {
        return createCharNFA(root->content);
    }
    
    if (root->content == '.') {
        struct NFA* left = compileAST(root->left);
        struct NFA* right = compileAST(root->right);
        return createConcatNFA(left, right);
    }
    
    if (root->content == '|') {
        struct NFA* left = compileAST(root->left);
        struct NFA* right = compileAST(root->right);
        return createUnionNFA(left, right);
    }
    
    return NULL; // Simplified for KLEE
}

// KLEE test function
int main() {
    // Symbolic input string
    char input[10];
    klee_make_symbolic(input, sizeof(input), "input");
    
    // Constrain input to reasonable length and ASCII characters
    klee_assume(strlen(input) <= 5);
    for (int i = 0; i < 5; i++) {
        klee_assume(input[i] >= 'a' && input[i] <= 'z' || input[i] == '\0');
    }
    
    // Build AST from input
    struct ASTNode* ast = buildSimpleAST(input);
    klee_assert(ast != NULL);
    
    // Compile to NFA
    struct NFA* nfa = compileAST(ast);
    klee_assert(nfa != NULL);
    klee_assert(nfa->start != NULL);
    klee_assert(nfa->accept != NULL);
    
    // Test with symbolic test string
    char test_string[10];
    klee_make_symbolic(test_string, sizeof(test_string), "test_string");
    
    klee_assume(strlen(test_string) <= 5);
    for (int i = 0; i < 5; i++) {
        klee_assume(test_string[i] >= 'a' && test_string[i] <= 'z' || test_string[i] == '\0');
    }
    
    // Simulate NFA
    int result = simulateNFA(nfa, test_string);
    
    // Add assertions for KLEE exploration
    if (strcmp(input, test_string) == 0) {
        klee_assert(result == 1);  // Should accept matching string
    }
    
    if (strlen(test_string) > strlen(input)) {
        klee_assert(result == 0);  // Should reject longer strings
    }
    
    return 0;
} 