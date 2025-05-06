#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "resource_dir.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "wordlist.h"
#include "game.h"

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    srand(time(NULL));

    InitWindow(screenWidth, screenHeight, "BombParty");

    // Inicialização Raygui: Carrega o estilo padrão
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

    WordList wordList = { NULL, 0 };
    wordList = LoadWordList("resources/data/palavras.txt");

    const char* currentSyllable = NULL;

    char playerInput[MAX_PLAYER_INPUT_CHARS + 1] = { 0 };
    bool playerInputEditMode = false;

    GameState currentGameState = MENU;

    float bombTimer = 0.0f;
    float initialBombTime = 15.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        switch (currentGameState)
        {
            case MENU:
            {
                if (IsKeyPressed(KEY_ONE) && wordList.count > 0)
                {
                    currentGameState = PLAYING;
                    bombTimer = initialBombTime;
                    currentSyllable = SelectRandomSyllable(&wordList);
                    TraceLog(LOG_INFO, TextFormat("Game started with syllable: %s", currentSyllable));

                    // Raygui: Ativa o modo de edição ao entrar no estado PLAYING
                    playerInputEditMode = true;
                    playerInput[0] = '\0'; // Limpa o buffer
                    GuiSetState(STATE_NORMAL); // Raygui: Garante que o estado visual do controle está normal
                }

            } break;

            case PLAYING:
            {
                bombTimer -= GetFrameTime();

                if (bombTimer <= 0.0f)
                {
                    bombTimer = 0.0f;
                    currentGameState = GAME_OVER;
                    TraceLog(LOG_INFO, "Bomb exploded! Game Over.");
                    playerInputEditMode = false;
                    currentSyllable = NULL;
                }

                if (IsKeyPressed(KEY_TWO))
                {
                 currentGameState = GAME_OVER;
                }

            } break;

            case GAME_OVER:
            {
                if (IsKeyPressed(KEY_THREE))
                {
                    currentGameState = MENU;
                }

            } break;

            default: break;
        }

        BeginDrawing();

            ClearBackground(RAYWHITE);

            switch (currentGameState)
            {
                case MENU:
                {
                    const char* menuText = "Pressione 1 para Comecar";
                    char wordCountText[64];

                    if (wordList.count > 0) {
                        snprintf(wordCountText, sizeof(wordCountText), " (%d palavras carregadas)", wordList.count);
                        menuText = TextFormat("Pressione 1 para Comecar%s", wordCountText);
                    } else {
                        menuText = "Erro: Lista de palavras nao carregada!";
                    }
                    DrawTextEx(textFont, "Bomb Party", (Vector2){screenWidth/2 - MeasureTextEx(textFont, "Bomb Party", 40, 0).x/2, screenHeight/3}, 40, 0, GRAY);
                    DrawTextEx(textFont, menuText, (Vector2){screenWidth/2 - MeasureTextEx(textFont, menuText, 20, 0).x/2, screenHeight/2}, 20, 0, DARKGRAY);
 
                } break;

                case PLAYING:
                {
                    DrawTextEx(textFont, "Estado: JOGANDO", (Vector2){10, 10}, 20, 0, BLACK);

                    if (currentSyllable != NULL && wordList.count > 0) {
                        Vector2 syllablePos = { screenWidth/2 - MeasureTextEx(textFont, currentSyllable, 60, 0).x/2, screenHeight/2 - 80 };
                        DrawTextEx(textFont, currentSyllable, syllablePos, 60, 0, BLUE);
                    } else {
                           DrawTextEx(textFont, "Sem Silaba!", (Vector2){screenWidth/2 - MeasureTextEx(textFont, "Sem Silaba!", 30, 0).x/2, screenHeight/2 - 80}, 30, 0, RED);
                    }

                    // Raygui: Define a área do campo de input
                    Rectangle inputBounds = { screenWidth/2 - 150, screenHeight - 80, 300, 40 };
                    // Raygui: Desenha o campo de input E processa o input do teclado se playerInputEditMode for true
                    // Retorna true quando Enter é pressionado E está em modo de edição
                    if (GuiTextBox(inputBounds, playerInput, MAX_PLAYER_INPUT_CHARS, playerInputEditMode)) {
                         TraceLog(LOG_INFO, TextFormat("Player submitted: '%s'", playerInput));
                         // TODO: CHAMAR LÓGICA DE VALIDAÇÃO AQUI!
                         // bool isValid = CheckWord(playerInput, currentSyllable, &wordList); // Sua função de validação
                         // if (isValid) {
                         //     TraceLog(LOG_INFO, "Word is valid!");
                         //     // TODO: Lógica de passar a bomba, selecionar nova sílaba, reiniciar timer, passar para próximo jogador
                         //     // currentSyllable = SelectRandomSyllable(&wordList); // Seleciona nova sílaba
                         //     // bombTimer = initialBombTime; // Reinicia o timer
                         //     playerInput[0] = '\0'; // Limpa o campo para a próxima entrada
                         // } else {
                         //     TraceLog(LOG_WARNING, "Word is NOT valid or does not contain syllable!");
                         //     // TODO: Lógica para punir o jogador (talvez ele perde a rodada, o timer continua, etc.)
                         // }
                    }

                    float arrowScale = 0.4f;
                    float arrowRotation = 0.0f;
                    
                    //Esse calculo serve pra por um elemento no meio da tela obviamente sem o 9, aquilo foi só pra ajustar!!
                    //arrow centralizada:
                    Vector2 arrowPosition = {
                        screenWidth / 2 - (arrowTexture.width * arrowScale) / 2,
                        (screenHeight / 2 - (arrowTexture.height * arrowScale) / 2) + 9
                    };

                    float bombScale = 0.3f;
                    float bombRotation = 0.0f;
                    
                    Vector2 bombPosition = {
                        screenWidth / 2 - (bombTexture.width * bombScale) / 2,
                        screenHeight / 2 - (bombTexture.height * bombScale) / 2
                    };

                    float sparkScale = 0.05f;
                    float sparkRotation = -30.0f * DEG2RAD;
                    
                    //Baseada na posição da bomba a faísca tem q ser movimentada manualmente!!, se o tamanho da bomba alterar a posição da faísca muda tbm!!
                    Vector2 sparkPosition = {
                        bombPosition.x + 100,
                        bombPosition.y + 5 
                    };

                    if (arrowTexture.id != 0) DrawTextureEx(arrowTexture, arrowPosition, arrowRotation, arrowScale, WHITE);
                    if (bombTexture.id != 0) DrawTextureEx(bombTexture, bombPosition, bombRotation, bombScale, WHITE);
                    if (sparkTexture.id != 0) DrawTextureEx(sparkTexture, sparkPosition, sparkRotation, sparkScale, WHITE);

                    DrawTextEx(textFont, TextFormat("Timer: %.1f", bombTimer), (Vector2){screenWidth - 150, 10}, 25, 0, (bombTimer <= 5.0f ? RED : DARKGRAY));

                } break;

                case GAME_OVER:
                {
                    DrawTextEx(textFont, "Fim de Jogo!", (Vector2){screenWidth/2 - MeasureTextEx(textFont, "Fim de Jogo!", 40, 0).x/2, screenHeight/3}, 40, 0, DARKGRAY);
                    DrawTextEx(textFont, "Pressione 3 para Reiniciar", (Vector2){screenWidth/2 - MeasureTextEx(textFont, "Pressione 3 para Reiniciar", 20, 0).x/2, screenHeight/2}, 20, 0, GRAY);
                } break;

                default: break;
            }

            DrawFPS(10, screenHeight - 20);

        EndDrawing();
    }

    TraceLog(LOG_INFO, "Loop principal terminou. Iniciando limpeza de recursos.");
    
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