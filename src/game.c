#include "game.h"
#include "raylib.h"
#include "wordlist.h"
#include "player.h"
#include "raygui.h"
#include <stdio.h>
#include <string.h>

static int CountLivingPlayers(const Player players[], int numPlayers) {
    int livingCount = 0;
    for (int i = 0; i < numPlayers; ++i) {
        if (players[i].lives > 0) {
            livingCount++;
        }
    }
    return livingCount;
}

static int FindNextLivingPlayerIndex(const Player players[], int numPlayers, int startingIndex) {
    int currentIndex = (startingIndex + 1) % numPlayers;
    int playersChecked = 0;

    while (playersChecked < numPlayers) {
        if (players[currentIndex].lives > 0) {
            return currentIndex;
        }
        currentIndex = (currentIndex + 1) % numPlayers;
        playersChecked++;
    }

    return -1;
}

GameState UpdatePlayingState(GameManager* game, float deltaTime, char* playerInput, bool* playerInputEditMode, WordList* wordList) {
    game->bombTimer -= deltaTime;

    bool turnShouldPass = false;

    if (game->bombTimer <= 0.0f) {
        game->bombTimer = 0.0f;
        TraceLog(LOG_INFO, TextFormat("Tempo esgotado para %s!", game->players[game->currentPlayerIndex].name));
        game->players[game->currentPlayerIndex].lives--;
        TraceLog(LOG_INFO, TextFormat("%s perdeu uma vida. Vidas restantes: %d", game->players[game->currentPlayerIndex].name, game->players[game->currentPlayerIndex].lives));

        turnShouldPass = true;
    }

    Rectangle inputBounds = {GetScreenWidth()/2 - 150, GetScreenHeight() - 80, 300, 40 };
    if (GuiTextBox(inputBounds, playerInput, MAX_PLAYER_INPUT_CHARS, *playerInputEditMode)) {
        TraceLog(LOG_INFO, TextFormat("Player submitted: '%s'", playerInput));

        bool isValid = checkWord(playerInput, game->currentSyllable, wordList);

        if (isValid){
            TraceLog(LOG_INFO, TextFormat("Palavra '%s' valida!", playerInput));
             turnShouldPass = true;

        } else {
             TraceLog(LOG_INFO, TextFormat("Palavra '%s' invalida!", playerInput));
             game->players[game->currentPlayerIndex].lives--;
              TraceLog(LOG_INFO, TextFormat("%s perdeu uma vida. Vidas restantes: %d", game->players[game->currentPlayerIndex].name, game->players[game->currentPlayerIndex].lives));

             turnShouldPass = true;
        }

         playerInput[0] = '\0';
    }

    if (turnShouldPass) {
        int livingPlayersCount = CountLivingPlayers(game->players, game->numPlayers);

        if (livingPlayersCount <= 1) {
            TraceLog(LOG_INFO, TextFormat("UpdatePlayingState: Jogo terminou! Jogadores vivos restantes: %d", livingPlayersCount));
            return GAME_OVER;
        }

        int nextPlayer = FindNextLivingPlayerIndex(game->players, game->numPlayers, game->currentPlayerIndex);

        if (nextPlayer != -1) {
            game->currentPlayerIndex = nextPlayer;
            game->bombTimer = game->initialBombTime;
            game->currentSyllable = SelectRandomSyllable(wordList);
            TraceLog(LOG_INFO, TextFormat("UpdatePlayingState: Turno de %s. Nova silaba: %s", game->players[game->currentPlayerIndex].name, game->currentSyllable));
        } else {
             TraceLog(LOG_ERROR, "UpdatePlayingState: Erro lógico crítico! Não foi possível encontrar o próximo jogador vivo. Forçando fim de jogo.");
             return GAME_OVER;
        }
    }

    return PLAYING;
}