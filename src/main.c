#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "resource_dir.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h> // Incluir para malloc e free
#include <string.h>
#include <time.h>
#include <math.h>

#include "wordlist.h"
#include "game.h"
#include "player.h"

// Remover a constante NUM_PLAYERS e o array global de jogadores
// #define NUM_PLAYERS 2
// Player players[NUM_PLAYERS];

Vector2 playerPositionsCenter;
float playerPositionsRadius = 250.0f;

GameManager game = { 0 }; // Inicializa a estrutura GameManager globalmente

// Função auxiliar para configurar os dados iniciais dos jogadores (nomes e vidas)
// Recebe o array de jogadores alocado dinamicamente e o número de jogadores
void SetupPlayers(Player* playersArray, int numPlayers) {
    if (playersArray == NULL || numPlayers <= 0) {
        TraceLog(LOG_ERROR, "SetupPlayers: Array de jogadores nulo ou numero invalido.");
        return;
    }

    for (int i = 0; i < numPlayers; ++i) {
        // Definir nome padrão (ex: Jogador 1, Jogador 2, ...)
        snprintf(playersArray[i].name, MAX_PLAYER_NAME_LEN, "Jogador %d", i + 1);
        playersArray[i].name[MAX_PLAYER_NAME_LEN - 1] = '\0'; // Garantir terminação nula
        playersArray[i].lives = 2; // Vidas iniciais
        // A screenPosition será calculada no loop de desenho
    }
     TraceLog(LOG_INFO, TextFormat("SetupPlayers: %d jogadores configurados.", numPlayers));
}


int main(void)
{
    int display = GetCurrentMonitor();
    const int initialScreenWidth = GetMonitorPhysicalWidth(display);
    const int initialScreenHeight = GetMonitorPhysicalHeight(display);

    srand(time(NULL));

    InitWindow(initialScreenWidth, initialScreenHeight, "BombParty");
    //ToggleFullscreen();

    GuiLoadStyleDefault();

    Font customFont = LoadFontEx("resources/fonts/Montserrat-Regular.ttf", 40, NULL, 0);
    Font textFont;

    if (customFont.texture.id == 0)
    {
        TraceLog(LOG_WARNING, "Failed to load font: resources/fonts/Montserrat-Regular.ttf. Using default font.");
        textFont = GetFontDefault();
    }
    else
    {
        TraceLog(LOG_INFO, "Successfully loaded font: resources/fonts/Montserrat-Regular.ttf");
        textFont = customFont;
    }

    Texture2D bombTexture = { 0 };
    Texture2D sparkTexture = { 0 };
    Texture2D arrowTexture = { 0 };

    bombTexture = LoadTexture("resources/textures/bomb.png");
    sparkTexture = LoadTexture("resources/textures/spark.png");
    arrowTexture = LoadTexture("resources/textures/arrow.png");

    if (bombTexture.id == 0 || sparkTexture.id == 0 || arrowTexture.id == 0)
    {
        TraceLog(LOG_ERROR, "Failed to load one or more textures!");
    } else {
        TraceLog(LOG_INFO, "Successfully loaded textures.");
    }

    WordList wordList = { NULL, NULL, 0 };
    wordList = LoadWordList("resources/data/palavras.txt");

    char playerInput[MAX_PLAYER_INPUT_CHARS + 1] = { 0 };
    bool playerInputEditMode = false;

    GameState currentGameState = MENU;

    // Variáveis de menu e seleção
    int menuOption = 0;
    const int maxMenuOptions = 3; // JOGAR, LEADERBOARD, CRÉDITOS

    int selectedPlayersIndex = 0; // 0: 2 players, 1: 3 players, 2: 4 players
    const int totalPlayerOptions = 3;

    int selectedMode = 0; // 0: Normal, 1: Louco

    // Variável para armazenar o número real de jogadores selecionado
    int actualNumPlayers = 0; // Será definido ao iniciar o jogo

    float initialBombTime_value = 15.0f; // Tempo inicial da bomba (pode variar com o modo)


    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        int currentActualWidth = GetScreenWidth();
        int currentActualHeight = GetScreenHeight();

        playerPositionsCenter = (Vector2){ currentActualWidth / 2.0f, currentActualHeight / 2.0f };

        float deltaTime = GetFrameTime();

        switch (currentGameState)
        {
            case MENU:
            {
                // Navegação do menu usando setas e Enter
                if (IsKeyPressed(KEY_DOWN)) {
                    menuOption = (menuOption + 1) % maxMenuOptions;
                }
                if (IsKeyPressed(KEY_UP)) {
                    menuOption = (menuOption - 1 + maxMenuOptions) % maxMenuOptions;
                }
                if (IsKeyPressed(KEY_ENTER)) {
                    switch (menuOption) {
                        case 0: currentGameState = SELECT_PLAYERS; break;
                        case 1: currentGameState = LEADERBOARD; break;
                        case 2: currentGameState = CREDITS; break;
                    }
                }
            } break;

            case SELECT_PLAYERS:
            {
                 // Navegação para selecionar número de jogadores
                if (IsKeyPressed(KEY_DOWN)) {
                    selectedPlayersIndex = (selectedPlayersIndex + 1) % totalPlayerOptions;
                }
                if (IsKeyPressed(KEY_UP)) {
                    selectedPlayersIndex = (selectedPlayersIndex - 1 + totalPlayerOptions) % totalPlayerOptions;
                }
                if (IsKeyPressed(KEY_ENTER)) {
                    // Calcula o número real de jogadores (0 -> 2, 1 -> 3, 2 -> 4)
                    actualNumPlayers = selectedPlayersIndex + 2;
                    currentGameState = SELECT_MODE; // Transita para seleção de modo
                }
                 // Voltar para o menu pressionando ESC
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    currentGameState = MENU;
                }
            } break;

            case SELECT_MODE:
            {
                // Navegação para selecionar modo de jogo
                if (IsKeyPressed(KEY_DOWN)) {
                    selectedMode = (selectedMode + 1) % 2; // 0: Normal, 1: Louco
                }
                if (IsKeyPressed(KEY_UP)) {
                    selectedMode = (selectedMode - 1 + 2) % 2;
                }
                if (IsKeyPressed(KEY_ENTER)) {
                    // **ALOCAÇÃO E INICIALIZAÇÃO DO JOGO AQUI**
                    TraceLog(LOG_INFO, TextFormat("Iniciando jogo com %d jogadores, modo %s", actualNumPlayers, (selectedMode == 0 ? "Normal" : "Louco")));

                    // 1. Aloca memória para o array de jogadores
                    game.players = (Player*)malloc(actualNumPlayers * sizeof(Player));
                    if (game.players == NULL) {
                        TraceLog(LOG_FATAL, "Falha ao alocar memoria para jogadores!");
                        // Tratar erro (sair do jogo, voltar para o menu, etc.)
                         CloseWindow(); // Exemplo: fechar a janela em caso de falha crítica
                         return 1;
                    }
                     TraceLog(LOG_INFO, TextFormat("Memoria alocada para %d jogadores em %p", actualNumPlayers, (void*)game.players));

                    // 2. Configura os dados iniciais dos jogadores
                    SetupPlayers(game.players, actualNumPlayers);

                    // 3. Define o tempo inicial da bomba com base no modo (exemplo simples)
                     if (selectedMode == 1) { // Modo Louco
                         initialBombTime_value = 10.0f; // Tempo menor
                     } else { // Modo Normal
                         initialBombTime_value = 15.0f;
                     }


                    // 4. Inicializa o GameManager usando a nova função
                    InitializeGame(&game, actualNumPlayers, initialBombTime_value, &wordList);


                    playerInputEditMode = true; // Habilita o textbox no início do jogo
                    playerInput[0] = '\0'; // Limpa input anterior
                    GuiSetState(STATE_NORMAL); // Garantir estado normal do GUI

                    currentGameState = PLAYING; // Transita para o estado de jogo

                }

                // Voltar para seleção de jogadores pressionando ESC
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    currentGameState = SELECT_PLAYERS;
                }
            } break;


            case PLAYING:
            {
                // Atualiza o estado do jogo (timer, input, turnos)
                currentGameState = UpdatePlayingState(&game, deltaTime, playerInput, &playerInputEditMode, &wordList);

                 // Verifica se o jogo terminou após a atualização
                 if (currentGameState == GAME_OVER) {
                     TraceLog(LOG_INFO, "Jogo transicionou para GAME_OVER.");
                     // Nada mais precisa ser feito aqui no loop de update,
                     // a lógica de limpeza e transição ocorrerá no case GAME_OVER
                 }

            } break;

            case GAME_OVER:
            {
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_THREE)) // Pode usar Enter ou 3 para reiniciar/voltar
                {
                    // **DESALOCAÇÃO E LIMPEZA DO JOGO AQUI**
                    ShutdownGame(&game); // Libera a memoria dos jogadores e reseta palavras usadas

                    currentGameState = MENU; // Volta para o menu
                }

            } break;

            // Adicionar casos para LEADERBOARD e CREDITS quando implementados
             case LEADERBOARD:
             {
                // Lógica de update para Leaderboard
                if (IsKeyPressed(KEY_BACKSPACE)) { // Voltar para o menu
                    currentGameState = MENU;
                }
             } break;

             case CREDITS:
             {
                // Lógica de update para Créditos
                 if (IsKeyPressed(KEY_BACKSPACE)) { // Voltar para o menu
                    currentGameState = MENU;
                }
             } break;


            default: break;
        }

        // --- Lógica de Desenho ---
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Calcula e desenha posições dos jogadores APENAS nos estados onde há jogadores
            if (currentGameState == PLAYING || currentGameState == GAME_OVER) {
                 for (int i = 0; i < game.numPlayers; ++i) {
                     // Usa game.players e game.numPlayers
                     game.players[i].screenPosition = CalculatePlayerPosition(i, game.numPlayers, playerPositionsCenter, playerPositionsRadius);
                 }

                 for (int i = 0; i < game.numPlayers; ++i) {
                     // Usa game.players e game.numPlayers
                     Color nameColor = (i == game.currentPlayerIndex && currentGameState == PLAYING) ? DARKBLUE : DARKGRAY;
                     Color lifeColor = (game.players[i].lives <= 1) ? RED : BLACK;
                     DrawPlayerInfo(&game.players[i], textFont, nameColor, lifeColor);
                 }
            }


            switch (currentGameState)
            {
                case MENU:
                {
                    ClearBackground(DARKGRAY);
                    DrawText("BOMB PARTY", GetScreenWidth()/2 - MeasureText("BOMB PARTY", 40)/2, GetScreenHeight()/3, 40, RAYWHITE); // Título centralizado

                    const char *options[] = { "JOGAR", "LEADERBOARD", "CRÉDITOS" };
                    // Desenhar opções do menu
                    int startY = GetScreenHeight()/2;
                    for (int i = 0; i < maxMenuOptions; i++)
                    {
                        Color color = (i == menuOption) ? YELLOW : RAYWHITE;
                        // Centralizar as opções horizontalmente
                        int textWidth = MeasureText(options[i], 30);
                        DrawText(options[i], GetScreenWidth()/2 - textWidth/2, startY + i * 40, 30, color);
                    }
                } break;

                case SELECT_PLAYERS:
                {
                    ClearBackground(BLACK);
                    DrawText("Selecione o número de jogadores", GetScreenWidth()/2 - MeasureText("Selecione o número de jogadores", 30)/2, 50, 30, RAYWHITE); // Título centralizado

                    const char *options[] = { "2 Jogadores", "3 Jogadores", "4 Jogadores" };

                    int startY = 120;
                    for (int i = 0; i < totalPlayerOptions; i++)
                    {
                        Color color = (i == selectedPlayersIndex) ? YELLOW : GRAY;
                         int textWidth = MeasureText(options[i], 25);
                        DrawText(options[i], GetScreenWidth()/2 - textWidth/2, startY + i * 40, 25, color);
                    }
                    DrawText("<- Voltar (BACKSPACE)", 20, GetScreenHeight() - 30, 20, RAYWHITE); // Indicativo para voltar
                } break;

                case SELECT_MODE:
                {
                    ClearBackground(DARKGRAY);
                    DrawText("Selecione o modo de jogo", GetScreenWidth()/2 - MeasureText("Selecione o modo de jogo", 30)/2, 50, 30, RAYWHITE); // Título centralizado

                    const char *modes[] = { "Normal", "Louco" };

                     int startY = 120;
                    for (int i = 0; i < 2; i++) // São sempre 2 modos
                    {
                        Color color = (i == selectedMode) ? SKYBLUE : LIGHTGRAY;
                        int textWidth = MeasureText(modes[i], 25);
                        DrawText(modes[i], GetScreenWidth()/2 - textWidth/2, startY + i * 40, 25, color);
                    }
                     DrawText("<- Voltar (BACKSPACE)", 20, GetScreenHeight() - 30, 20, RAYWHITE); // Indicativo para voltar
                } break;


                case PLAYING:
                {
                    // Desenho do input, bomba, timer e sílaba
                    Rectangle inputBounds = {currentActualWidth/2 - 150, currentActualHeight - 80, 300, 40 };
                    GuiTextBox(inputBounds, playerInput, MAX_PLAYER_INPUT_CHARS, playerInputEditMode);

                    // Desenho da seta indicando o jogador atual (usando game.numPlayers)
                    if (arrowTexture.id != 0 && game.numPlayers > 0 && game.currentPlayerIndex >= 0 && game.currentPlayerIndex < game.numPlayers) {
                         Vector2 arrowPivot = playerPositionsCenter;
                         Vector2 targetPlayerPos = game.players[game.currentPlayerIndex].screenPosition;

                         // Calcular a direção da seta
                         Vector2 direction = {
                            targetPlayerPos.x - arrowPivot.x,
                            targetPlayerPos.y - arrowPivot.y // Corrigido o erro de usar targetPlayerPos.y duas vezes
                         };

                         // Calcular o ângulo em radianos e converter para graus
                         float angle_radians = atan2f(direction.y, direction.x);
                         float angle_degrees = angle_radians * RAD2DEG;

                         // Ajuste a rotação para a seta apontar na direção correta
                         // (pode precisar de +90 ou -90 graus dependendo da orientação original da textura)
                         float arrowDrawingRotation = angle_degrees + 90.0f; // Ajuste comum para texturas apontando para cima

                         float arrowScale = 0.4f;

                         Rectangle sourceRecArrow = { 0.0f, 0.0f, (float)arrowTexture.width, (float)arrowTexture.height };
                         Rectangle destRecArrow = { arrowPivot.x, arrowPivot.y, arrowTexture.width * arrowScale, arrowTexture.height * arrowScale };
                         Vector2 originArrow = { (arrowTexture.width * arrowScale) / 2.0f, (arrowTexture.height * arrowScale) / 2.0f }; // Origem no centro da seta

                         // Desenhar a seta rotacionada
                         DrawTexturePro(arrowTexture, sourceRecArrow, destRecArrow, originArrow, arrowDrawingRotation, WHITE);
                    } else if (game.numPlayers > 0 && (game.currentPlayerIndex < 0 || game.currentPlayerIndex >= game.numPlayers)) {
                         TraceLog(LOG_WARNING, TextFormat("Indice de jogador atual (%d) fora dos limites (0 a %d) no desenho da seta.", game.currentPlayerIndex, game.numPlayers - 1));
                    }


                    float bombScale = 0.3f;
                    float bombRotation = 0.0f;

                    Vector2 bombPosition = {
                        currentActualWidth / 2 - (bombTexture.width * bombScale) / 2,
                        currentActualHeight / 2 - (bombTexture.height * bombScale) / 2
                    };

                    float sparkScale = 0.05f;
                    // A rotação e posição da faísca podem ser ajustadas para parecer que sai da bomba
                    float sparkRotation = -30.0f;
                    Vector2 sparkPosition = {
                        bombPosition.x + bombTexture.width * bombScale * 0.7f, // Posição relativa à bomba
                        bombPosition.y + bombTexture.height * bombScale * 0.5f // Posição relativa à bomba
                    };


                    if (bombTexture.id != 0) DrawTextureEx(bombTexture, bombPosition, bombRotation, bombScale, WHITE);
                    if (sparkTexture.id != 0) DrawTextureEx(sparkTexture, sparkPosition, sparkRotation, sparkScale, WHITE);

                    DrawTextEx(textFont, TextFormat("Timer: %.1f", game.bombTimer), (Vector2){currentActualWidth - 180, 10}, 25, 0, (game.bombTimer <= 5.0f ? RED : DARKGRAY));

                    // Desenha a sílaba atual
                    if (game.currentSyllable != NULL) { // A wordList.count > 0 já é verificada em SelectRandomSyllable
                        Vector2 syllablePos = {currentActualWidth/2 - MeasureTextEx(textFont, game.currentSyllable, 60, 0).x/2, currentActualHeight/2 - 80 };
                        DrawTextEx(textFont, game.currentSyllable, syllablePos, 60, 0, BLUE);
                    } else {
                         // Isso pode acontecer se houver um erro ao carregar a wordlist ou selecionar sílaba
                         DrawTextEx(textFont, "Sem Silaba!", (Vector2){currentActualWidth/2 - MeasureTextEx(textFont, "Sem Silaba!", 30, 0).x/2, currentActualHeight/2 - 80}, 30, 0, RED);
                    }

                } break;

                case GAME_OVER:
                {
                    ClearBackground(RAYWHITE); // Limpa a tela no fim de jogo

                    // Encontrar o(s) vencedor(es) - jogadores com vidas > 0
                    const char* gameOverText = "Fim de Jogo!";
                    char winnerText[100] = {0}; // Buffer para a mensagem do vencedor

                    int livingPlayersCount = 0;
                    int winnerIndex = -1;

                     // Verifica se game.players é válido antes de acessar
                    if (game.players != NULL && game.numPlayers > 0) {
                        for (int i = 0; i < game.numPlayers; ++i) {
                            if (game.players[i].lives > 0) {
                                livingPlayersCount++;
                                winnerIndex = i; // Assume que o último jogador vivo é o vencedor
                            }
                        }

                        if (livingPlayersCount == 1 && winnerIndex != -1) {
                            snprintf(winnerText, sizeof(winnerText), "%s venceu!", game.players[winnerIndex].name);
                        } else {
                            // Caso de empate, ou todos perderam rapidamente, ou 0 jogadores vivos (erro)
                             strncpy(winnerText, "Nenhum vencedor claro.", sizeof(winnerText) -1);
                             winnerText[sizeof(winnerText)-1] = '\0';
                        }
                    } else {
                         strncpy(winnerText, "Erro ao determinar vencedor.", sizeof(winnerText) -1);
                         winnerText[sizeof(winnerText)-1] = '\0';
                         TraceLog(LOG_ERROR, "GAME_OVER: game.players eh nulo ou numPlayers invalido.");
                    }


                    DrawTextEx(textFont, gameOverText, (Vector2){currentActualWidth/2 - MeasureTextEx(textFont, gameOverText, 40, 0).x/2, currentActualHeight/3}, 40, 0, DARKGRAY);

                    // Desenha a mensagem do vencedor/resultado
                    Vector2 winnerTextPos = {currentActualWidth/2 - MeasureTextEx(textFont, winnerText, 30, 0).x/2, currentActualHeight/3 + 60};
                    DrawTextEx(textFont, winnerText, winnerTextPos, 30, 0, BLUE);


                    const char* restartText = "Pressione ENTER para Voltar ao Menu"; // Alterado para voltar ao menu
                    Vector2 restartTextPos = {currentActualWidth/2 - MeasureTextEx(textFont, restartText, 20, 0).x/2, currentActualHeight/2};
                    DrawTextEx(textFont, restartText, restartTextPos, 20, 0, GRAY);

                } break;

                 case LEADERBOARD:
                 {
                    ClearBackground(LIGHTGRAY);
                    DrawText("LEADERBOARD", GetScreenWidth()/2 - MeasureText("LEADERBOARD", 40)/2, GetScreenHeight()/3, 40, BLACK);
                    DrawText("<- Voltar (BACKSPACE)", 20, GetScreenHeight() - 30, 20, DARKGRAY);
                    // Implementar lógica de desenho do leaderboard aqui
                 } break;

                 case CREDITS:
                 {
                    ClearBackground(GRAY);
                     DrawText("CREDITOS", GetScreenWidth()/2 - MeasureText("CREDITOS", 40)/2, GetScreenHeight()/3, 40, BLACK);
                     DrawText("<- Voltar (BACKSPACE)", 20, GetScreenHeight() - 30, 20, DARKGRAY);
                    // Implementar lógica de desenho dos créditos aqui
                 } break;


                default: break;
            }

            DrawFPS(10, currentActualHeight - 20);

        EndDrawing();
    }

    TraceLog(LOG_INFO, "Loop principal terminou. Iniciando limpeza de recursos.");

    // Garante que a memória dos jogadores é liberada se o jogo estiver ativo ao fechar a janela
    if (currentGameState == PLAYING || currentGameState == GAME_OVER) {
         ShutdownGame(&game);
    }


    UnloadWordList(&wordList);

    if (customFont.texture.id != 0 && textFont.texture.id != GetFontDefault().texture.id)
    {
        UnloadFont(customFont);
    }

    if (bombTexture.id != 0) UnloadTexture(bombTexture);
    if (sparkTexture.id != 0) UnloadTexture(sparkTexture);
    if (arrowTexture.id != 0) UnloadTexture(arrowTexture);

    CloseWindow();

    return 0;
}