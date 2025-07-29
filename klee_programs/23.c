#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <klee/klee.h>

#define N 9
#define N2 3

typedef struct {
    int grid[N][N];
} Sudoku;

// Check if a number can be placed in the given position
bool is_valid(Sudoku* s, int row, int col, int num) {
    // Check row
    for (int x = 0; x < N; x++) {
        if (s->grid[row][x] == num) {
            return false;
        }
    }
    
    // Check column
    for (int x = 0; x < N; x++) {
        if (s->grid[x][col] == num) {
            return false;
        }
    }
    
    // Check 3x3 box
    int start_row = row - row % N2;
    int start_col = col - col % N2;
    for (int i = 0; i < N2; i++) {
        for (int j = 0; j < N2; j++) {
            if (s->grid[i + start_row][j + start_col] == num) {
                return false;
            }
        }
    }
    
    return true;
}

// Find empty cell
bool find_empty(Sudoku* s, int* row, int* col) {
    for (*row = 0; *row < N; (*row)++) {
        for (*col = 0; *col < N; (*col)++) {
            if (s->grid[*row][*col] == 0) {
                return true;
            }
        }
    }
    return false;
}

// Solve Sudoku using backtracking
bool solve_sudoku(Sudoku* s) {
    int row, col;
    
    if (!find_empty(s, &row, &col)) {
        return true; // No empty cell found, puzzle solved
    }
    
    for (int num = 1; num <= N; num++) {
        if (is_valid(s, row, col, num)) {
            s->grid[row][col] = num;
            
            if (solve_sudoku(s)) {
                return true;
            }
            
            s->grid[row][col] = 0; // Backtrack
        }
    }
    
    return false;
}

// Check if Sudoku is valid (no duplicates in rows, columns, boxes)
bool is_valid_sudoku(Sudoku* s) {
    // Check rows
    for (int i = 0; i < N; i++) {
        bool used[N + 1] = {false};
        for (int j = 0; j < N; j++) {
            if (s->grid[i][j] != 0) {
                if (used[s->grid[i][j]]) {
                    return false;
                }
                used[s->grid[i][j]] = true;
            }
        }
    }
    
    // Check columns
    for (int j = 0; j < N; j++) {
        bool used[N + 1] = {false};
        for (int i = 0; i < N; i++) {
            if (s->grid[i][j] != 0) {
                if (used[s->grid[i][j]]) {
                    return false;
                }
                used[s->grid[i][j]] = true;
            }
        }
    }
    
    // Check 3x3 boxes
    for (int box = 0; box < N; box++) {
        bool used[N + 1] = {false};
        int start_row = (box / N2) * N2;
        int start_col = (box % N2) * N2;
        for (int i = 0; i < N2; i++) {
            for (int j = 0; j < N2; j++) {
                if (s->grid[start_row + i][start_col + j] != 0) {
                    if (used[s->grid[start_row + i][start_col + j]]) {
                        return false;
                    }
                    used[s->grid[start_row + i][start_col + j]] = true;
                }
            }
        }
    }
    
    return true;
}

// Count empty cells
int count_empty_cells(Sudoku* s) {
    int count = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (s->grid[i][j] == 0) {
                count++;
            }
        }
    }
    return count;
}

// Check if Sudoku is complete
bool is_complete(Sudoku* s) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (s->grid[i][j] == 0) {
                return false;
            }
        }
    }
    return true;
}

// KLEE test function
int main() {
    Sudoku sudoku;
    
    // Initialize with symbolic values
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            klee_make_symbolic(&sudoku.grid[i][j], sizeof(int), "cell");
            // Constrain values to valid Sudoku range (0-9)
            klee_assume(sudoku.grid[i][j] >= 0 && sudoku.grid[i][j] <= 9);
        }
    }
    
    // Test initial validity
    klee_assert(is_valid_sudoku(&sudoku));
    
    // Count initial empty cells
    int initial_empty = count_empty_cells(&sudoku);
    klee_assert(initial_empty >= 0 && initial_empty <= N * N);
    
    // Try to solve the Sudoku
    bool solved = solve_sudoku(&sudoku);
    
    if (solved) {
        // If solved, check that it's complete and valid
        klee_assert(is_complete(&sudoku));
        klee_assert(is_valid_sudoku(&sudoku));
        
        // Verify no empty cells remain
        int final_empty = count_empty_cells(&sudoku);
        klee_assert(final_empty == 0);
        
        // Verify all numbers are in valid range
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                klee_assert(sudoku.grid[i][j] >= 1 && sudoku.grid[i][j] <= 9);
            }
        }
    } else {
        // If not solved, check that it's still valid
        klee_assert(is_valid_sudoku(&sudoku));
    }
    
    // Test edge cases
    Sudoku empty_sudoku;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            empty_sudoku.grid[i][j] = 0;
        }
    }
    
    klee_assert(is_valid_sudoku(&empty_sudoku));
    klee_assert(count_empty_cells(&empty_sudoku) == N * N);
    klee_assert(!is_complete(&empty_sudoku));
    
    // Test with a simple valid Sudoku
    Sudoku simple_sudoku;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            simple_sudoku.grid[i][j] = (i + j) % 9 + 1;
        }
    }
    
    klee_assert(is_valid_sudoku(&simple_sudoku));
    klee_assert(is_complete(&simple_sudoku));
    klee_assert(count_empty_cells(&simple_sudoku) == 0);
    
    return 0;
} 