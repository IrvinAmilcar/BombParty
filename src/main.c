#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "resource_dir.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "wordlist.h"
#include "game.h"
#include "player.h"

#define NUM_PLAYERS 2
Player players[NUM_PLAYERS];
Vector2 playerPositionsCenter;
float playerPositionsRadius = 250.0f;

GameManager game;

void InitializePlayers() {
    char playerNames[NUM_PLAYERS][MAX_PLAYER_NAME_LEN] = {"Jogador 1", "Jogador 2"};

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        strncpy(players[i].name, playerNames[i], MAX_PLAYER_NAME_LEN - 1);
        players[i].name[MAX_PLAYER_NAME_LEN - 1] = '\0';
        players[i].lives = 2;
    }
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

    const float initialBombTime_value = 15.0f;

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
                if (IsKeyPressed(KEY_ONE) && wordList.count > 0)
                {
                    currentGameState = PLAYING;

                    InitializePlayers();

                    game.bombTimer = initialBombTime_value;
                    game.initialBombTime = initialBombTime_value;
                    game.players = players;
                    game.numPlayers = NUM_PLAYERS;
                    game.currentPlayerIndex = 0;
                    game.currentSyllable = SelectRandomSyllable(&wordList);

                    TraceLog(LOG_INFO, TextFormat("Game started with syllable: %s", game.currentSyllable));

                    playerInputEditMode = true;
                    playerInput[0] = '\0';
                    GuiSetState(STATE_NORMAL);

                    ResetUsedWordList();
                }

            } break;

            case PLAYING:
            {
                currentGameState = UpdatePlayingState(&game, deltaTime, playerInput, &playerInputEditMode, &wordList);
            } break;

            case GAME_OVER:
            {
                if (IsKeyPressed(KEY_THREE))
                {
                    ResetUsedWordList();
                    currentGameState = MENU;
                }

            } break;

            default: break;
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            for (int i = 0; i < NUM_PLAYERS; ++i) {
                 players[i].screenPosition = CalculatePlayerPosition(i, NUM_PLAYERS, playerPositionsCenter, playerPositionsRadius);
            }

            for (int i = 0; i < NUM_PLAYERS; ++i) {
                Color nameColor = (i == game.currentPlayerIndex) ? DARKBLUE : DARKGRAY;
                Color lifeColor = (players[i].lives <= 1) ? RED : BLACK;
                DrawPlayerInfo(&players[i], textFont, nameColor, lifeColor);
            }


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
                    DrawTextEx(textFont, "Bomb Party", (Vector2){currentActualWidth/2 - MeasureTextEx(textFont, "Bomb Party", 40, 0).x/2, currentActualHeight/3}, 40, 0, GRAY);
                    DrawTextEx(textFont, menuText, (Vector2){currentActualWidth/2 - MeasureTextEx(textFont, menuText, 20, 0).x/2, currentActualHeight/2}, 20, 0, DARKGRAY);

                } break;

                case PLAYING:
                {
                    if (game.currentSyllable != NULL && wordList.count > 0) {
                        Vector2 syllablePos = {currentActualWidth/2 - MeasureTextEx(textFont, game.currentSyllable, 60, 0).x/2, currentActualHeight/2 - 80 };
                        DrawTextEx(textFont, game.currentSyllable, syllablePos, 60, 0, BLUE);
                    } else {
                            DrawTextEx(textFont, "Sem Silaba!", (Vector2){currentActualWidth/2 - MeasureTextEx(textFont, "Sem Silaba!", 30, 0).x/2, currentActualHeight/2 - 80}, 30, 0, RED);
                    }

                    Rectangle inputBounds = {currentActualWidth/2 - 150, currentActualHeight - 80, 300, 40 };
                    GuiTextBox(inputBounds, playerInput, MAX_PLAYER_INPUT_CHARS, playerInputEditMode);

                    if (arrowTexture.id != 0 && game.numPlayers > 0) {
                         Vector2 arrowPivot = playerPositionsCenter;
                         Vector2 targetPlayerPos = game.players[game.currentPlayerIndex].screenPosition;

                         Vector2 direction = {
                            targetPlayerPos.x - arrowPivot.x,
                            targetPlayerPos.y - arrowPivot.y
                         };

                         float angle_radians = atan2f(direction.y, direction.x);
                         float angle_degrees = angle_radians * RAD2DEG;

                         float arrowDrawingRotation = angle_degrees;

                         float arrowScale = 0.4f;

                         Rectangle sourceRecArrow = { 0.0f, 0.0f, (float)arrowTexture.width, (float)arrowTexture.height };
                         Rectangle destRecArrow = { arrowPivot.x, arrowPivot.y, arrowTexture.width * arrowScale, arrowTexture.height * arrowScale };
                         Vector2 originArrow = { (arrowTexture.width * arrowScale) / 2.0f, (arrowTexture.height * arrowScale) / 2.0f };

                         DrawTexturePro(arrowTexture, sourceRecArrow, destRecArrow, originArrow, arrowDrawingRotation, WHITE);
                    }

                    float bombScale = 0.3f;
                    float bombRotation = 0.0f;

                    Vector2 bombPosition = {
                        currentActualWidth / 2 - (bombTexture.width * bombScale) / 2,
                        currentActualHeight / 2 - (bombTexture.height * bombScale) / 2
                    };

                    float sparkScale = 0.05f;
                    float sparkRotation = -30.0f;

                    Vector2 sparkPosition = {
                        bombPosition.x + 100,
                        bombPosition.y + 5
                    };

                    if (bombTexture.id != 0) DrawTextureEx(bombTexture, bombPosition, bombRotation, bombScale, WHITE);
                    if (sparkTexture.id != 0) DrawTextureEx(sparkTexture, sparkPosition, sparkRotation, sparkScale, WHITE);

                    DrawTextEx(textFont, TextFormat("Timer: %.1f", game.bombTimer), (Vector2){currentActualWidth - 150, 10}, 25, 0, (game.bombTimer <= 5.0f ? RED : DARKGRAY));

                } break;

                case GAME_OVER:
                {
                    DrawTextEx(textFont, "Fim de Jogo!", (Vector2){currentActualWidth/2 - MeasureTextEx(textFont, "Fim de Jogo!", 40, 0).x/2, currentActualHeight/3}, 40, 0, DARKGRAY);
                    DrawTextEx(textFont, "Pressione 3 para Reiniciar", (Vector2){currentActualWidth/2 - MeasureTextEx(textFont, "Pressione 3 para Reiniciar", 20, 0).x/2, currentActualHeight/2}, 20, 0, GRAY);
                } break;

                default: break;
            }

            DrawFPS(10, currentActualHeight - 20);

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