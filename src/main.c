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

Vector2 playerPositionsCenter;
float playerPositionsRadius = 250.0f;

GameManager game = { 0 };

int main(void)
{
    int display = GetCurrentMonitor();
    const int initialScreenWidth = GetMonitorPhysicalWidth(display);
    const int initialScreenHeight = GetMonitorPhysicalHeight(display);

    srand(time(NULL));

    InitWindow(initialScreenWidth, initialScreenHeight, "BombParty");

    GuiLoadStyleDefault();

    Font customFont = LoadFontEx("resources/fonts/Montserrat-Regular.ttf", 40, NULL, 0);
    Font textFont;

    if (customFont.texture.id == 0)
    {
        textFont = GetFontDefault();
    }
    else
    {
        textFont = customFont;
    }

    Texture2D bombTexture = { 0 };
    Texture2D sparkTexture = { 0 };
    Texture2D arrowTexture = { 0 };

    bombTexture = LoadTexture("resources/textures/bomb.png");
    if (bombTexture.id == 0) TraceLog(LOG_WARNING, "Failed to load bomb texture.");
    sparkTexture = LoadTexture("resources/textures/spark.png");
    if (sparkTexture.id == 0) TraceLog(LOG_WARNING, "Failed to load spark texture.");
    arrowTexture = LoadTexture("resources/textures/arrow.png");
    if (arrowTexture.id == 0) TraceLog(LOG_WARNING, "Failed to load arrow texture.");


    WordList wordList = { NULL, NULL, 0 };
    wordList = LoadWordList("resources/data/palavras.txt");
    if (wordList.count == 0) TraceLog(LOG_FATAL, "Failed to load word list or word list is empty.");


    char playerInput[MAX_PLAYER_INPUT_CHARS + 1] = { 0 };
    bool playerInputEditMode = false; // Initialize as false, set to true when game starts

    GameState currentGameState = MENU;

    int menuOption = 0;
    const int maxMenuOptions = 3;

    int selectedPlayersIndex = 0;
    const int totalPlayerOptions = 3;

    int selectedMode = 0;

    int numPlayersSelectedInMenu = 0;

    float initialBombTime_value = 15.0f;

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
                if (IsKeyPressed(KEY_DOWN)) {
                    menuOption = (menuOption + 1) % maxMenuOptions;
                }
                if (IsKeyPressed(KEY_UP)) {
                    menuOption = (menuOption - 1 + maxMenuOptions) % maxMenuOptions;
                }
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                    switch (menuOption) {
                        case 0: currentGameState = SELECT_PLAYERS; break;
                        case 1: currentGameState = LEADERBOARD; break;
                        case 2: currentGameState = CREDITS; break;
                    }
                }
            } break;

            case SELECT_PLAYERS:
            {
                if (IsKeyPressed(KEY_DOWN)) {
                    selectedPlayersIndex = (selectedPlayersIndex + 1) % totalPlayerOptions;
                }
                if (IsKeyPressed(KEY_UP)) {
                    selectedPlayersIndex = (selectedPlayersIndex - 1 + totalPlayerOptions) % totalPlayerOptions;
                }
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                    numPlayersSelectedInMenu = selectedPlayersIndex + 2;
                    currentGameState = SELECT_MODE;
                }
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    currentGameState = MENU;
                }
            } break;

            case SELECT_MODE:
            {
                if (IsKeyPressed(KEY_DOWN)) {
                    selectedMode = (selectedMode + 1) % 2;
                }
                if (IsKeyPressed(KEY_UP)) {
                    selectedMode = (selectedMode - 1 + 2) % 2;
                }
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                    if (selectedMode == 1) {
                        initialBombTime_value = 10.0f;
                    } else {
                        initialBombTime_value = 15.0f;
                    }

                    InitializeGame(&game, numPlayersSelectedInMenu, initialBombTime_value, &wordList);

                    if (game.firstPlayer == NULL || game.numPlayers == 0) {
                        TraceLog(LOG_ERROR, "Falha ao iniciar o jogo apos selecao de modo.");
                        currentGameState = MENU;
                    } else {
                        // --- New: Set edit mode to true and clear input on game start ---
                        playerInputEditMode = true;
                        playerInput[0] = '\0';
                        TraceLog(LOG_INFO, "Entering PLAYING state: playerInputEditMode set to true.");
                        // --- End New ---
                        GuiSetState(STATE_NORMAL);
                        currentGameState = PLAYING;
                    }
                }

                if (IsKeyPressed(KEY_BACKSPACE)) {
                    currentGameState = SELECT_PLAYERS;
                }
            } break;

            case PLAYING:
            {
                 if (game.currentPlayer == NULL || game.numPlayers <= 0) {
                     TraceLog(LOG_WARNING, "PLAYING state entered with no current player or zero players. Transitioning to GAME_OVER.");
                     currentGameState = GAME_OVER;
                 } else {
                    // Pass playerInput and playerInputEditMode to UpdatePlayingState
                    currentGameState = UpdatePlayingState(&game, deltaTime, playerInput, &playerInputEditMode, &wordList);
                 }

            } break;

            case GAME_OVER:
            {
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
                {
                    ShutdownGame(&game);
                    currentGameState = MENU;
                }
            } break;

            case LEADERBOARD:
            {
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    currentGameState = MENU;
                }
            } break;

            case CREDITS:
            {
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    currentGameState = MENU;
                }
            } break;

            default: break;
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            if ((currentGameState == PLAYING || currentGameState == GAME_OVER) && game.allocatedPlayersArrayBase != NULL && numPlayersSelectedInMenu > 0) {

                 for (int i = 0; i < numPlayersSelectedInMenu; ++i) {
                     Player* player = &game.allocatedPlayersArrayBase[i];

                     if (player->lives > 0) {
                        player->screenPosition = CalculatePlayerPosition(player->originalIndex, numPlayersSelectedInMenu, playerPositionsCenter, playerPositionsRadius);

                        Color nameColor = DARKGRAY;
                        Color lifeColor = (player->lives <= 1) ? RED : BLACK;

                        if (currentGameState == PLAYING && game.currentPlayer != NULL && player == game.currentPlayer) {
                            nameColor = DARKBLUE;
                        }

                        DrawPlayerInfo(player, textFont, nameColor, lifeColor);
                     }
                 }

            } else if ((currentGameState == PLAYING || currentGameState == GAME_OVER) && (game.allocatedPlayersArrayBase == NULL || numPlayersSelectedInMenu == 0)) {
                 DrawText("Erro: Jogadores nao inicializados corretamente.", 20, 20, 20, RED);
            }


            switch (currentGameState)
            {
                case MENU:
                {
                    ClearBackground(DARKGRAY);
                    const char* menuTitle = "BOMB PARTY";
                    Vector2 titlePos = {GetScreenWidth()/2 - MeasureText(menuTitle, 40)/2, GetScreenHeight()/3};
                    DrawText(menuTitle, titlePos.x, titlePos.y, 40, RAYWHITE);

                    const char *options[] = { "JOGAR", "LEADERBOARD", "CRÉDITOS" };
                    int startY = GetScreenHeight()/2;
                    for (int i = 0; i < maxMenuOptions; i++)
                    {
                        Color color = (i == menuOption) ? YELLOW : RAYWHITE;
                        int textWidth = MeasureText(options[i], 30);
                        DrawText(options[i], GetScreenWidth()/2 - textWidth/2, startY + i * 40, 30, color);
                    }
                } break;

                case SELECT_PLAYERS:
                {
                    ClearBackground(BLACK);
                    const char* selectPlayersTitle = "Selecione o número de jogadores";
                    Vector2 selectPlayersTitlePos = {GetScreenWidth()/2 - MeasureText(selectPlayersTitle, 30)/2, 50};
                    DrawText(selectPlayersTitle, selectPlayersTitlePos.x, selectPlayersTitlePos.y, 30, RAYWHITE);

                    const char *options[] = { "2 Jogadores", "3 Jogadores", "4 Jogadores" };

                    int startY = 120;
                    for (int i = 0; i < totalPlayerOptions; i++)
                    {
                        Color color = (i == selectedPlayersIndex) ? YELLOW : GRAY;
                        int textWidth = MeasureText(options[i], 25);
                        DrawText(options[i], GetScreenWidth()/2 - textWidth/2, startY + i * 40, 25, color);
                    }
                    DrawText("<- Voltar (BACKSPACE)", 20, GetScreenHeight() - 30, 20, RAYWHITE);
                } break;

                case SELECT_MODE:
                {
                    ClearBackground(DARKGRAY);
                    const char* selectModeTitle = "Selecione o modo de jogo";
                    Vector2 selectModeTitlePos = {GetScreenWidth()/2 - MeasureText(selectModeTitle, 30)/2, 50};
                    DrawText(selectModeTitle, selectModeTitlePos.x, selectModeTitlePos.y, 30, RAYWHITE);

                    const char *modes[] = { "Normal", "Louco" };

                    int startY = 120;
                    for (int i = 0; i < 2; i++)
                    {
                        Color color = (i == selectedMode) ? SKYBLUE : LIGHTGRAY;
                        int textWidth = MeasureText(modes[i], 25);
                        DrawText(modes[i], GetScreenWidth()/2 - textWidth/2, startY + i * 40, 25, color);
                    }
                    DrawText("<- Voltar (BACKSPACE)", 20, GetScreenHeight() - 30, 20, RAYWHITE);
                } break;

                case PLAYING:
                {
                    Rectangle inputBounds = {currentActualWidth/2 - 150, currentActualHeight - 80, 300, 40 };
                    // GuiTextBox is now only for drawing, input handling is in UpdatePlayingState
                    GuiTextBox(inputBounds, playerInput, MAX_PLAYER_INPUT_CHARS, playerInputEditMode);

                    // Draw the arrow
                    if (arrowTexture.id != 0 && game.currentPlayer != NULL) {
                         Vector2 arrowPivot = playerPositionsCenter;
                         Vector2 targetPlayerPos = game.currentPlayer->screenPosition;

                         Vector2 direction = {
                            targetPlayerPos.x - arrowPivot.x,
                            targetPlayerPos.y - arrowPivot.y
                         };

                         float angle_radians = atan2f(direction.y, direction.x);
                         float angle_degrees = angle_radians * RAD2DEG;
                         float arrowDrawingRotation = angle_degrees + 90.0f;

                         float arrowScale = 0.4f;

                         Rectangle sourceRecArrow = { 0.0f, 0.0f, (float)arrowTexture.width, (float)arrowTexture.height };
                         Rectangle destRecArrow = { arrowPivot.x, arrowPivot.y, arrowTexture.width * arrowScale, arrowTexture.height * arrowScale };
                         Vector2 originArrow = { (arrowTexture.width * arrowScale) / 2.0f, (arrowTexture.height * arrowScale) / 2.0f };

                         DrawTexturePro(arrowTexture, sourceRecArrow, destRecArrow, originArrow, arrowDrawingRotation, WHITE);
                    } else if (game.currentPlayer == NULL && game.numPlayers > 0) {
                         TraceLog(LOG_WARNING, "PLAYING: game.currentPlayer is NULL but numPlayers > 0.");
                    }


                    // Draw bomb and timer
                    float bombScale = 0.3f;
                    float bombRotation = 0.0f;

                    Vector2 bombPosition = {
                        currentActualWidth / 2 - (bombTexture.width * bombScale) / 2,
                        currentActualHeight / 2 - (bombTexture.height * bombScale) / 2
                    };

                    float sparkScale = 0.05f;
                    float sparkRotation = -30.0f + (float)GetTime() * 10.0f;
                    Vector2 sparkPosition = {
                        bombPosition.x + bombTexture.width * bombScale * 0.7f,
                        bombPosition.y + bombTexture.height * bombScale * 0.5f
                    };

                    if (bombTexture.id != 0) DrawTextureEx(bombTexture, bombPosition, bombRotation, bombScale, WHITE);
                    if (sparkTexture.id != 0 && bombTexture.id != 0 && game.bombTimer <= 5.0f && fmod((float)GetTime(), 0.5f) < 0.25f) {
                        DrawTextureEx(sparkTexture, sparkPosition, sparkRotation, sparkScale, WHITE);
                    }


                    DrawTextEx(textFont, TextFormat("Timer: %.1f", game.bombTimer), (Vector2){currentActualWidth - 180, 10}, 25, 0, (game.bombTimer <= 5.0f ? RED : DARKGRAY));

                    if (game.currentSyllable != NULL) {
                        Vector2 syllablePos = {currentActualWidth/2 - MeasureTextEx(textFont, game.currentSyllable, 60, 0).x/2, currentActualHeight/2 - 80 };
                        DrawTextEx(textFont, game.currentSyllable, syllablePos, 60, 0, BLUE);
                    } else {
                        DrawTextEx(textFont, "Sem Silaba!", (Vector2){currentActualWidth/2 - MeasureTextEx(textFont, "Sem Silaba!", 30, 0).x/2, currentActualHeight/2 - 80}, 30, 0, RED);
                    }

                } break;

                case GAME_OVER:
                {
                    ClearBackground(RAYWHITE);

                    const char* gameOverText = "Fim de Jogo!";
                    char winnerText[100] = {0};

                    if (game.numPlayers == 1 && game.firstPlayer != NULL) {
                         Player* winner = game.firstPlayer;
                         if (winner != NULL) {
                             snprintf(winnerText, sizeof(winnerText), "%s venceu!", winner->name);
                         } else {
                             strncpy(winnerText, "Erro ao determinar vencedor.", sizeof(winnerText) -1);
                             winnerText[sizeof(winnerText)-1] = '\0';
                         }

                    } else if (game.numPlayers == 0) {
                         strncpy(winnerText, "Todos foram eliminados!", sizeof(winnerText) -1);
                         winnerText[sizeof(winnerText)-1] = '\0';
                    }
                    else {
                         strncpy(winnerText, "Nenhum vencedor claro (erro ou multiplos jogadores restantes).", sizeof(winnerText) -1);
                         winnerText[sizeof(winnerText)-1] = '\0';
                    }

                    DrawTextEx(textFont, gameOverText, (Vector2){currentActualWidth/2 - MeasureTextEx(textFont, gameOverText, 40, 0).x/2, currentActualHeight/3}, 40, 0, DARKGRAY);
                    Vector2 winnerTextPos = {currentActualWidth/2 - MeasureTextEx(textFont, winnerText, 30, 0).x/2, currentActualHeight/3 + 60};
                    DrawTextEx(textFont, winnerText, winnerTextPos, 30, 0, BLUE);

                    const char* restartText = "Pressione ENTER ou ESPACO para Voltar ao Menu";
                    Vector2 restartTextPos = {currentActualWidth/2 - MeasureTextEx(textFont, restartText, 20, 0).x/2, currentActualHeight/2};
                    DrawTextEx(textFont, restartText, restartTextPos, 20, 0, GRAY);

                } break;

                case LEADERBOARD:
                {
                    ClearBackground(LIGHTGRAY);
                    DrawText("LEADERBOARD", GetScreenWidth()/2 - MeasureText("LEADERBOARD", 40)/2, GetScreenHeight()/3, 40, BLACK);
                    DrawText("<- Voltar (BACKSPACE)", 20, GetScreenHeight() - 30, 20, DARKGRAY);
                } break;

                case CREDITS:
                {
                    ClearBackground(GRAY);
                    DrawText("CREDITOS", GetScreenWidth()/2 - MeasureText("CREDITOS", 40)/2, GetScreenHeight()/3, 40, BLACK);
                    DrawText("<- Voltar (BACKSPACE)", 20, GetScreenHeight() - 30, 20, DARKGRAY);
                } break;

                default: break;
            }

            DrawFPS(10, currentActualHeight - 20);

        EndDrawing();
    }

    ShutdownGame(&game);

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