#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "wordlist.h"
#include "player.h" 
#include <stdbool.h>
#include <stdlib.h> 

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

    //Novos campos para implementação dos powerUPS!
    bool isTimerPaused;
    double timerPauseEndTime;
    int turnDirection;
    bool skipToNextPlayer;

    Player* currentPlayer; 
    Player* firstPlayer;   
    int numPlayers;

    Player* allocatedPlayersArrayBase;

} GameManager;

extern int selectedMode;

GameState UpdatePlayingState(GameManager* game, float deltaTime, char* playerInput, bool* playerInputEditMode, WordList* wordList);

void InitializeGame(GameManager* game, int numInitialPlayers, float initialBombTime, WordList* wordList);
void ShutdownGame(GameManager* game);

//Vai gerar um numero aleatorio de 1 - 4 pra definir os powerUPs!!
int generatePowerUp();

//Função pra aplicar os powerUPS:
void applyPowerUp(Player *player, int powerUp);

//Funções modificadoras do jogo (Os powerUPS em sikkkk):
void pauseBomboTimer(GameManager *game, Player *player);   //Função pra pausar o tempo da bomba por 10 segundos!
void inverterOrdemDoJogo(GameManager *game, Player *player); //Função pra inverter a ordem do jogo
void skipPlayer(GameManager *game, Player *player); //Função pra pular o jogador
void changeSilaba(GameManager *game, Player *player); //Troca a silaba do player atual!

#endif