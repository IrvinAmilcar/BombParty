#include "game.h"
#include "raylib.h"
#include "wordlist.h"
#include "player.h"
#include "raygui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> //Novo include pra função de gerar numeros aleatoros!

int generatePowerUp() {
    srand((unsigned int)time(NULL));
    // Gera um número aleatório entre 1 e 4
    return (rand() % 4) + 1;
}

//Função pra aplicar o powerUP: 
//Dentro dessa função precisamos chamar as funções que aplicarão verdadeiramente os efeitos!
void applyPowerUp(Player *player, int powerUp){
    if (powerUp == 1) {
        //Pausar a bomba por 10 segundos
    } else if (powerUp == 2) {
        //Inverter a ordem dos jogadores
    } else if (powerUp == 3) {
        //pular a vez
    } else if (powerUp == 4) {
        //trocar de silaba
    }
}

static void RemovePlayerFromList(GameManager* game, Player* playerToRemove) {
    if (game == NULL || playerToRemove == NULL || game->numPlayers <= 0) {
        if (game != NULL && game->numPlayers <= 1) {
             TraceLog(LOG_INFO, "RemovePlayerFromList: Tentativa de remover o ultimo ou penultimo jogador. Fim de jogo se o ultimo.");
        } else {
             TraceLog(LOG_WARNING, "RemovePlayerFromList: Chamada invalida (game, playerToRemove nulo ou numPlayers <= 0).");
        }
        return;
    }

    TraceLog(LOG_INFO, TextFormat("Removendo jogador %s (original index %d) da lista circular. Jogadores antes: %d", playerToRemove->name, playerToRemove->originalIndex, game->numPlayers));

    if (game->numPlayers > 1) {
        playerToRemove->prev->next = playerToRemove->next;
        playerToRemove->next->prev = playerToRemove->prev;

        if (game->firstPlayer == playerToRemove) {
            game->firstPlayer = playerToRemove->next;
            TraceLog(LOG_INFO, "RemovePlayerFromList: Primeiro jogador atualizado.");
        }

         if (game->currentPlayer == playerToRemove) {
             TraceLog(LOG_INFO, "RemovePlayerFromList: Jogador atual removido. PassTurn cuidara do proximo.");
         }

        game->numPlayers--;
        TraceLog(LOG_INFO, TextFormat("Jogador %s removido. Jogadores restantes: %d.", playerToRemove->name, game->numPlayers));

        if (game->numPlayers == 1) {
             // When only one player remains, firstPlayer should point to the winner.
             // Since currentPlayer was advanced BEFORE removal in PassTurn if eliminated,
             // game->currentPlayer *should* be the winner.
             game->firstPlayer = game->currentPlayer;
             TraceLog(LOG_INFO, TextFormat("RemovePlayerFromList: Apos remocao, apenas 1 jogador restante: %s.", game->firstPlayer->name));
        } else if (game->numPlayers == 0) {
             game->firstPlayer = NULL;
             game->currentPlayer = NULL;
             TraceLog(LOG_INFO, "RemovePlayerFromList: Apos remocao, 0 jogadores restantes.");
        }

    } else {
        TraceLog(LOG_WARNING, "RemovePlayerFromList: Tentativa de remover jogador quando ja ha 1 ou menos. Isso nao deveria acontecer aqui.");
         game->numPlayers = 0;
         game->firstPlayer = NULL;
         game->currentPlayer = NULL;
    }
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
        playersArray[i].score = 0;

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

// Modified to no longer toggle edit mode on input processing,
// but returns true if a word was successfully submitted or a life was lost (turn ends).
static bool ProcessPlayerInput(GameManager* game, char* playerInput, bool* playerInputEditMode, WordList* wordList) {
    // The actual input capture and edit mode management is now primarily in main.c

    // This function is called by UpdatePlayingState when the turn ends due to input.
    // It checks the validity of the current playerInput buffer.

    if (playerInput[0] == '\0') {
        // Nothing was submitted
        return false;
    }

    TraceLog(LOG_INFO, TextFormat("Processing submitted input: '%s'", playerInput));

    bool isValid = checkWord(playerInput, game->currentSyllable, wordList);

    if (isValid){
        TraceLog(LOG_INFO, TextFormat("Palavra '%s' valida!", playerInput));
        game->currentPlayer->score++;
        TraceLog(LOG_INFO, TextFormat("%s pontou! Score atual: %d", game->currentPlayer->name, game->currentPlayer->score));
        playerInput[0] = '\0'; // Clear input buffer
        return true; // Turn ends
    } else {
        TraceLog(LOG_INFO, TextFormat("Palavra '%s' invalida!", playerInput));
        game->currentPlayer->lives--;
        TraceLog(LOG_INFO, TextFormat("%s perdeu uma vida. Vidas restantes: %d", game->currentPlayer->name, game->currentPlayer->lives));
        playerInput[0] = '\0'; // Clear input buffer
        return true; // Turn ends
    }

    return false; // Should not be reached
}


static bool HandleBombTimer(GameManager* game, float deltaTime) {
    game->bombTimer -= deltaTime;

    if (game->bombTimer <= 0.0f) {
        game->bombTimer = 0.0f;
        TraceLog(LOG_INFO, TextFormat("Tempo esgotado para %s!", game->currentPlayer->name));
        game->currentPlayer->lives--;
        TraceLog(LOG_INFO, TextFormat("%s perdeu uma vida. Vidas restantes: %d", game->currentPlayer->name, game->currentPlayer->lives));
        return true; // Turn ends
    }
    return false;
}

// Modified to accept playerInput and playerInputEditMode pointers
static GameState PassTurn(GameManager* game, WordList* wordList, char* playerInput, bool* playerInputEditMode) {
     if (game == NULL || game->currentPlayer == NULL || game->numPlayers <= 0) {
         TraceLog(LOG_ERROR, "PassTurn: GameManager, currentPlayer nulo ou numPlayers <= 0. Forcando fim de jogo.");
         return GAME_OVER;
     }

     bool currentPlayerWasEliminated = false;

     if (game->currentPlayer->lives <= 0) {
         TraceLog(LOG_INFO, TextFormat("PassTurn: Jogador %s (original index %d) foi eliminado.", game->currentPlayer->name, game->currentPlayer->originalIndex));
         if (game->numPlayers > 1) {
            Player* playerToRemove = game->currentPlayer;
            game->currentPlayer = game->currentPlayer->next; // Move to the next player BEFORE removal
            RemovePlayerFromList(game, playerToRemove);
            currentPlayerWasEliminated = true;
         } else {
             TraceLog(LOG_INFO, "PassTurn: O ultimo jogador vivo foi eliminado.");
             game->numPlayers = 0;
             game->firstPlayer = NULL;
             game->currentPlayer = NULL;
              return GAME_OVER;
         }
     } else {
          // If the current player was NOT eliminated, move to the next player
         game->currentPlayer = game->currentPlayer->next;
     }

    if (game->numPlayers <= 1) {
        TraceLog(LOG_INFO, TextFormat("PassTurn: Jogo terminou! Jogadores vivos restantes: %d", game->numPlayers));
        return GAME_OVER;
    }

    // If the game is not over, prepare for the next turn
    game->bombTimer = game->initialBombTime;
    game->currentSyllable = SelectRandomSyllable(wordList);

    // --- New: Set input box to be edited automatically ---
    if (playerInputEditMode != NULL) {
        *playerInputEditMode = true;
         TraceLog(LOG_INFO, "PassTurn: Setting playerInputEditMode to true for the next turn.");
    }
    if (playerInput != NULL) {
        playerInput[0] = '\0'; // Clear input buffer for the new turn
        TraceLog(LOG_INFO, "PassTurn: Clearing player input buffer.");
    }
    // --- End New ---


    TraceLog(LOG_INFO, TextFormat("PassTurn: Turno de %s (original index %d). Nova silaba: %s", game->currentPlayer->name, game->currentPlayer->originalIndex, game->currentSyllable));

    return PLAYING;
}

// Modified to pass playerInput and playerInputEditMode pointers to PassTurn
GameState UpdatePlayingState(GameManager* game, float deltaTime, char* playerInput, bool* playerInputEditMode, WordList* wordList) {
     if (game == NULL || game->currentPlayer == NULL || game->numPlayers <= 0) {
        TraceLog(LOG_ERROR, "UpdatePlayingState: GameManager, currentPlayer nulo ou numPlayers <= 0. Forcando fim de jogo.");
        return GAME_OVER;
    }

    bool turnEnded = false;

    // Check for input *first* so a player can answer right as the timer hits zero
    // ProcessPlayerInput now just validates the current input buffer
    if (*playerInputEditMode && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))) {
         // If in edit mode and Enter is pressed, attempt to process the input
         if (ProcessPlayerInput(game, playerInput, playerInputEditMode, wordList)) {
            turnEnded = true;
         }
    }


    // Then check the timer if input didn't end the turn
    if (!turnEnded && HandleBombTimer(game, deltaTime)) {
        turnEnded = true;
    }

    if (turnEnded) {
        // Pass the playerInput and playerInputEditMode pointers to PassTurn
        return PassTurn(game, wordList, playerInput, playerInputEditMode);
    } else {
         // If the turn didn't end, handle player input while in edit mode
         if (*playerInputEditMode) {
             SetMouseCursor(MOUSE_CURSOR_IBEAM);
             int key = GetCharPressed();

             while (key > 0) {
                 if ((key >= 32) && (key <= 126) && (strlen(playerInput) < MAX_PLAYER_INPUT_CHARS)) {
                     int len = strlen(playerInput);
                     playerInput[len] = (char)key;
                     playerInput[len + 1] = '\0';
                 }
                 key = GetCharPressed();
             }

             if (IsKeyPressed(KEY_BACKSPACE)) {
                 int len = strlen(playerInput);
                 if (len > 0) {
                     playerInput[len - 1] = '\0';
                 }
             }
         } else {
             SetMouseCursor(MOUSE_CURSOR_DEFAULT);
         }
    }


    return PLAYING;
}

// --- Placeholder para outras funções de estado (Implementar em arquivos separados se o projeto crescer) ---

// GameState UpdateMenuState(...) { ... }
// GameState UpdateGameOverState(...) { ... }
// void DrawPlayingState(...) { ... } // O desenho está atualmente em main.c, pode ser movido aqui
// void DrawMenuState(...) { ... }
// void DrawGameOverState(...) { ... }