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
#include <stdbool.h>

#include "wordlist.h"
#include "game.h"
#include "player.h"
#include "leaderboard.h"

#define MAX_INPUT_CHARS 32

Vector2 playerPositionsCenter;
float playerPositionsRadius = 250.0f;

GameManager game = { 0 };

Rectangle frameRec = { 0.0f, 0.0f, 0.0f, 0.0f };
int currentFrame = 0;
int framesCounter = 0;
int framesSpeed = 8;
Texture2D playerSprite1 = {0};

Rectangle normalFireFrameRec = { 0.0f, 0.0f, 0.0f, 0.0f }; 
int normalFireCurrentFrame = 0; 
int normalFireFramesCounter = 0;
int normalFireFramesSpeed = 8; 
Texture2D normalFireTexture = {0};

int main(void)
{
    int display = GetCurrentMonitor();
    const int initialScreenWidth = GetMonitorPhysicalWidth(display);
    const int initialScreenHeight = GetMonitorPhysicalHeight(display);

    srand(time(NULL));

    InitWindow(initialScreenWidth, initialScreenHeight, "BombParty");
    char name[MAX_INPUT_CHARS + 1] = "\0";
    int letterCount = 0;

    Rectangle textBox = {GetScreenWidth()/2.0f - 150, 120 + 2 * 40 + 50, 300, 50};
    bool mouseOnText = false;

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

    Texture2D arrowTexture = { 0 };
    Texture2D normalModeBackgroundTexture = {0};
    Texture2D menuBackgroundTexture = {0};
    Texture2D titleTexture = {0};
    Texture2D backgroundTexture = {0};

    arrowTexture = LoadTexture("resources/textures/arrow.png");
    if (arrowTexture.id == 0) TraceLog(LOG_WARNING, "Failed to load arrow texture.");

    normalModeBackgroundTexture = LoadTexture("resources/textures/normalModeBackground.jpg");
    if (normalModeBackgroundTexture.id == 0) TraceLog(LOG_WARNING, "Failed to load normalModeBackground texture.");

    menuBackgroundTexture = LoadTexture("resources/textures/menuBackground.png");
    if (menuBackgroundTexture.id == 0) TraceLog(LOG_WARNING, "Failed to load menuBackground texture.");

    titleTexture = LoadTexture("resources/textures/title.png");
    if (titleTexture.id == 0) TraceLog(LOG_WARNING, "Failed to load title texture.");


    normalFireTexture = LoadTexture("resources/textures/normalFire.png");
    if (normalFireTexture.id == 0) {
        TraceLog(LOG_WARNING, "Failed to load normalFire texture.");
    } else {
        normalFireFrameRec = (Rectangle){ 0.0f, 0.0f, (float)normalFireTexture.width/3, (float)normalFireTexture.height/3 };
    }

    playerSprite1 = LoadTexture("resources/textures/playerSprite1.png");
    if (playerSprite1.id == 0) {
        TraceLog(LOG_WARNING, "Failed to load playerSprite1 texture.");
    } else {
        frameRec = (Rectangle){ 0.0f, 0.0f, (float)playerSprite1.width/5, (float)playerSprite1.height/4};
    }

    WordList wordList = { NULL, NULL, 0 };
    wordList = LoadWordList("resources/data/palavras.txt");
    if (wordList.count == 0) TraceLog(LOG_FATAL, "Failed to load word list or word list is empty.");

    char playerInput[MAX_PLAYER_INPUT_CHARS + 1] = { 0 };
    bool playerInputEditMode = false;

    GameState currentGameState = MENU;

    int menuOption = 0;
    const int maxMenuOptions = 3;

    int selectedPlayersIndex = 0;
    const int totalPlayerOptions = 3;

    int numPlayersSelectedInMenu = 0;

    float initialBombTime_value = 15.0f;

    LoadLeaderboard();

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
                        playerInputEditMode = true;
                        playerInput[0] = '\0';
                        GuiSetState(STATE_NORMAL);
                        if(selectedMode == 1)
                        {
                            currentGameState = TOPIC_INPUT;
                        }
                        else
                        {
                            currentGameState = PLAYING;
                        }

                    }
                }

                if (IsKeyPressed(KEY_BACKSPACE)) {
                    currentGameState = SELECT_PLAYERS;
                }
            } break;

            case TOPIC_INPUT:
            {
                if (CheckCollisionPointRec(GetMousePosition(), textBox)) mouseOnText = true;
                else mouseOnText = false;

                if (mouseOnText)
                {
                    // Set the window's cursor to the I-Beam
                    SetMouseCursor(MOUSE_CURSOR_IBEAM);

                    // Get char pressed (unicode character) on the queue
                    int key = GetCharPressed();

                    // Check if more characters have been pressed on the same frame
                    while (key > 0)
                    {
                        // NOTE: Only allow keys in range [32..125]
                        if ((key >= 32) && (key <= 125) && (letterCount < MAX_INPUT_CHARS))
                        {
                            name[letterCount] = (char)key;
                            name[letterCount+1] = '\0'; // Add null terminator at the end of the string.
                            letterCount++;
                        }

                        key = GetCharPressed();  // Check next character in the queue
                    }

                    if (IsKeyPressed(KEY_BACKSPACE))
                    {
                        letterCount--;
                        if (letterCount < 0) letterCount = 0;
                        name[letterCount] = '\0';
                    }
                }
                else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

                if (mouseOnText) framesCounter++;
                else framesCounter = 0;

                if (IsKeyPressed(KEY_ENTER))
                {
                    currentGameState = PLAYING;
                }
            } break;

            case PLAYING:
            {
                framesCounter++;
                if (framesCounter >= (60/framesSpeed)){
                    framesCounter = 0;
                    currentFrame++;

                    if(currentFrame > 3) currentFrame = 0;

                    frameRec.x = (float)2 * (float)playerSprite1.width/5;
                    frameRec.y = (float)currentFrame * (float)playerSprite1.height/4;
                }

                if (game.currentPlayer == NULL || game.numPlayers <= 0) {
                    TraceLog(LOG_WARNING, "PLAYING state entered with no current player or zero players. Transitioning to GAME_OVER.");
                    currentGameState = GAME_OVER;
                } else {
                    currentGameState = UpdatePlayingState(&game, deltaTime, playerInput, &playerInputEditMode, &wordList);
                }

                int normalFireAnimationSequence[] = {0, 3, 6, 1, 4}; // sequência de índices
                int normalFireAnimationLength = sizeof(normalFireAnimationSequence) / sizeof(normalFireAnimationSequence[0]);

                normalFireFramesCounter++;
                if (normalFireFramesCounter >= (60/normalFireFramesSpeed)){
                    normalFireFramesCounter = 0;
                    normalFireCurrentFrame++; 

                    if(normalFireCurrentFrame >= normalFireAnimationLength) { 
                        normalFireCurrentFrame = 0;
                    }

                    int currentSpritesheetFrameIndex = normalFireAnimationSequence[normalFireCurrentFrame];

                    normalFireFrameRec.x = (float)(currentSpritesheetFrameIndex % 3) * normalFireFrameRec.width; // coluna
                    normalFireFrameRec.y = (float)(currentSpritesheetFrameIndex / 3) * normalFireFrameRec.height; // linha
                }

            } break;

            case GAME_OVER:
            {
                if (game.numPlayers == 1 && game.firstPlayer != NULL) {
                    Player* winner = game.firstPlayer;
                    AddToLeaderboard(winner->name, winner->score);
                    TraceLog(LOG_INFO, TextFormat("Vencedor %s com score %d processado para leaderboard.", winner->name, winner->score));
                } else if (game.numPlayers == 0) {
                    TraceLog(LOG_INFO, "Nenhum vencedor para adicionar ao leaderboard.");
                }
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

            Texture2D backgroundToDraw = {0};
            if (currentGameState == MENU) {
                backgroundToDraw = menuBackgroundTexture;
            } else if (currentGameState == PLAYING || currentGameState == GAME_OVER) {
                 backgroundToDraw = normalModeBackgroundTexture;
            }

            if (backgroundToDraw.id != 0) {
                float screenRatio = (float)currentActualWidth / (float)currentActualHeight;
                float textureRatio = (float)backgroundToDraw.width / (float)backgroundToDraw.height;

                Rectangle sourceRec = { 0.0f, 0.0f, (float)backgroundToDraw.width, (float)backgroundToDraw.height };
                Rectangle destRec = { 0.0f, 0.0f, (float)currentActualWidth, (float)currentActualHeight };
                Vector2 origin = { 0.0f, 0.0f };

                float scale = 1.0f;
                if (screenRatio > textureRatio) {
                    scale = (float)currentActualHeight / (float)backgroundToDraw.height;
                } else {
                    scale = (float)currentActualWidth / (float)backgroundToDraw.width;
                }

                destRec.width = (float)backgroundToDraw.width * scale;
                destRec.height = (float)backgroundToDraw.height * scale;
                destRec.x = (float)currentActualWidth / 2.0f - destRec.width / 2.0f;
                destRec.y = (float)currentActualHeight / 2.0f - destRec.height / 2.0f;

                DrawTexturePro(backgroundToDraw, sourceRec, destRec, origin, 0.0f, WHITE);
            }

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

                        DrawPlayerInfo(player, textFont, nameColor, lifeColor, playerSprite1, frameRec);
                    }
                }

            } else if ((currentGameState == PLAYING || currentGameState == GAME_OVER) && (game.allocatedPlayersArrayBase == NULL || numPlayersSelectedInMenu == 0)) {
                DrawText("Erro: Jogadores nao inicializados corretamente.", 20, 20, 20, RED);
            }

            switch (currentGameState)
            {
                case MENU:
                {

                    if (titleTexture.id != 0) {
                        float targetWidth = GetScreenWidth() * 0.3f; 
                        float scale = targetWidth / titleTexture.width; 

                        float titleWidthScaled = titleTexture.width * scale;
                        float titleHeightScaled = titleTexture.height * scale;

                        Vector2 titlePos = {
                            GetScreenWidth()/2.0f - titleWidthScaled/2.0f,
                            GetScreenHeight()/16.0f 
                        };

                        DrawTexturePro(titleTexture,
                                       (Rectangle){0, 0, (float)titleTexture.width, (float)titleTexture.height}, 
                                       (Rectangle){titlePos.x, titlePos.y, titleWidthScaled, titleHeightScaled}, 
                                       (Vector2){0,0}, // origem para rotação
                                       0.0f, // rotação
                                       WHITE); // cor

                    }


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

                case TOPIC_INPUT:
                {
                    ClearBackground(GREEN);

                    int startY = 120;
                    // --- Adicionar o input de texto aqui ---

                    // Posição e tamanho da caixa de texto (ajuste conforme necessário)
                    Rectangle nameInputBox = { GetScreenWidth()/2.0f - 150, startY + 2 * 40 + 50, 300, 50 }; // Posicionado abaixo das opções de modo

                    DrawText("Digite o tema da partida:", GetScreenWidth()/2 - MeasureText("Digite o tema da partida:", 20)/2, nameInputBox.y - 30, 20, RAYWHITE);

                    DrawRectangleRec(nameInputBox, LIGHTGRAY);
                     if (mouseOnText) {
                        DrawRectangleLines((int)nameInputBox.x, (int)nameInputBox.y, (int)nameInputBox.width, (int)nameInputBox.height, RED);
                    } else {
                        DrawRectangleLines((int)nameInputBox.x, (int)nameInputBox.y, (int)nameInputBox.width, (int)nameInputBox.height, DARKGRAY);
                    }

                    DrawTextEx(textFont, name, (Vector2){nameInputBox.x + 5, nameInputBox.y + (nameInputBox.height - textFont.baseSize)/2}, textFont.baseSize, 0, MAROON);

                    DrawText(TextFormat("CARACTERES: %i/%i", letterCount, MAX_INPUT_CHARS), GetScreenWidth()/2 - MeasureText(TextFormat("CARACTERES: %i/%i", letterCount, MAX_INPUT_CHARS), 20)/2, nameInputBox.y + nameInputBox.height + 10, 20, DARKGRAY);

                    if (mouseOnText && letterCount < MAX_INPUT_CHARS)
                    {
                         if (((framesCounter/20)%2) == 0) {
                             Vector2 cursor_pos = MeasureTextEx(textFont, name, textFont.baseSize, 0);
                             DrawTextEx(textFont, "_", (Vector2){nameInputBox.x + 5 + cursor_pos.x, nameInputBox.y + (nameInputBox.height - textFont.baseSize)/2}, textFont.baseSize, 0, MAROON);
                         }
                    }
                    else if (letterCount >= MAX_INPUT_CHARS) {
                         DrawText("Máximo de caracteres atingido", GetScreenWidth()/2 - MeasureText("Máximo de caracteres atingido", 20)/2, nameInputBox.y + nameInputBox.height + 10, 20, GRAY);
                    }

                    // --- Fim do input de texto ---
                }break;

                case PLAYING:
                {
                    DrawTexture(backgroundTexture, 0, 0, WHITE);
                    Rectangle inputBounds = {currentActualWidth/2 - 150, currentActualHeight - 80, 300, 40 };
                    GuiTextBox(inputBounds, playerInput, MAX_PLAYER_INPUT_CHARS, playerInputEditMode);

                    if (arrowTexture.id != 0 && game.currentPlayer != NULL) {
                        Vector2 arrowPivot = playerPositionsCenter;
                        Vector2 targetPlayerPos = game.currentPlayer->screenPosition;

                        Vector2 direction = {
                            targetPlayerPos.x - arrowPivot.x,
                            targetPlayerPos.y - arrowPivot.y
                        };

                        float angle_radians = atan2f(direction.y, direction.x);
                        float angle_degrees = angle_radians * RAD2DEG;
                        float arrowDrawingRotation = angle_degrees + 0.0f;

                        float arrowScale = 0.4f;

                        Rectangle sourceRecArrow = { 0.0f, 0.0f, (float)arrowTexture.width, (float)arrowTexture.height };
                        Rectangle destRecArrow = { arrowPivot.x, arrowPivot.y, arrowTexture.width * arrowScale, arrowTexture.height * arrowScale };
                        Vector2 originArrow = { (arrowTexture.width * arrowScale) / 2.0f, (arrowTexture.height * arrowScale) / 2.0f };

                        DrawTexturePro(arrowTexture, sourceRecArrow, destRecArrow, originArrow, arrowDrawingRotation, WHITE);
                    } else if (game.currentPlayer == NULL && currentGameState == PLAYING && game.numPlayers > 0) {
                         TraceLog(LOG_WARNING, "PLAYING: game.currentPlayer is NULL but numPlayers > 0. (Drawing)");
                    }

                    if (normalFireTexture.id != 0) {
                        Vector2 monsterPosition = {
                            currentActualWidth / 2.0f - normalFireFrameRec.width / 0.68f,
                            currentActualHeight / 2.0f - normalFireFrameRec.height / 2.0f
                        };
                        float monsterScale = 2.0f;
                        Rectangle monsterDestRec = {monsterPosition.x, monsterPosition.y, normalFireFrameRec.width * monsterScale, normalFireFrameRec.height * monsterScale};
                        Vector2 monsterOrigin = {0,0};

                        DrawTexturePro(normalFireTexture, normalFireFrameRec, monsterDestRec, monsterOrigin, 0.0f, WHITE); 
                    }

                    DrawTextEx(textFont, TextFormat("Timer: %.1f", game.bombTimer), (Vector2){currentActualWidth - 180, 10}, 25, 0, (game.bombTimer <= 5.0f ? RED : DARKGRAY));

                    if (game.currentSyllable != NULL) {
                        Vector2 syllablePos = {currentActualWidth/2 - MeasureTextEx(textFont, game.currentSyllable, 60, 0).x/2, currentActualHeight/2 - 120 };
                        DrawTextEx(textFont, game.currentSyllable, syllablePos, 60, 0, BLUE);
                    } else {
                        DrawTextEx(textFont, "Sem Silaba!", (Vector2){currentActualWidth/2 - MeasureTextEx(textFont, "Sem Silaba!", 30, 0).x/2, currentActualHeight/2 - 120}, 30, 0, RED);
                    }
                } break;

                case GAME_OVER:
                {
                    const char* gameOverText = "Fim de Jogo!";
                    char winnerText[100] = {0};

                    if (game.numPlayers == 1 && game.firstPlayer != NULL) {
                        Player* winner = game.firstPlayer;
                        if (winner != NULL) {
                            snprintf(winnerText, sizeof(winnerText), "%s venceu! Score: %d", winner->name, winner->score);
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
                    ClearBackground(RAYWHITE);
                    DrawLeaderboard(textFont, currentActualWidth, currentActualHeight);
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

    if (arrowTexture.id != 0) UnloadTexture(arrowTexture);
    if (normalModeBackgroundTexture.id != 0) UnloadTexture(normalModeBackgroundTexture);
    if (menuBackgroundTexture.id != 0) UnloadTexture(menuBackgroundTexture);
    if (menuBackgroundTexture.id != 0) UnloadTexture(titleTexture);
    if (playerSprite1.id != 0) UnloadTexture(playerSprite1);
    if (normalFireTexture.id != 0) UnloadTexture(normalFireTexture);

    CloseWindow();

    return 0;
}