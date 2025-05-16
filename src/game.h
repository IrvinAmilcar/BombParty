#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "wordlist.h"
#include "player.h"
#include <stdbool.h>

typedef enum GameState {
    MENU = 0,
    SELECT_PLAYERS,
    SELECT_PALYERS_NAME,
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

typedef struct {
    float bombTimer;
    float initialBombTime;
    const char* currentSyllable;
    int currentPlayerIndex;
    Player* players;
    int numPlayers;
} GameManager;

GameState UpdatePlayingState(GameManager* game, float deltaTime, char* playerInput, bool* playerInputEditMode, WordList* wordList);

// --- Novos protótipos adicionados ---
void InitializeGame(GameManager* game, int numPlayers, float initialBombTime, WordList* wordList);
void ShutdownGame(GameManager* game);
// --- Fim dos novos protótipos ---

// - Funções de Update para outros estados (UpdateMenuState, UpdateGameOverState)
// - Funções de Desenho para cada estado (DrawMenuState, DrawPlayingState, DrawGameOverState)
// - Funções auxiliares de alto nível que podem ser usadas em múltiplos arquivos.

#endif