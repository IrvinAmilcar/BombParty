#include "game.h"
#include "raylib.h"
#include "wordlist.h"
#include "player.h"
#include "raygui.h"
#include <stdio.h>
#include <string.h>

// --- Funções Auxiliares (mantidas static) ---

// Conta o número de jogadores com vidas > 0
static int CountLivingPlayers(const Player players[], int numPlayers) {
    int livingCount = 0;
    for (int i = 0; i < numPlayers; ++i) {
        if (players[i].lives > 0) {
            livingCount++;
        }
    }
    return livingCount;
}

// Encontra o índice do próximo jogador vivo no ciclo, começando APÓS o startingIndex
// Retorna -1 se nenhum jogador vivo for encontrado (todos eliminados)
static int FindNextLivingPlayerIndex(const Player players[], int numPlayers, int startingIndex) {
    if (numPlayers <= 0) return -1;

    int currentIndex = (startingIndex + 1) % numPlayers;
    int playersChecked = 0; // Contador para evitar loop infinito caso não haja jogadores vivos

    while (playersChecked < numPlayers) {
        if (players[currentIndex].lives > 0) {
            return currentIndex;
        }
        currentIndex = (currentIndex + 1) % numPlayers;
        playersChecked++;
    }

    return -1; // Nenhum jogador vivo encontrado
}

// --- Novas Funções para Refatorar UpdatePlayingState ---

// Processa a entrada do jogador e verifica a palavra
// Retorna true se a palavra for válida ou inválida (turno passa), false se o input ainda está sendo editado
static bool ProcessPlayerInput(GameManager* game, char* playerInput, bool* playerInputEditMode, WordList* wordList) {
    Rectangle inputBounds = {GetScreenWidth()/2 - 150, GetScreenHeight() - 80, 300, 40 };
    if (GuiTextBox(inputBounds, playerInput, MAX_PLAYER_INPUT_CHARS, *playerInputEditMode)) {
        TraceLog(LOG_INFO, TextFormat("Player submitted: '%s'", playerInput));

        bool isValid = checkWord(playerInput, game->currentSyllable, wordList);

        if (isValid){
            TraceLog(LOG_INFO, TextFormat("Palavra '%s' valida!", playerInput));
             // turnShouldPass = true; // O chamador decidirá se o turno passa
             playerInput[0] = '\0'; // Limpa o input após submissão
             return true; // Indica que o input foi processado
        } else {
             TraceLog(LOG_INFO, TextFormat("Palavra '%s' invalida!", playerInput));
             game->players[game->currentPlayerIndex].lives--;
              TraceLog(LOG_INFO, TextFormat("%s perdeu uma vida. Vidas restantes: %d", game->players[game->currentPlayerIndex].name, game->players[game->currentPlayerIndex].lives));
             // turnShouldPass = true; // O chamador decidirá se o turno passa
             playerInput[0] = '\0'; // Limpa o input após submissão
             return true; // Indica que o input foi processado
        }
    }
    return false; // Input ainda está sendo editado
}

// Lida com a lógica do timer da bomba
// Retorna true se o timer esgotou, false caso contrário
static bool HandleBombTimer(GameManager* game, float deltaTime) {
    game->bombTimer -= deltaTime;

    if (game->bombTimer <= 0.0f) {
        game->bombTimer = 0.0f;
        TraceLog(LOG_INFO, TextFormat("Tempo esgotado para %s!", game->players[game->currentPlayerIndex].name));
        game->players[game->currentPlayerIndex].lives--;
        TraceLog(LOG_INFO, TextFormat("%s perdeu uma vida. Vidas restantes: %d", game->players[game->currentPlayerIndex].name, game->players[game->currentPlayerIndex].lives));
        return true; // Indica que o timer esgotou
    }
    return false; // Timer ainda não esgotou
}

// Passa o turno para o próximo jogador vivo
// Retorna o novo GameState (PLAYING ou GAME_OVER)
static GameState PassTurn(GameManager* game, WordList* wordList) {
     int livingPlayersCount = CountLivingPlayers(game->players, game->numPlayers);

    if (livingPlayersCount <= 1) {
        TraceLog(LOG_INFO, TextFormat("PassTurn: Jogo terminou! Jogadores vivos restantes: %d", livingPlayersCount));
        return GAME_OVER;
    }

    int nextPlayer = FindNextLivingPlayerIndex(game->players, game->numPlayers, game->currentPlayerIndex);

    if (nextPlayer != -1) {
        game->currentPlayerIndex = nextPlayer;
        game->bombTimer = game->initialBombTime; // Reinicia o timer para o próximo jogador
        game->currentSyllable = SelectRandomSyllable(wordList); // Seleciona nova sílaba
        TraceLog(LOG_INFO, TextFormat("PassTurn: Turno de %s. Nova silaba: %s", game->players[game->currentPlayerIndex].name, game->currentSyllable));
        return PLAYING; // Continua no estado PLAYING
    } else {
         // Isso não deve acontecer se livingPlayersCount > 1, mas como fallback:
         TraceLog(LOG_ERROR, "PassTurn: Erro lógico crítico! Não foi possível encontrar o próximo jogador vivo. Forçando fim de jogo.");
         return GAME_OVER;
    }
}


// --- Função Principal de Update do Estado PLAYING (Refatorada) ---

GameState UpdatePlayingState(GameManager* game, float deltaTime, char* playerInput, bool* playerInputEditMode, WordList* wordList) {

    bool turnEnded = false; // Flag para saber se o turno atual acabou

    // 1. Lida com o timer da bomba
    if (HandleBombTimer(game, deltaTime)) {
        turnEnded = true; // O turno acabou porque o tempo esgotou
    }

    // 2. Processa a entrada do jogador (se o turno ainda não acabou pelo timer)
    // Usa 'else if' para que apenas uma condição (timer ou input) determine o fim do turno
    else if (ProcessPlayerInput(game, playerInput, playerInputEditMode, wordList)) {
        turnEnded = true; // O turno acabou porque o jogador submeteu uma palavra
    }

    // 3. Se o turno acabou, passa para o próximo jogador ou termina o jogo
    if (turnEnded) {
        return PassTurn(game, wordList); // Passa o turno e retorna o próximo estado (PLAYING ou GAME_OVER)
    }

    // Se o turno não acabou, permanece no estado PLAYING
    return PLAYING;
}

// --- Outras Funções de Jogo (Exemplo - Adicione conforme necessário) ---

// Inicializa um novo jogo (esta função seria chamada em main.c após seleção de jogadores e modo)
void InitializeGame(GameManager* game, int numPlayers, float initialBombTime, WordList* wordList) {
    // Assume-se que game->players já foi alocado dinamicamente em main.c
    game->numPlayers = numPlayers;
    game->initialBombTime = initialBombTime;
    game->bombTimer = initialBombTime;
    game->currentPlayerIndex = 0; // Começa com o primeiro jogador
    game->currentSyllable = SelectRandomSyllable(wordList); // Seleciona a primeira sílaba
    // A inicialização dos dados individuais dos jogadores (nome, vidas) deve ser feita em main.c
    // ou em uma função separada chamada de main.c após a alocação.

    TraceLog(LOG_INFO, TextFormat("InitializeGame: Jogo iniciado com %d jogadores. Silaba inicial: %s", game->numPlayers, game->currentSyllable));
}

// Limpa os recursos do jogo (esta função seria chamada em main.c ao sair do estado PLAYING)
void ShutdownGame(GameManager* game) {
    // Libera a memória alocada para os jogadores em main.c
    if (game->players != NULL) {
        free(game->players); // Libera o array de structs Player
        game->players = NULL;
    }
    game->numPlayers = 0;
    game->currentPlayerIndex = -1; // Índice inválido após desligamento
    game->currentSyllable = NULL;

    ResetUsedWordList(); // Reseta a lista de palavras usadas para o próximo jogo

    TraceLog(LOG_INFO, "ShutdownGame: Recursos do jogo liberados.");
}

// --- Placeholder para outras funções de estado (Implementar em arquivos separados se o projeto crescer) ---

// GameState UpdateMenuState(...) { ... }
// GameState UpdateGameOverState(...) { ... }
// void DrawPlayingState(...) { ... } // O desenho está atualmente em main.c, pode ser movido aqui
// void DrawMenuState(...) { ... }
// void DrawGameOverState(...) { ... }