#include <stdio.h>
#include <stdlib.h>
#include <klee/klee.h>

#define BOARD_SIZE 3
#define EMPTY 0
#define X_PLAYER 1
#define O_PLAYER 2

// Game board
int board[BOARD_SIZE][BOARD_SIZE];
int current_player = X_PLAYER;

// Initialize board
void init_board() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = EMPTY;
        }
    }
}

// Check if position is valid
int is_valid_move(int row, int col) {
    return (row >= 0 && row < BOARD_SIZE && 
            col >= 0 && col < BOARD_SIZE && 
            board[row][col] == EMPTY);
}

// Make a move
int make_move(int row, int col) {
    if (!is_valid_move(row, col)) {
        return 0;  // Invalid move
    }
    
    board[row][col] = current_player;
    return 1;  // Valid move
}

// Check for win condition
int check_win(int player) {
    // Check rows
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (board[i][0] == player && board[i][1] == player && board[i][2] == player) {
            return 1;
        }
    }
    
    // Check columns
    for (int j = 0; j < BOARD_SIZE; j++) {
        if (board[0][j] == player && board[1][j] == player && board[2][j] == player) {
            return 1;
        }
    }
    
    // Check diagonals
    if (board[0][0] == player && board[1][1] == player && board[2][2] == player) {
        return 1;
    }
    if (board[0][2] == player && board[1][1] == player && board[2][0] == player) {
        return 1;
    }
    
    return 0;
}

// Check if board is full
int is_board_full() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == EMPTY) {
                return 0;
            }
        }
    }
    return 1;
}

// Switch player
void switch_player() {
    current_player = (current_player == X_PLAYER) ? O_PLAYER : X_PLAYER;
}

// Count pieces for a player
int count_pieces(int player) {
    int count = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == player) {
                count++;
            }
        }
    }
    return count;
}

// KLEE test function
int main() {
    init_board();
    
    // Symbolic inputs for moves
    int move1_row, move1_col, move2_row, move2_col, move3_row, move3_col;
    klee_make_symbolic(&move1_row, sizeof(move1_row), "move1_row");
    klee_make_symbolic(&move1_col, sizeof(move1_col), "move1_col");
    klee_make_symbolic(&move2_row, sizeof(move2_row), "move2_row");
    klee_make_symbolic(&move2_col, sizeof(move2_col), "move2_col");
    klee_make_symbolic(&move3_row, sizeof(move3_row), "move3_row");
    klee_make_symbolic(&move3_col, sizeof(move3_col), "move3_col");
    
    // Constrain moves to valid board positions
    klee_assume(move1_row >= 0 && move1_row < BOARD_SIZE);
    klee_assume(move1_col >= 0 && move1_col < BOARD_SIZE);
    klee_assume(move2_row >= 0 && move2_row < BOARD_SIZE);
    klee_assume(move2_col >= 0 && move2_col < BOARD_SIZE);
    klee_assume(move3_row >= 0 && move3_row < BOARD_SIZE);
    klee_assume(move3_col >= 0 && move3_col < BOARD_SIZE);
    
    // Ensure different positions
    klee_assume(move1_row != move2_row || move1_col != move2_col);
    klee_assume(move1_row != move3_row || move1_col != move3_col);
    klee_assume(move2_row != move3_row || move2_col != move3_col);
    
    // Make moves
    int result1 = make_move(move1_row, move1_col);
    if (result1) {
        klee_assert(board[move1_row][move1_col] == X_PLAYER);
        klee_assert(count_pieces(X_PLAYER) == 1);
        klee_assert(count_pieces(O_PLAYER) == 0);
        
        switch_player();
        
        int result2 = make_move(move2_row, move2_col);
        if (result2) {
            klee_assert(board[move2_row][move2_col] == O_PLAYER);
            klee_assert(count_pieces(X_PLAYER) == 1);
            klee_assert(count_pieces(O_PLAYER) == 1);
            
            switch_player();
            
            int result3 = make_move(move3_row, move3_col);
            if (result3) {
                klee_assert(board[move3_row][move3_col] == X_PLAYER);
                klee_assert(count_pieces(X_PLAYER) == 2);
                klee_assert(count_pieces(O_PLAYER) == 1);
                
                // Check win conditions
                int x_wins = check_win(X_PLAYER);
                int o_wins = check_win(O_PLAYER);
                
                // Both players can't win simultaneously
                klee_assert(!(x_wins && o_wins));
                
                // If X wins, O shouldn't win
                if (x_wins) {
                    klee_assert(!o_wins);
                }
                
                // Test board state
                int total_pieces = count_pieces(X_PLAYER) + count_pieces(O_PLAYER);
                klee_assert(total_pieces <= BOARD_SIZE * BOARD_SIZE);
                
                // Test that valid moves are properly recorded
                if (is_valid_move(move1_row, move1_col)) {
                    klee_assert(board[move1_row][move1_col] != EMPTY);
                }
            }
        }
    }
    
    return 0;
} 