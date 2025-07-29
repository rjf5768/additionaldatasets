#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#define MAX_WORD_LENGTH 20
#define MAX_GUESSES 6

typedef struct HangmanGame {
    char word[MAX_WORD_LENGTH];
    char guessed_letters[26];
    char display[MAX_WORD_LENGTH];
    int word_length;
    int remaining_guesses;
    int game_won;
    int game_over;
} HangmanGame;

// Initialize game
void init_game(HangmanGame* game, const char* word) {
    strncpy(game->word, word, MAX_WORD_LENGTH - 1);
    game->word[MAX_WORD_LENGTH - 1] = '\0';
    game->word_length = strlen(game->word);
    game->remaining_guesses = MAX_GUESSES;
    game->game_won = 0;
    game->game_over = 0;
    
    // Initialize guessed letters
    memset(game->guessed_letters, 0, sizeof(game->guessed_letters));
    
    // Initialize display with underscores
    for (int i = 0; i < game->word_length; i++) {
        game->display[i] = '_';
    }
    game->display[game->word_length] = '\0';
}

// Check if letter was already guessed
int is_letter_guessed(HangmanGame* game, char letter) {
    return game->guessed_letters[letter - 'a'];
}

// Make a guess
int make_guess(HangmanGame* game, char letter) {
    if (game->game_over) return 0;
    if (letter < 'a' || letter > 'z') return 0;
    if (is_letter_guessed(game, letter)) return 0;
    
    // Mark letter as guessed
    game->guessed_letters[letter - 'a'] = 1;
    
    // Check if letter is in word
    int found = 0;
    for (int i = 0; i < game->word_length; i++) {
        if (game->word[i] == letter) {
            game->display[i] = letter;
            found = 1;
        }
    }
    
    if (!found) {
        game->remaining_guesses--;
    }
    
    // Check win condition
    game->game_won = 1;
    for (int i = 0; i < game->word_length; i++) {
        if (game->display[i] == '_') {
            game->game_won = 0;
            break;
        }
    }
    
    // Check lose condition
    if (game->remaining_guesses <= 0) {
        game->game_over = 1;
    }
    
    if (game->game_won) {
        game->game_over = 1;
    }
    
    return found;
}

// Get number of correct letters
int get_correct_letters(HangmanGame* game) {
    int count = 0;
    for (int i = 0; i < game->word_length; i++) {
        if (game->display[i] != '_') {
            count++;
        }
    }
    return count;
}

// Get number of guessed letters
int get_guessed_letters_count(HangmanGame* game) {
    int count = 0;
    for (int i = 0; i < 26; i++) {
        if (game->guessed_letters[i]) {
            count++;
        }
    }
    return count;
}

// Check if word is complete
int is_word_complete(HangmanGame* game) {
    for (int i = 0; i < game->word_length; i++) {
        if (game->display[i] == '_') {
            return 0;
        }
    }
    return 1;
}

// Get game status
int get_game_status(HangmanGame* game) {
    if (game->game_won) return 1;  // Won
    if (game->game_over) return -1; // Lost
    return 0; // Still playing
}

// KLEE test function
int main() {
    HangmanGame game;
    
    // Symbolic inputs
    char word[MAX_WORD_LENGTH];
    char guess1, guess2, guess3, guess4;
    
    klee_make_symbolic(word, sizeof(word), "word");
    klee_make_symbolic(&guess1, sizeof(guess1), "guess1");
    klee_make_symbolic(&guess2, sizeof(guess2), "guess2");
    klee_make_symbolic(&guess3, sizeof(guess3), "guess3");
    klee_make_symbolic(&guess4, sizeof(guess4), "guess4");
    
    // Constrain word to be valid lowercase letters
    for (int i = 0; i < MAX_WORD_LENGTH - 1; i++) {
        klee_assume(word[i] >= 'a' && word[i] <= 'z' || word[i] == '\0');
    }
    word[MAX_WORD_LENGTH - 1] = '\0';
    
    // Ensure word is not empty
    klee_assume(strlen(word) > 0);
    klee_assume(strlen(word) <= 10);
    
    // Constrain guesses to be valid letters
    klee_assume(guess1 >= 'a' && guess1 <= 'z');
    klee_assume(guess2 >= 'a' && guess2 <= 'z');
    klee_assume(guess3 >= 'a' && guess3 <= 'z');
    klee_assume(guess4 >= 'a' && guess4 <= 'z');
    
    // Initialize game
    init_game(&game, word);
    
    // Verify initial state
    klee_assert(game.word_length > 0);
    klee_assert(game.remaining_guesses == MAX_GUESSES);
    klee_assert(!game.game_won);
    klee_assert(!game.game_over);
    klee_assert(get_correct_letters(&game) == 0);
    klee_assert(get_guessed_letters_count(&game) == 0);
    
    // Make first guess
    int result1 = make_guess(&game, guess1);
    klee_assert(result1 >= 0 && result1 <= 1);
    klee_assert(get_guessed_letters_count(&game) == 1);
    klee_assert(is_letter_guessed(&game, guess1));
    
    // Check if guess was correct
    int correct1 = 0;
    for (int i = 0; i < game.word_length; i++) {
        if (game.word[i] == guess1) {
            correct1 = 1;
            break;
        }
    }
    klee_assert(result1 == correct1);
    
    // Make second guess
    int result2 = make_guess(&game, guess2);
    klee_assert(result2 >= 0 && result2 <= 1);
    klee_assert(get_guessed_letters_count(&game) == 2);
    
    // Make third guess
    int result3 = make_guess(&game, guess3);
    klee_assert(result3 >= 0 && result3 <= 1);
    
    // Make fourth guess
    int result4 = make_guess(&game, guess4);
    klee_assert(result4 >= 0 && result4 <= 1);
    
    // Test game state after guesses
    klee_assert(game.remaining_guesses >= 0 && game.remaining_guesses <= MAX_GUESSES);
    klee_assert(get_correct_letters(&game) <= game.word_length);
    klee_assert(get_guessed_letters_count(&game) <= 4);
    
    // Test win condition
    if (is_word_complete(&game)) {
        klee_assert(game.game_won);
        klee_assert(game.game_over);
        klee_assert(get_game_status(&game) == 1);
    }
    
    // Test lose condition
    if (game.remaining_guesses <= 0) {
        klee_assert(game.game_over);
        klee_assert(!game.game_won);
        klee_assert(get_game_status(&game) == -1);
    }
    
    // Test display consistency
    for (int i = 0; i < game.word_length; i++) {
        if (game.display[i] != '_') {
            klee_assert(game.display[i] == game.word[i]);
        }
    }
    
    // Test that guessed letters are marked
    klee_assert(is_letter_guessed(&game, guess1));
    klee_assert(is_letter_guessed(&game, guess2));
    klee_assert(is_letter_guessed(&game, guess3));
    klee_assert(is_letter_guessed(&game, guess4));
    
    // Test invalid guesses
    int invalid_result = make_guess(&game, 'z' + 1); // Invalid letter
    klee_assert(invalid_result == 0);
    
    // Test duplicate guesses
    int duplicate_result = make_guess(&game, guess1); // Already guessed
    klee_assert(duplicate_result == 0);
    
    return 0;
} 