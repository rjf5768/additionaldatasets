#include <klee/klee.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

enum {LEFT, UP, DIAG};

void lcslen(const char *s1, const char *s2, int l1, int l2, int **L, int **B) {
    int i, j;

    for (i = 1; i <= l1; ++i) {
        for (j = 1; j <= l2; ++j) {
            if (s1[i-1] == s2[j-1]) {
                L[i][j] = 1 + L[i-1][j-1];
                B[i][j] = DIAG;
            }
            else if (L[i-1][j] < L[i][j-1]) {
                L[i][j] = L[i][j-1];
                B[i][j] = LEFT;
            }
            else {
                L[i][j] = L[i-1][j];
                B[i][j] = UP;
            }
        }
    }
}

char *lcsbuild(const char *s1, int l1, int l2, int **L, int **B) {
    int i, j, lcsl;
    char *lcs;
    lcsl = L[l1][l2];
    
    lcs = (char *)calloc(lcsl+1, sizeof(char));
    if (!lcs) {
        return NULL;
    }

    i = l1, j = l2;
    while (i > 0 && j > 0) {
        if (B[i][j] == DIAG) {
            lcs[--lcsl] = s1[i-1];
            i = i - 1;
            j = j - 1;
        }
        else if (B[i][j] == LEFT) {
            j = j - 1;
        }
        else {
            i = i - 1;
        }
    }
    return lcs;
}

int main() {
    char s1[10], s2[10];
    
    // Make inputs symbolic
    klee_make_symbolic(s1, sizeof(s1), "s1");
    klee_make_symbolic(s2, sizeof(s2), "s2");
    
    // Ensure strings are null-terminated
    s1[9] = '\0';
    s2[9] = '\0';
    
    // Constrain strings to printable ASCII
    for (int i = 0; i < 9; i++) {
        klee_assume(s1[i] >= 32 && s1[i] <= 126);
        klee_assume(s2[i] >= 32 && s2[i] <= 126);
    }
    
    int l1 = strlen(s1);
    int l2 = strlen(s2);
    
    // Constrain lengths to reasonable values
    klee_assume(l1 > 0 && l1 <= 9);
    klee_assume(l2 > 0 && l2 <= 9);
    
    int **L = (int **)calloc(l1+1, sizeof(int *));
    int **B = (int **)calloc(l1+1, sizeof(int *));
    
    if (!L || !B) {
        return 1;
    }
    
    for (int j = 0; j <= l1; j++) {
        L[j] = (int *)calloc(l2+1, sizeof(int));
        B[j] = (int *)calloc(l2+1, sizeof(int));
        if (!L[j] || !B[j]) {
            return 1;
        }
    }
    
    lcslen(s1, s2, l1, l2, L, B);
    char *lcs = lcsbuild(s1, l1, l2, L, B);
    
    // Verify LCS properties
    klee_assert(lcs != NULL);
    klee_assert(L[l1][l2] >= 0);
    klee_assert(L[l1][l2] <= l1 && L[l1][l2] <= l2);
    
    // Verify LCS length matches the matrix value
    klee_assert(strlen(lcs) == L[l1][l2]);
    
    // Verify LCS is a subsequence of both strings
    int lcs_len = strlen(lcs);
    if (lcs_len > 0) {
        // Check that LCS characters appear in both strings in order
        int pos1 = 0, pos2 = 0;
        for (int i = 0; i < lcs_len; i++) {
            // Find lcs[i] in s1 after pos1
            while (pos1 < l1 && s1[pos1] != lcs[i]) pos1++;
            // Find lcs[i] in s2 after pos2
            while (pos2 < l2 && s2[pos2] != lcs[i]) pos2++;
            
            klee_assert(pos1 < l1 && pos2 < l2);
            pos1++;
            pos2++;
        }
    }
    
    free(lcs);
    for (int j = 0; j <= l1; j++) {
        free(L[j]);
        free(B[j]);
    }
    free(L);
    free(B);
    
    return 0;
} 