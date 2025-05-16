#include "game.h"
#include "raylib.h"
#include "wordlist.h"
#include "player.h" 
#include "raygui.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

static void RemovePlayerFromList(GameManager* game, Player* playerToRemove) {
    if (game == NULL || playerToRemove == NULL || game->numPlayers <= 1) {
        if (game != NULL && game->numPlayers <= 1) {
             TraceLog(LOG_INFO, "RemovePlayerFromList: Tentativa de remover o ultimo ou penultimo jogador. Fim de jogo se o ultimo.");
        } else {
             TraceLog(LOG_WARNING, "RemovePlayerFromList: Chamada invalida (game ou playerToRemove nulo).");
        }
        return;
    }

    TraceLog(LOG_INFO, TextFormat("Removendo jogador %s (original index %d) da lista circular.", playerToRemove->name, playerToRemove->originalIndex));

    playerToRemove->prev->next = playerToRemove->next;
    playerToRemove->next->prev = playerToRemove->prev;

    if (game->firstPlayer == playerToRemove) {
        game->firstPlayer = playerToRemove->next;
         TraceLog(LOG_INFO, "RemovePlayerFromList: Primeiro jogador atualizado.");
    }

     if (game->currentPlayer == playerToRemove) {
         TraceLog(LOG_INFO, "RemovePlayerFromList: Jogador atual removido. Proximo jogador eh o novo currentPlayer.");
     }


    game->numPlayers--;

    TraceLog(LOG_INFO, TextFormat("Jogador %s removido. Jogadores restantes: %d.", playerToRemove->name, game->numPlayers));
}


void InitializeGame(GameManager* game, int numInitialPlayers, float initialBombTime, WordList* wordList) {
    if (game == NULL || numInitialPlayers <= 0) {
        TraceLog(LOG_ERROR, "InitializeGame: GameManager nulo ou numero de jogadores invalido.");
        if (game != NULL) {
            game->currentPlayer = NULL;
            game->firstPlayer = NULL;
            game->numPlayers = 0;
            game->allocatedPlayersArrayBase = NULL;
        }
        return;
    }

     TraceLog(LOG_INFO, TextFormat("InitializeGame: Iniciando com %d jogadores (lista circular).", numInitialPlayers));

    Player* playersArray = (Player*)malloc(numInitialPlayers * sizeof(Player));
    if (playersArray == NULL) {
        TraceLog(LOG_FATAL, "InitializeGame: Falha ao alocar memoria para jogadores!");
        game->currentPlayer = NULL;
        game->firstPlayer = NULL;
        game->numPlayers = 0;
        game->allocatedPlayersArrayBase = NULL;
        return;
    }
    
    TraceLog(LOG_INFO, TextFormat("Memoria alocada para %d jogadores em %p", numInitialPlayers, (void*)playersArray));

    game->allocatedPlayersArrayBase = playersArray;

    for (int i = 0; i < numInitialPlayers; ++i) {
        snprintf(playersArray[i].name, MAX_PLAYER_NAME_LEN, "Jogador %d", i + 1);
        playersArray[i].name[MAX_PLAYER_NAME_LEN - 1] = '\0';
        playersArray[i].lives = 2; 
        playersArray[i].screenPosition = (Vector2){0, 0};
        playersArray[i].originalIndex = i;

         playersArray[i].next = NULL;
         playersArray[i].prev = NULL;
    }

    for (int i = 0; i < numInitialPlayers; ++i) {
        playersArray[i].next = &playersArray[(i + 1) % numInitialPlayers];
        playersArray[i].prev = &playersArray[(i - 1 + numInitialPlayers) % numInitialPlayers]; 
    }

    game->firstPlayer = &playersArray[0]; 
    game->currentPlayer = game->firstPlayer; 
    game->numPlayers = numInitialPlayers; 

    game->initialBombTime = initialBombTime;
    game->bombTimer = initialBombTime;
    game->currentSyllable = SelectRandomSyllable(wordList); 

    ResetUsedWordList(); 

    TraceLog(LOG_INFO, TextFormat("InitializeGame: Jogo configurado com lista circular. Silaba inicial: %s", game->currentSyllable));
}

void ShutdownGame(GameManager* game) {
    if (game == NULL) {
        TraceLog(LOG_WARNING, "ShutdownGame: GameManager nulo. Nada para liberar.");
        return;
    }

    TraceLog(LOG_INFO, "ShutdownGame: Liberando memoria dos jogadores (lista circular)...");

    if (game->allocatedPlayersArrayBase != NULL) {
         TraceLog(LOG_INFO, TextFormat("ShutdownGame: Liberando bloco alocado em %p", (void*)game->allocatedPlayersArrayBase));
         free(game->allocatedPlayersArrayBase);
         game->allocatedPlayersArrayBase = NULL; 
         TraceLog(LOG_INFO, "ShutdownGame: Bloco de jogadores liberado.");
    } else {
         TraceLog(LOG_INFO, "ShutdownGame: Ponteiro para base do array alocado nulo. Nada para liberar.");
    }

    game->currentPlayer = NULL;
    game->firstPlayer = NULL;
    game->numPlayers = 0;
    game->currentSyllable = NULL; 


    ResetUsedWordList(); 

    TraceLog(LOG_INFO, "ShutdownGame: Recursos do jogo liberados.");
}

static bool ProcessPlayerInput(GameManager* game, char* playerInput, bool* playerInputEditMode, WordList* wordList) {
    Rectangle inputBounds = {GetScreenWidth()/2 - 150, GetScreenHeight() - 80, 300, 40 };
    if (GuiTextBox(inputBounds, playerInput, MAX_PLAYER_INPUT_CHARS, *playerInputEditMode)) {
        TraceLog(LOG_INFO, TextFormat("Player submitted: '%s'", playerInput));

        bool isValid = checkWord(playerInput, game->currentSyllable, wordList);

        if (isValid){
            TraceLog(LOG_INFO, TextFormat("Palavra '%s' valida!", playerInput));
             playerInput[0] = '\0'; 
             return true; 
        } else {
             TraceLog(LOG_INFO, TextFormat("Palavra '%s' invalida!", playerInput));
             game->currentPlayer->lives--;
              TraceLog(LOG_INFO, TextFormat("%s perdeu uma vida. Vidas restantes: %d", game->currentPlayer->name, game->currentPlayer->lives));
             playerInput[0] = '\0'; 
             return true; 
        }
    }
    return false; 
}

static bool HandleBombTimer(GameManager* game, float deltaTime) {
    game->bombTimer -= deltaTime;

    if (game->bombTimer <= 0.0f) {
        game->bombTimer = 0.0f;
        TraceLog(LOG_INFO, TextFormat("Tempo esgotado para %s!", game->currentPlayer->name));
        game->currentPlayer->lives--;
        TraceLog(LOG_INFO, TextFormat("%s perdeu uma vida. Vidas restantes: %d", game->currentPlayer->name, game->currentPlayer->lives));
        return true; 
    }
    return false; 
}

static GameState PassTurn(GameManager* game, WordList* wordList) {
     if (game == NULL || game->currentPlayer == NULL || game->numPlayers <= 0) {
         TraceLog(LOG_ERROR, "PassTurn: GameManager, currentPlayer nulo ou numPlayers <= 0. Forcando fim de jogo.");
         return GAME_OVER;
     }

     bool currentPlayerWasEliminated = false;

     if (game->currentPlayer->lives <= 0) {
         TraceLog(LOG_INFO, TextFormat("PassTurn: Jogador %s (original index %d) foi eliminado.", game->currentPlayer->name, game->currentPlayer->originalIndex));
         if (game->numPlayers > 1) {
            RemovePlayerFromList(game, game->currentPlayer); 
            currentPlayerWasEliminated = true; // flag
         } else {
             TraceLog(LOG_INFO, "PassTurn: O ultimo jogador vivo foi eliminado.");
              return GAME_OVER;
         }
     }

    if (game->numPlayers <= 1) {
        TraceLog(LOG_INFO, TextFormat("PassTurn: Jogo terminou! Jogadores vivos restantes: %d", game->numPlayers));
        return GAME_OVER;
    }

    if (!currentPlayerWasEliminated) {
        game->currentPlayer = game->currentPlayer->next;
    }

    game->bombTimer = game->initialBombTime;
    game->currentSyllable = SelectRandomSyllable(wordList);

    TraceLog(LOG_INFO, TextFormat("PassTurn: Turno de %s (original index %d). Nova silaba: %s", game->currentPlayer->name, game->currentPlayer->originalIndex, game->currentSyllable));

    return PLAYING; 
}

GameState UpdatePlayingState(GameManager* game, float deltaTime, char* playerInput, bool* playerInputEditMode, WordList* wordList) {
     if (game == NULL || game->currentPlayer == NULL || game->numPlayers <= 0) {
        TraceLog(LOG_ERROR, "UpdatePlayingState: GameManager, currentPlayer nulo ou numPlayers <= 0. Forcando fim de jogo.");
        return GAME_OVER;
    }

    bool turnEnded = false; 

    if (HandleBombTimer(game, deltaTime)) {
        turnEnded = true; 
    }

    else if (ProcessPlayerInput(game, playerInput, playerInputEditMode, wordList)) {
        turnEnded = true; 
    }

    if (turnEnded) {
        return PassTurn(game, wordList);
    }

    return PLAYING;
}

// --- Placeholder para outras funções de estado (Implementar em arquivos separados se o projeto crescer) ---

// GameState UpdateMenuState(...) { ... }
// GameState UpdateGameOverState(...) { ... }
// void DrawPlayingState(...) { ... } // O desenho está atualmente em main.c, pode ser movido aqui
// void DrawMenuState(...) { ... }
// void DrawGameOverState(...) { ... }