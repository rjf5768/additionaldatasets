#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <klee/klee.h>

#define ALPHABET_SIZE 26
#define MAX_WORD_LENGTH 50
#define MAX_WORDS 20

typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    bool is_end_of_word;
} TrieNode;

// Create a new trie node
TrieNode* create_trie_node() {
    TrieNode* node = (TrieNode*)malloc(sizeof(TrieNode));
    if (node) {
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            node->children[i] = NULL;
        }
        node->is_end_of_word = false;
    }
    return node;
}

// Insert a word into the trie
int insert_word(TrieNode* root, const char* word) {
    if (!root || !word) {
        return -1;
    }
    
    TrieNode* current = root;
    int len = strlen(word);
    
    for (int i = 0; i < len; i++) {
        char c = word[i];
        
        // Check if character is lowercase letter
        if (c < 'a' || c > 'z') {
            return -1; // Invalid character
        }
        
        int index = c - 'a';
        
        if (!current->children[index]) {
            current->children[index] = create_trie_node();
            if (!current->children[index]) {
                return -1; // Memory allocation failed
            }
        }
        
        current = current->children[index];
    }
    
    current->is_end_of_word = true;
    return 0;
}

// Search for a word in the trie
bool search_word(TrieNode* root, const char* word) {
    if (!root || !word) {
        return false;
    }
    
    TrieNode* current = root;
    int len = strlen(word);
    
    for (int i = 0; i < len; i++) {
        char c = word[i];
        
        // Check if character is lowercase letter
        if (c < 'a' || c > 'z') {
            return false;
        }
        
        int index = c - 'a';
        
        if (!current->children[index]) {
            return false;
        }
        
        current = current->children[index];
    }
    
    return current->is_end_of_word;
}

// Check if a word is a prefix of any word in the trie
bool starts_with(TrieNode* root, const char* prefix) {
    if (!root || !prefix) {
        return false;
    }
    
    TrieNode* current = root;
    int len = strlen(prefix);
    
    for (int i = 0; i < len; i++) {
        char c = prefix[i];
        
        // Check if character is lowercase letter
        if (c < 'a' || c > 'z') {
            return false;
        }
        
        int index = c - 'a';
        
        if (!current->children[index]) {
            return false;
        }
        
        current = current->children[index];
    }
    
    return true;
}

// Count words in trie
int count_words(TrieNode* root) {
    if (!root) {
        return 0;
    }
    
    int count = root->is_end_of_word ? 1 : 0;
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (root->children[i]) {
            count += count_words(root->children[i]);
        }
    }
    
    return count;
}

// Count nodes in trie
int count_nodes(TrieNode* root) {
    if (!root) {
        return 0;
    }
    
    int count = 1;
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (root->children[i]) {
            count += count_nodes(root->children[i]);
        }
    }
    
    return count;
}

// Check if trie is empty
bool is_empty(TrieNode* root) {
    if (!root) {
        return true;
    }
    
    if (root->is_end_of_word) {
        return false;
    }
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (root->children[i]) {
            return false;
        }
    }
    
    return true;
}

// Delete a word from trie
bool delete_word(TrieNode* root, const char* word) {
    if (!root || !word) {
        return false;
    }
    
    return delete_word_helper(root, word, 0);
}

// Helper function for deletion
bool delete_word_helper(TrieNode* root, const char* word, int depth) {
    if (!root) {
        return false;
    }
    
    if (depth == strlen(word)) {
        if (root->is_end_of_word) {
            root->is_end_of_word = false;
            return is_empty(root);
        }
        return false;
    }
    
    char c = word[depth];
    if (c < 'a' || c > 'z') {
        return false;
    }
    
    int index = c - 'a';
    if (!root->children[index]) {
        return false;
    }
    
    bool should_delete = delete_word_helper(root->children[index], word, depth + 1);
    
    if (should_delete) {
        free(root->children[index]);
        root->children[index] = NULL;
        return !root->is_end_of_word && is_empty(root);
    }
    
    return false;
}

// Free trie memory
void free_trie(TrieNode* root) {
    if (!root) {
        return;
    }
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (root->children[i]) {
            free_trie(root->children[i]);
        }
    }
    
    free(root);
}

// Check if string contains only lowercase letters
bool is_lowercase_alpha(const char* str) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (str[i] < 'a' || str[i] > 'z') {
            return false;
        }
    }
    return true;
}

// Get longest common prefix
int longest_common_prefix(TrieNode* root, const char* word) {
    if (!root || !word) {
        return 0;
    }
    
    TrieNode* current = root;
    int len = strlen(word);
    int common_len = 0;
    
    for (int i = 0; i < len; i++) {
        char c = word[i];
        
        if (c < 'a' || c > 'z') {
            break;
        }
        
        int index = c - 'a';
        
        if (!current->children[index]) {
            break;
        }
        
        current = current->children[index];
        common_len++;
    }
    
    return common_len;
}

// KLEE test function
int main() {
    char words[MAX_WORDS][MAX_WORD_LENGTH];
    char search_word_str[MAX_WORD_LENGTH];
    int num_words;
    
    // Make inputs symbolic
    klee_make_symbolic(&num_words, sizeof(int), "num_words");
    klee_assume(num_words >= 0 && num_words <= MAX_WORDS);
    
    for (int i = 0; i < num_words; i++) {
        klee_make_symbolic(words[i], sizeof(words[i]), "word");
        klee_assume(strlen(words[i]) < MAX_WORD_LENGTH);
        klee_assume(is_lowercase_alpha(words[i]));
    }
    
    klee_make_symbolic(search_word_str, sizeof(search_word_str), "search_word");
    klee_assume(strlen(search_word_str) < MAX_WORD_LENGTH);
    klee_assume(is_lowercase_alpha(search_word_str));
    
    // Create trie
    TrieNode* root = create_trie_node();
    klee_assert(root != NULL);
    
    // Test initial state
    klee_assert(is_empty(root));
    klee_assert(count_words(root) == 0);
    klee_assert(count_nodes(root) == 1);
    
    // Insert words
    for (int i = 0; i < num_words; i++) {
        int result = insert_word(root, words[i]);
        klee_assert(result == 0);
    }
    
    // Test trie properties after insertion
    klee_assert(count_words(root) == num_words);
    klee_assert(count_nodes(root) >= 1);
    
    if (num_words > 0) {
        klee_assert(!is_empty(root));
    }
    
    // Test search functionality
    for (int i = 0; i < num_words; i++) {
        bool found = search_word(root, words[i]);
        klee_assert(found);
    }
    
    // Test prefix search
    for (int i = 0; i < num_words; i++) {
        int len = strlen(words[i]);
        for (int j = 1; j <= len; j++) {
            char prefix[MAX_WORD_LENGTH];
            strncpy(prefix, words[i], j);
            prefix[j] = '\0';
            
            bool has_prefix = starts_with(root, prefix);
            klee_assert(has_prefix);
        }
    }
    
    // Test search for non-existent word
    bool found = search_word(root, search_word_str);
    // This could be true if search_word_str happens to match one of the inserted words
    
    // Test with known words
    TrieNode* test_root = create_trie_node();
    
    // Insert test words
    insert_word(test_root, "hello");
    insert_word(test_root, "world");
    insert_word(test_root, "help");
    insert_word(test_root, "helicopter");
    
    // Test search
    klee_assert(search_word(test_root, "hello"));
    klee_assert(search_word(test_root, "world"));
    klee_assert(search_word(test_root, "help"));
    klee_assert(search_word(test_root, "helicopter"));
    klee_assert(!search_word(test_root, "hell"));
    klee_assert(!search_word(test_root, "xyz"));
    
    // Test prefix search
    klee_assert(starts_with(test_root, "hel"));
    klee_assert(starts_with(test_root, "wor"));
    klee_assert(!starts_with(test_root, "xyz"));
    
    // Test word count
    klee_assert(count_words(test_root) == 4);
    
    // Test longest common prefix
    klee_assert(longest_common_prefix(test_root, "hello") == 5);
    klee_assert(longest_common_prefix(test_root, "help") == 4);
    klee_assert(longest_common_prefix(test_root, "helicopter") == 10);
    klee_assert(longest_common_prefix(test_root, "xyz") == 0);
    
    // Test deletion
    delete_word(test_root, "help");
    klee_assert(!search_word(test_root, "help"));
    klee_assert(search_word(test_root, "hello"));
    klee_assert(search_word(test_root, "helicopter"));
    klee_assert(count_words(test_root) == 3);
    
    // Test edge cases
    // Empty string
    TrieNode* empty_root = create_trie_node();
    insert_word(empty_root, "");
    klee_assert(search_word(empty_root, ""));
    klee_assert(count_words(empty_root) == 1);
    
    // Single character
    TrieNode* single_root = create_trie_node();
    insert_word(single_root, "a");
    klee_assert(search_word(single_root, "a"));
    klee_assert(!search_word(single_root, "b"));
    klee_assert(count_words(single_root) == 1);
    
    // Duplicate words
    TrieNode* dup_root = create_trie_node();
    insert_word(dup_root, "test");
    insert_word(dup_root, "test");
    klee_assert(search_word(dup_root, "test"));
    klee_assert(count_words(dup_root) == 1); // Should count as one word
    
    // Test with symbolic input
    if (num_words > 0) {
        // Test that all inserted words can be found
        for (int i = 0; i < num_words; i++) {
            klee_assert(search_word(root, words[i]));
        }
        
        // Test that prefixes of inserted words can be found
        for (int i = 0; i < num_words; i++) {
            int len = strlen(words[i]);
            if (len > 0) {
                char first_char[2] = {words[i][0], '\0'};
                klee_assert(starts_with(root, first_char));
            }
        }
    }
    
    // Clean up
    free_trie(root);
    free_trie(test_root);
    free_trie(empty_root);
    free_trie(single_root);
    free_trie(dup_root);
    
    return 0;
} 