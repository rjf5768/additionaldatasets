#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

// Simplified naval battle game for KLEE testing
#define BOARD_SIZE 5  // Smaller board for KLEE analysis

int board[BOARD_SIZE][BOARD_SIZE];
int ships_remaining = 3;  // 3 ships of size 1

// Initialize board
void initBoard() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = 0;  // 0 = empty, 1 = ship, 2 = hit, 3 = miss
        }
    }
}

// Place ships (simplified for KLEE)
void placeShips() {
    // Place 3 ships at fixed positions for testing
    board[0][0] = 1;  // Ship 1
    board[2][2] = 1;  // Ship 2  
    board[4][4] = 1;  // Ship 3
}

// Check if position is valid
int isValidPosition(int row, int col) {
    return (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE);
}

// Make a shot
int makeShot(int row, int col) {
    if (!isValidPosition(row, col)) {
        return -1;  // Invalid position
    }
    
    if (board[row][col] == 1) {
        board[row][col] = 2;  // Hit
        ships_remaining--;
        return 1;  // Hit
    } else if (board[row][col] == 0) {
        board[row][col] = 3;  // Miss
        return 0;  // Miss
    } else {
        return -2;  // Already shot here
    }
}

// Check if game is over
int isGameOver() {
    return ships_remaining == 0;
}

// KLEE test function
int main() {
    initBoard();
    placeShips();
    
    // Symbolic inputs for shots
    int shot_row, shot_col;
    klee_make_symbolic(&shot_row, sizeof(shot_row), "shot_row");
    klee_make_symbolic(&shot_col, sizeof(shot_col), "shot_col");
    
    // Constrain inputs to valid board positions
    klee_assume(shot_row >= 0 && shot_row < BOARD_SIZE);
    klee_assume(shot_col >= 0 && shot_col < BOARD_SIZE);
    
    // Make the shot
    int result = makeShot(shot_row, shot_col);
    
    // Add assertions for KLEE to explore
    if (result == 1) {
        klee_assert(ships_remaining < 3);  // Hit should reduce ships
    }
    
    if (result == 0) {
        klee_assert(ships_remaining == 3);  // Miss shouldn't affect ships
    }
    
    // Test multiple shots
    int shot2_row, shot2_col;
    klee_make_symbolic(&shot2_row, sizeof(shot2_row), "shot2_row");
    klee_make_symbolic(&shot2_col, sizeof(shot2_col), "shot2_col");
    
    klee_assume(shot2_row >= 0 && shot2_row < BOARD_SIZE);
    klee_assume(shot2_col >= 0 && shot2_col < BOARD_SIZE);
    
    // Ensure different positions
    klee_assume(shot2_row != shot_row || shot2_col != shot_col);
    
    int result2 = makeShot(shot2_row, shot2_col);
    
    // Test game state after two shots
    if (result == 1 && result2 == 1) {
        klee_assert(ships_remaining <= 1);
    }
    
    return 0;
} 