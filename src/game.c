#include "game.h"
#include "raylib.h"
#include "wordlist.h"
#include "player.h"
#include "raygui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> //Novo include pra função de gerar numeros aleatoros!
#include "armazenarTxt.h"

int generatePowerUp() {
    srand((unsigned int)time(NULL));
    // Gera um número aleatório entre 1 e 4
    return (rand() % 4) + 1;
}

int selectedMode = 0;

//Função pra aplicar o powerUP: 
//Dentro dessa função precisamos chamar as funções que aplicarão verdadeiramente os efeitos!
void applyPowerUp(GameManager *game, Player *player, int powerUp){
    if (powerUp == 1) {
        //Pausar a bomba por 10 segundos
        pauseBomboTimer(game, player);
        game -> currentPlayer -> powerUP = 0; //Reseta o uso do powerUP após o uso!
        
    } else if (powerUp == 2) {
        //Inverter a ordem dos jogadores
        inverterOrdemDoJogo(game, player);
        game -> currentPlayer -> powerUP = 0;
        
    } else if (powerUp == 3) {
        //pular a vez
        forceTurnEnd(game, player);
        game -> currentPlayer -> powerUP = 0;

    } else if (powerUp == 4) {
        //trocar de silaba
        changeSilaba(game, player);
        game -> currentPlayer -> powerUP = 0;

    }
}

//Funçõs de powerUps:
//Pausar o tempo por 10 segundos (Adicionar mais 10 segundos ao relogio!)
void pauseBomboTimer(GameManager *game, Player *player) {
    if (game != NULL) {
        game->isTimerPaused = true;
        // Calcula o tempo EXATO do jogo (usando GetTime()) quando a pausa deve terminar
        game->timerPauseEndTime = GetTime() + 10.0; // GetTime() retorna double, por isso timerPauseEndTime deve ser double

        TraceLog(LOG_INFO, TextFormat("PowerUp PAUSE ativado por %s: Timer pausado ate %f", player->name, game->timerPauseEndTime));
    }
}

//Função pra inverter a ordem dos jogadores:
void inverterOrdemDoJogo(GameManager *game, Player *player){
    if (game != NULL) {
        // Alterna entre 0 e 1
        game->turnDirection = 1 - game->turnDirection;

        TraceLog(LOG_INFO, TextFormat("PowerUp INVERTER ativado por %s: Ordem do jogo %s.", player->name, (game->turnDirection == 0) ? "NORMAL (direita)" : "INVERTIDA (esquerda)"));

    }
}

//Função pra pular o jogador da vez
void forceTurnEnd(GameManager *game, Player *player) {
    if (game != NULL) {
        game->skipToNextPlayer = true;
        TraceLog(LOG_INFO, TextFormat("PowerUp PULAR ativado por %s: O proximo jogador sera pulado.", player->name));
        // A logica de pular DE FATO o jogador acontecera dentro de PassTurn.
    }
}

//Função pra trocar a silaba do jogador:
void changeSilaba(GameManager *game, Player *player){
    if (game != NULL && game-> gameWordList != NULL) {
        // Chama a funcao para obter uma NOVA silaba aleatoria
        const char* newSyllable = SelectRandomSyllable(game-> gameWordList);

        // Atualiza o ponteiro da silaba atual no GameManager
        game->currentSyllable = newSyllable;

        TraceLog(LOG_INFO, TextFormat("PowerUp SILABA ativado por %s: Silaba trocada para '%s'.", (player != NULL ? player->name : "Desconhecido"), game->currentSyllable));

    } else {
        TraceLog(LOG_WARNING, "PowerUp SILABA: Nao foi possivel trocar a silaba (GameManager ou gameWordList nulo?).");
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
        playersArray[i].powerUP = 0;

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

    game->gameWordList = wordList;

    //Novas inicializações pras variaveis modificadoras dos powerUPS!!!
    game -> isTimerPaused = false;
    game -> skipToNextPlayer = false;
    game -> turnDirection = 0; //Se for zero, a direção é normal (Pra direita), se for 1, pra esquerda!
    game -> timerPauseEndTime = 0.0f;

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

    if (selectedMode == 0){

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

    } else {
    
        //char *aiFile = lerArquivoParaString("resources/data/palavras_da_ia.txt");
        

        TraceLog(LOG_INFO, TextFormat("Processing submitted input: '%s'", playerInput));

        bool isValid = checkWord(playerInput, game->currentSyllable, wordList);
        
        if (isValid){
            TraceLog(LOG_INFO, TextFormat("Palavra '%s' valida!", playerInput));
            game->currentPlayer->score++;
            TraceLog(LOG_INFO, TextFormat("%s pontou! Score atual: %d", game->currentPlayer->name, game->currentPlayer->score));
            playerInput[0] = '\0'; // Clear input buffer
            
            bool isValid2 = isWordInAIlist(playerInput, "resources/data/palavras_da_ia.txt");
            if (isValid2){
                TraceLog(LOG_INFO, TextFormat("Palavra '%s' valida pela IA!", playerInput));
                game ->  currentPlayer -> powerUP = generatePowerUp(); //Player ativo recebe um powerUP (1 - 4)!
            }
            //fclose(aiFile);
            return true; // Turn ends
        } else {
            TraceLog(LOG_INFO, TextFormat("Palavra '%s' invalida!", playerInput));
            game->currentPlayer->lives--;
            TraceLog(LOG_INFO, TextFormat("%s perdeu uma vida. Vidas restantes: %d", game->currentPlayer->name, game->currentPlayer->lives));
            playerInput[0] = '\0'; // Clear input buffer
            //fclose(aiFile);
            return true; // Turn ends
        }
        //fclose(aiFile);
        return false; 

    }

    
}


static bool HandleBombTimer(GameManager* game, float deltaTime) {
    // --- Adicionado: Checar se o timer está pausado ---
    if (game->isTimerPaused) {
        // Se estiver pausado, checar se o tempo de pausa acabou
        if (GetTime() >= game->timerPauseEndTime) {
            // Tempo de pausa acabou, despausa o timer
            game->isTimerPaused = false;
            TraceLog(LOG_INFO, "Pausa do timer terminou. Timer despausado.");
        }
        // Se estiver pausado E o tempo não acabou, simplesmente não faz NADA (não decrementa o timer)
        return false; // O turno não termina por causa do timer enquanto estiver pausado
    }

    // Se o timer NÃO ESTÁ pausado, decrementa normalmente
    game->bombTimer -= deltaTime;

    if (game->bombTimer <= 0.0f) {
        game->bombTimer = 0.0f;
        TraceLog(LOG_INFO, TextFormat("Tempo esgotado para %s!", game->currentPlayer->name));
        game->currentPlayer->lives--;
        TraceLog(LOG_INFO, TextFormat("%s perdeu uma vida. Vidas restantes: %d", game->currentPlayer->name, game->currentPlayer->lives));
        return true; // Turno termina por tempo esgotado
    }

    return false; // Turno não termina pelo timer ainda
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

            if (game->turnDirection == 0) { // Direcao Normal (0): Vai para o 'next'
                game->currentPlayer = game->currentPlayer->next;
                TraceLog(LOG_INFO, "PassTurn: Movendo para o proximo na direcao NORMAL antes de remover.");
            } else { // Direcao Invertida (1): Vai para o 'prev'
                game->currentPlayer = game->currentPlayer->prev;
                TraceLog(LOG_INFO, "PassTurn: Movendo para o proximo na direcao INVERTIDA antes de remover.");
            }

            RemovePlayerFromList(game, playerToRemove); // Remove o jogador ELIMINADO da lista circular
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
        if (game->turnDirection == 0) { // Direcao Normal (0): Vai para o 'next'
            game->currentPlayer = game->currentPlayer->next;
            TraceLog(LOG_INFO, "PassTurn: Movendo para o proximo na direcao NORMAL.");
        } else { // Direcao Invertida (1): Vai para o 'prev'
            game->currentPlayer = game->currentPlayer->prev;
            TraceLog(LOG_INFO, "PassTurn: Movendo para o proximo na direcao INVERTIDA.");
        }
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

    // --- ADICIONADO: Verificar se o PowerUp "Pular (encerrar turno)" forcou o fim ---
    // Checamos game->skipToNextPlayer (que agora significa forceTurnEnd)
    if (!turnEnded && game->skipToNextPlayer) { // Se o turno ainda nao terminou E a flag "skipToNextPlayer" esta ativa
       TraceLog(LOG_INFO, TextFormat("Turno forcado a terminar (via skipToNextPlayer) para %s.", game->currentPlayer->name));
       turnEnded = true; // Forca o fim do turno
       game->skipToNextPlayer = false; // Reseta a flag depois de usá-la
    }
    // --- FIM DA ADICAO ---

    if (turnEnded) {
        // Pass the playerInput and playerInputEditMode pointers to PassTurn
        return PassTurn(game, wordList, playerInput, playerInputEditMode);
    } else {
        // If the turn didn't end, handle player input while in edit mode
        if (*playerInputEditMode) {
            SetMouseCursor(MOUSE_CURSOR_IBEAM);

            // --- NOVO: Flag para controlar se um input especial foi tratado neste frame ---
            bool specialKeyHandledThisFrame = false; // <-- Declare a flag aqui

            // --- Primeira prioridade: Checar teclas de comando (como Power-Up) ---
            if (IsKeyPressed(KEY_ONE)) {
                // Checar se o jogador atual possui um PowerUp (powerUP > 0)
                if (game-> currentPlayer != NULL && game->currentPlayer->powerUP > 0) {
                    TraceLog(LOG_INFO, TextFormat("%s ativou o PowerUp %d!", game->currentPlayer->name, game->currentPlayer->powerUP));

                    int usedPowerUpType = game->currentPlayer->powerUP; // Guarda o tipo antes de resetar
                    game->currentPlayer->powerUP = 0; // Consome o PowerUp
                    // Aplica o efeito do PowerUp usado
                    applyPowerUp(game, game->currentPlayer, usedPowerUpType);

                    specialKeyHandledThisFrame = true; // <-- NOVO: Marca que um comando foi processado
                } else {
                     // Opcional: feedback para o jogador que nao tem PowerUp
                     // TraceLog(LOG_INFO, TextFormat("%s tentou usar PowerUp, mas nao tem nenhum.", game->currentPlayer->name));
                     // Como você não quer que o '1' digite NUNCA no input se a tecla for usada como comando,
                     // mesmo que não tenha power-up, marcamos como tratado.
                     specialKeyHandledThisFrame = true; // <-- NOVO: Marca como tratado para nao digitar '1'
                }
            }
            // Adicione outras verificacoes de teclas de comando aqui com IsKeyPressed antes do proximo bloco if()

            // --- Segunda prioridade: Processar input de caracteres normais SOMENTE se nenhum comando foi tratado ---
            if (!specialKeyHandledThisFrame) { // <-- NOVO: Só processa caracteres se nenhum comando especial foi tratado
                int key = GetCharPressed();

                // Loop para pegar todos os caracteres digitados no frame
                while (key > 0) {
                    // Verifica se o caractere é imprimível (letras, espaco, hifen, etc.)
                    // e se há espaço no buffer MAX_PLAYER_INPUT_CHARS
                    if ((key >= 32) && (key <= 126) && (strlen(playerInput) < MAX_PLAYER_INPUT_CHARS)) {
                        int len = strlen(playerInput);
                        playerInput[len] = (char)key;
                        playerInput[len + 1] = '\0'; // Garante que a string esta terminada
                    }
                    key = GetCharPressed(); // Pega o proximo caractere (se houver)
                }

                // Lidar com backspace (Backspace nao é um caractere digitavel, entao nao conflita com GetCharPressed)
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    int len = strlen(playerInput);
                    if (len > 0) {
                        playerInput[len - 1] = '\0'; // Remove o ultimo caractere
                    }
                }
                // Lidar com ENTER já está sendo feito acima (IsKeyPressed(KEY_ENTER)) antes deste bloco
            } else {
                 // NOVO (Opcional mas recomendado): Limpar o buffer de GetCharPressed
                 // se uma tecla especial foi tratada, para que nenhum caractere
                 // pressionado junto (ou no mesmo frame) vaze para o proximo frame
                 // do input de texto.
                 while(GetCharPressed() > 0) {}
            }


        } else { // If not in edit mode (e.g., when not player's turn or game paused)
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }
    }


    return PLAYING; // Retorna o estado atual (continua PLAYING se o turno nao terminou)
}

// --- Placeholder para outras funções de estado (Implementar em arquivos separados se o projeto crescer) ---

// GameState UpdateMenuState(...) { ... }
// GameState UpdateGameOverState(...) { ... }
// void DrawPlayingState(...) { ... } // O desenho está atualmente em main.c, pode ser movido aqui
// void DrawMenuState(...) { ... }
// void DrawGameOverState(...) { ... }