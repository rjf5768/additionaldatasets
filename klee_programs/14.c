#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#define MAX_WORDS 10
#define MAX_WORD_LENGTH 20

// Word structure
struct word {
    char text[MAX_WORD_LENGTH];
    int length;
    int id;
};

// Initialize word
void init_word(struct word* w, const char* text, int id) {
    strncpy(w->text, text, MAX_WORD_LENGTH - 1);
    w->text[MAX_WORD_LENGTH - 1] = '\0';
    w->length = strlen(w->text);
    w->id = id;
}

// Compare two words alphabetically
int compare_words(struct word* w1, struct word* w2) {
    return strcmp(w1->text, w2->text);
}

// Bubble sort for words
void bubble_sort_words(struct word* words, int num_words) {
    for (int i = 0; i < num_words - 1; i++) {
        for (int j = 0; j < num_words - i - 1; j++) {
            if (compare_words(&words[j], &words[j + 1]) > 0) {
                // Swap words
                struct word temp = words[j];
                words[j] = words[j + 1];
                words[j + 1] = temp;
            }
        }
    }
}

// Quick sort for words
void quick_sort_words(struct word* words, int low, int high) {
    if (low < high) {
        // Partition
        struct word pivot = words[high];
        int i = low - 1;
        
        for (int j = low; j < high; j++) {
            if (compare_words(&words[j], &pivot) <= 0) {
                i++;
                // Swap words
                struct word temp = words[i];
                words[i] = words[j];
                words[j] = temp;
            }
        }
        
        // Swap pivot
        struct word temp = words[i + 1];
        words[i + 1] = words[high];
        words[high] = temp;
        
        int pi = i + 1;
        
        // Recursive calls
        quick_sort_words(words, low, pi - 1);
        quick_sort_words(words, pi + 1, high);
    }
}

// Check if array is sorted alphabetically
int is_sorted_alphabetically(struct word* words, int num_words) {
    for (int i = 1; i < num_words; i++) {
        if (compare_words(&words[i-1], &words[i]) > 0) {
            return 0;
        }
    }
    return 1;
}

// Count words with specific length
int count_words_with_length(struct word* words, int num_words, int length) {
    int count = 0;
    for (int i = 0; i < num_words; i++) {
        if (words[i].length == length) {
            count++;
        }
    }
    return count;
}

// Find word by text
int find_word(struct word* words, int num_words, const char* text) {
    for (int i = 0; i < num_words; i++) {
        if (strcmp(words[i].text, text) == 0) {
            return i;
        }
    }
    return -1;
}

// Generate random word
void generate_random_word(char* word, int max_length) {
    int length = (rand() % (max_length - 1)) + 1;
    for (int i = 0; i < length; i++) {
        word[i] = 'a' + (rand() % 26);
    }
    word[length] = '\0';
}

// KLEE test function
int main() {
    // Symbolic parameters
    int num_words;
    klee_make_symbolic(&num_words, sizeof(num_words), "num_words");
    
    // Constrain parameters
    klee_assume(num_words >= 2 && num_words <= MAX_WORDS);
    
    // Allocate word array
    struct word* words = malloc(num_words * sizeof(struct word));
    
    // Initialize words with symbolic data
    for (int i = 0; i < num_words; i++) {
        char word_text[MAX_WORD_LENGTH];
        klee_make_symbolic(word_text, sizeof(word_text), "word_text");
        
        // Constrain word to be valid lowercase letters
        for (int j = 0; j < MAX_WORD_LENGTH - 1; j++) {
            klee_assume(word_text[j] >= 'a' && word_text[j] <= 'z' || word_text[j] == '\0');
        }
        word_text[MAX_WORD_LENGTH - 1] = '\0';
        
        // Ensure word is not empty
        klee_assume(strlen(word_text) > 0);
        
        init_word(&words[i], word_text, i);
    }
    
    // Test bubble sort
    struct word* bubble_words = malloc(num_words * sizeof(struct word));
    for (int i = 0; i < num_words; i++) {
        bubble_words[i] = words[i];
    }
    
    bubble_sort_words(bubble_words, num_words);
    
    // Verify bubble sort results
    klee_assert(is_sorted_alphabetically(bubble_words, num_words));
    
    // Test quick sort
    struct word* quick_words = malloc(num_words * sizeof(struct word));
    for (int i = 0; i < num_words; i++) {
        quick_words[i] = words[i];
    }
    
    quick_sort_words(quick_words, 0, num_words - 1);
    
    // Verify quick sort results
    klee_assert(is_sorted_alphabetically(quick_words, num_words));
    
    // Both sorting algorithms should produce the same result
    for (int i = 0; i < num_words; i++) {
        klee_assert(strcmp(bubble_words[i].text, quick_words[i].text) == 0);
    }
    
    // Test word properties
    for (int i = 0; i < num_words; i++) {
        // Word length should be valid
        klee_assert(words[i].length > 0 && words[i].length < MAX_WORD_LENGTH);
        
        // Word ID should be preserved
        klee_assert(words[i].id == i);
        
        // Word text should be null-terminated
        klee_assert(words[i].text[words[i].length] == '\0');
    }
    
    // Test word counting
    for (int len = 1; len <= 5; len++) {
        int count = count_words_with_length(words, num_words, len);
        klee_assert(count >= 0 && count <= num_words);
    }
    
    // Test word finding
    for (int i = 0; i < num_words; i++) {
        int found_index = find_word(words, num_words, words[i].text);
        klee_assert(found_index >= 0 && found_index < num_words);
        klee_assert(strcmp(words[found_index].text, words[i].text) == 0);
    }
    
    // Test with symbolic query word
    char query_word[MAX_WORD_LENGTH];
    klee_make_symbolic(query_word, sizeof(query_word), "query_word");
    
    // Constrain query word
    for (int j = 0; j < MAX_WORD_LENGTH - 1; j++) {
        klee_assume(query_word[j] >= 'a' && query_word[j] <= 'z' || query_word[j] == '\0');
    }
    query_word[MAX_WORD_LENGTH - 1] = '\0';
    
    int query_found = find_word(words, num_words, query_word);
    if (query_found >= 0) {
        klee_assert(query_found >= 0 && query_found < num_words);
        klee_assert(strcmp(words[query_found].text, query_word) == 0);
    }
    
    // Test sorting stability (if words are equal, order should be preserved)
    // Create duplicate words
    if (num_words >= 2) {
        strcpy(words[1].text, words[0].text);
        words[1].length = words[0].length;
        
        quick_sort_words(words, 0, num_words - 1);
        
        // After sorting, equal words should be adjacent
        int found_equal = 0;
        for (int i = 1; i < num_words; i++) {
            if (strcmp(words[i].text, words[i-1].text) == 0) {
                found_equal = 1;
                break;
            }
        }
        klee_assert(found_equal == 1);
    }
    
    free(words);
    free(bubble_words);
    free(quick_words);
    return 0;
} 