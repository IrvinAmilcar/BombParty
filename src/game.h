#ifndef GAME_H
#define GAME_H

typedef enum GameState {
    MENU = 0,
    SELECT_PLAYERS,
    SELECT_MODE,
    LEADERBOARD,
    CREDITS,
    PLAYING,
    GAME_OVER
} GameState;

#define MAX_WORD_LENGTH 64
#define MAX_SYLLABLE_LENGTH 3
#define MIN_SYLLABLE_LENGTH 2
#define MAX_PLAYER_INPUT_CHARS 30

#endif
