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
#include "player.h" // Inclua player.h para a declaração de DrawPlayerInfo
#include "leaderboard.h"

#define MAX_INPUT_CHARS 9

Vector2 playerPositionsCenter;
float playerPositionsRadius = 250.0f;

GameManager game = { 0 };

// Variáveis de animação para o wizardLittle
Rectangle frameRec = { 0.0f, 0.0f, 0.0f, 0.0f }; // Inicializado com 0, será preenchido após carregar a textura
int currentFrame = 0;
int framesCounter = 0;
int framesSpeed = 8; // Ajuste a velocidade conforme necessário
Texture2D wizardLittle = {0}; // Declarar a textura aqui para estar disponível globalmente no main

int main(void)
{
    int display = GetCurrentMonitor();
    const int initialScreenWidth = GetMonitorPhysicalWidth(display);
    const int initialScreenHeight = GetMonitorPhysicalHeight(display);

    srand(time(NULL));

    InitWindow(initialScreenWidth, initialScreenHeight, "BombParty");
    char name[MAX_INPUT_CHARS + 1] = "\0";
    int letterCount = 0;

    Rectangle textBox = { initialScreenHeight/2.0f - 100, 180, 225, 50 };
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

    Texture2D bombTexture = { 0 };
    Texture2D sparkTexture = { 0 };
    Texture2D arrowTexture = { 0 };
    Texture2D normalModeBackgroundTexture = {0}; // Alterei o nome da variável para ser mais específico
    // Texture2D wizardLittle = {0}; // Movido para declaração global

    bombTexture = LoadTexture("resources/textures/bomb.png");
    if (bombTexture.id == 0) TraceLog(LOG_WARNING, "Failed to load bomb texture.");
    sparkTexture = LoadTexture("resources/textures/spark.png");
    if (sparkTexture.id == 0) TraceLog(LOG_WARNING, "Failed to load spark texture.");
    arrowTexture = LoadTexture("resources/textures/arrow.png");
    if (arrowTexture.id == 0) TraceLog(LOG_WARNING, "Failed to load arrow texture.");
    // Carregar a textura de background correta
    normalModeBackgroundTexture = LoadTexture("resources/textures/normalModeBackground.jpg"); // Use o nome do seu arquivo de background
    if (normalModeBackgroundTexture.id == 0) TraceLog(LOG_WARNING, "Failed to load normalModeBackground texture.");
    wizardLittle = LoadTexture("resources/textures/wizardLittle.png");
    if (wizardLittle.id == 0) {
        TraceLog(LOG_WARNING, "Failed to load wizardLittle texture.");
    } else {
        // Inicializar frameRec após carregar a textura
        frameRec = (Rectangle){ 0.0f, 0.0f, (float)wizardLittle.width/5, (float)wizardLittle.height/4};
    }


    // Vector2 wizardLittlePosition = { 350.0f, 280.0f }; // Esta variável não é mais necessária aqui, o DrawPlayerInfo calculará a posição
    // Rectangle frameRec = { 0.0f, 0.0f, (float)wizardLittle.width/5, (float)wizardLittle.height/4}; // Movido/ajustado
    // int currentFrame = 0; // Movido para declaração global
    // int framesCounter = 0; // Movido para declaração global
    // int framesSpeed = 8; // Movido para declaração global


    WordList wordList = { NULL, NULL, 0 };
    wordList = LoadWordList("resources/data/palavras.txt");
    if (wordList.count == 0) TraceLog(LOG_FATAL, "Failed to load word list or word list is empty.");


    char playerInput[MAX_PLAYER_INPUT_CHARS + 1] = { 0 };
    bool playerInputEditMode = false;

    GameState currentGameState = MENU;
    // int selectedMode = 0; // REMOVIDO: Esta variável está declarada externamente em game.h

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
                        // Removido o input de texto aqui, pois a lógica de input deve ser global ou em uma tela dedicada de entrada de nome.
                        // Se você precisa da entrada de nome, mova essa lógica para um estado/tela apropriado (ex: SELECT_MODE ou uma nova tela).
                    } else {
                        initialBombTime_value = 15.0f;
                    }

                    // Certifique-se de que a variável 'name' tem o nome do jogador aqui, se necessário para InitializeGame
                    // Se a entrada de nome for em outra tela, passe o nome de forma apropriada.
                    InitializeGame(&game, numPlayersSelectedInMenu, initialBombTime_value, &wordList);

                    if (game.firstPlayer == NULL || game.numPlayers == 0) {
                        TraceLog(LOG_ERROR, "Falha ao iniciar o jogo apos selecao de modo.");
                        currentGameState = MENU;
                    } else {
                        playerInputEditMode = true;
                        playerInput[0] = '\0';
                        GuiSetState(STATE_NORMAL);
                        currentGameState = PLAYING;
                    }
                }

                if (IsKeyPressed(KEY_BACKSPACE)) {
                    currentGameState = SELECT_PLAYERS;
                }
                 // Lógica de input de texto (movida/removida dependendo de onde você quer a entrada de nome)
                 // Certifique-se que a variável 'name' e a lógica de mouseOnText/letterCount estão no escopo correto se usadas aqui.
            } break;

            case PLAYING:
            {
                // Lógica de animação do wizardLittle (permanece aqui para atualizar a frame globalmente)
                framesCounter++;
                if (framesCounter >= (60/framesSpeed)){
                    framesCounter = 0;
                    currentFrame++;

                    // Sua lógica de animação vertical (opção 1)
                    if(currentFrame > 3) currentFrame = 0; // Anima 4 frames (0, 1, 2, 3)

                    // spritesheet 5 colunas, 4 linhas
                    // A posição X fica fixa na terceira coluna (índice 2)
                    frameRec.x = (float)2 * (float)wizardLittle.width/5;
                    // A posição Y muda diretamente com o currentFrame (linhas 0 a 3)
                    frameRec.y = (float)currentFrame * (float)wizardLittle.height/4;
                }


                if (game.currentPlayer == NULL || game.numPlayers <= 0) {
                    TraceLog(LOG_WARNING, "PLAYING state entered with no current player or zero players. Transitioning to GAME_OVER.");
                    currentGameState = GAME_OVER;
                } else {
                    currentGameState = UpdatePlayingState(&game, deltaTime, playerInput, &playerInputEditMode, &wordList);
                }

            } break;

            case GAME_OVER:
            {
                // --- Add winner to leaderboard using the function from leaderboard.c ---
                if (game.numPlayers == 1 && game.firstPlayer != NULL) {
                    Player* winner = game.firstPlayer;
                    AddToLeaderboard(winner->name, winner->score); // Call AddToLeaderboard
                    TraceLog(LOG_INFO, TextFormat("Vencedor %s com score %d processado para leaderboard.", winner->name, winner->score));
                } else if (game.numPlayers == 0) {
                    TraceLog(LOG_INFO, "Nenhum vencedor para adicionar ao leaderboard.");
                }
                // --- End Add winner to leaderboard ---
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
                {
                    ShutdownGame(&game);
                    currentGameState = MENU;
                }
            } break;

            case LEADERBOARD:
            {
                // No update logic needed here, just handle input for going back
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
            ClearBackground(RAYWHITE); // Limpa o fundo da tela (pode ser a cor de fundo padrão)

            // --- Desenhar o background para os estados PLAYING e GAME_OVER ---
            if (currentGameState == PLAYING || currentGameState == GAME_OVER)
            {
                 if (normalModeBackgroundTexture.id != 0) { // Verifica se a textura foi carregada
                    // Calcular a proporção da tela e da textura de background
                    float screenRatio = (float)currentActualWidth / (float)currentActualHeight;
                    float textureRatio = (float)normalModeBackgroundTexture.width / (float)normalModeBackgroundTexture.height;

                    Rectangle sourceRec = { 0.0f, 0.0f, (float)normalModeBackgroundTexture.width, (float)normalModeBackgroundTexture.height };
                    Rectangle destRec = { 0.0f, 0.0f, (float)currentActualWidth, (float)currentActualHeight };
                    Vector2 origin = { 0.0f, 0.0f };

                    // Calcular o escalonamento para cobrir a tela mantendo a proporção
                    float scale = 1.0f;
                    if (screenRatio > textureRatio) {
                        // A tela é mais larga que a textura, escalar pela altura
                        scale = (float)currentActualHeight / (float)normalModeBackgroundTexture.height;
                    } else {
                        // A tela é mais alta ou tem a mesma proporção, escalar pela largura
                        scale = (float)currentActualWidth / (float)normalModeBackgroundTexture.width;
                    }

                    // Ajustar o destino e origem para centralizar a textura escalonada
                    destRec.width = (float)normalModeBackgroundTexture.width * scale;
                    destRec.height = (float)normalModeBackgroundTexture.height * scale;
                    destRec.x = (float)currentActualWidth / 2.0f - destRec.width / 2.0f;
                    destRec.y = (float)currentActualHeight / 2.0f - destRec.height / 2.0f;

                    // Desenhar o background ajustado
                    DrawTexturePro(normalModeBackgroundTexture, sourceRec, destRec, origin, 0.0f, WHITE);
                 }
            }
            // --- Fim do desenho do background ---


            // 3. Desenha os jogadores e seus elementos (nomes, vidas, wizards)
            // Este bloco já itera sobre os jogadores e chama DrawPlayerInfo
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

                        // Chamar DrawPlayerInfo passando a textura do wizard e a frameRec atual
                        // Assumindo que DrawPlayerInfo foi atualizado para aceitar a textura e frameRec
                        DrawPlayerInfo(player, textFont, nameColor, lifeColor, wizardLittle, frameRec);
                    }
                }

            } else if ((currentGameState == PLAYING || currentGameState == GAME_OVER) && (game.allocatedPlayersArrayBase == NULL || numPlayersSelectedInMenu == 0)) {
                DrawText("Erro: Jogadores nao inicializados corretamente.", 20, 20, 20, RED);
            }


            // 4. Desenha elementos específicos de cada estado (menus, UI de jogo, etc.)
            switch (currentGameState)
            {
                case MENU:
                {
                    ClearBackground(DARKGRAY); // Fundo para o menu
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
                    ClearBackground(BLACK); // Fundo para seleção de jogadores
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
                     ClearBackground(DARKGRAY); // Fundo para seleção de modo

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

                    // Removido o input de texto desta seção no código anterior,
                    // se você adicionou uma tela de input separada, mova a lógica de desenho para lá.
                    // Se a entrada de nome ocorre AQUI, você precisará adicionar a lógica de desenho do input box
                    // e o texto digitado neste case SELECT_MODE, como estava no código anterior.
                     // Exemplo (re-adicione se o input de nome for nesta tela):
                     /*
                     int nameInputY = startY + 2 * 40 + 50;
                     Rectangle nameInputBox = { GetScreenWidth()/2.0f - 150, (float)nameInputY, 300, 50 };

                     DrawText("Digite seu nome:", GetScreenWidth()/2 - MeasureText("Digite seu nome:", 20)/2, (float)nameInputY - 30, 20, RAYWHITE);

                     DrawRectangleRec(nameInputBox, LIGHTGRAY);
                      if (mouseOnText) {
                         DrawRectangleLines((int)nameInputBox.x, (int)nameInputBox.y, (int)nameInputBox.width, (int)nameInputBox.height, RED);
                     } else {
                         DrawRectangleLines((int)nameInputBox.x, (int)nameInputBox.y, (int)nameInputBox.width, (int)nameInputBox.height, DARKGRAY);
                     }
                     DrawTextEx(textFont, name, (Vector2){nameInputBox.x + 5, nameInputBox.y + (nameInputBox.height - textFont.baseSize)/2}, textFont.baseSize, 0, MAROON);
                     DrawText(TextFormat("CARACTERES: %i/%i", letterCount, MAX_INPUT_CHARS), GetScreenWidth()/2 - MeasureText(TextFormat("CARACTERES: %i/%i", letterCount, MAX_INPUT_CHARS), 20)/2, nameInputBox.y + nameInputBox.height + 10, 20, DARKGRAY);
                      if (mouseOnText && letterCount < MAX_INPUT_CHARS) {
                          if (((framesCounter/20)%2) == 0) {
                              Vector2 cursor_pos = MeasureTextEx(textFont, name, textFont.baseSize, 0);
                              DrawTextEx(textFont, "_", (Vector2){nameInputBox.x + 5 + cursor_pos.x, nameInputBox.y + (nameInputBox.height - textFont.baseSize)/2}, textFont.baseSize, 0, MAROON);
                          }
                      } else if (letterCount >= MAX_INPUT_CHARS) {
                          DrawText("Máximo de caracteres atingido", GetScreenWidth()/2 - MeasureText("Máximo de caracteres atingido", 20)/2, nameInputBox.y + nameInputBox.height + 10, 20, GRAY);
                      }
                      */

                } break;
                 // Se você tem um estado TOPIC_INPUT, o conteúdo de input de texto deve estar lá
                /*
                case TOPIC_INPUT:
                {
                     // Desenhar background específico para TOPIC_INPUT se necessário
                     ClearBackground(MAGENTA); // Exemplo

                     int startY = 120;
                     Rectangle nameInputBox = { GetScreenWidth()/2.0f - 150, startY + 2 * 40 + 50, 300, 50 };

                     DrawText("Digite seu nome:", GetScreenWidth()/2 - MeasureText("Digite seu nome:", 20)/2, (float)nameInputBox.y - 30, 20, RAYWHITE);

                     DrawRectangleRec(nameInputBox, LIGHTGRAY);
                     if (mouseOnText) {
                         DrawRectangleLines((int)nameInputBox.x, (int)nameInputBox.y, (int)nameInputBox.width, (int)nameInputBox.height, RED);
                     } else {
                         DrawRectangleLines((int)nameInputBox.x, (int)nameInputBox.y, (int)nameInputBox.width, (int)nameInputBox.height, DARKGRAY);
                     }
                     DrawTextEx(textFont, name, (Vector2){nameInputBox.x + 5, nameInputBox.y + (nameInputBox.height - textFont.baseSize)/2}, textFont.baseSize, 0, MAROON);
                     DrawText(TextFormat("CARACTERES: %i/%i", letterCount, MAX_INPUT_CHARS), GetScreenWidth()/2 - MeasureText(TextFormat("CARACTERES: %i/%i", letterCount, MAX_INPUT_CHARS), 20)/2, nameInputBox.y + nameInputBox.height + 10, 20, DARKGRAY);
                      if (mouseOnText && letterCount < MAX_INPUT_CHARS) {
                           if (((framesCounter/20)%2) == 0) {
                               Vector2 cursor_pos = MeasureTextEx(textFont, name, textFont.baseSize, 0);
                               DrawTextEx(textFont, "_", (Vector2){nameInputBox.x + 5 + cursor_pos.x, nameInputBox.y + (nameInputBox.height - textFont.baseSize)/2}, textFont.baseSize, 0, MAROON);
                           }
                       } else if (letterCount >= MAX_INPUT_CHARS) {
                           DrawText("Máximo de caracteres atingido", GetScreenWidth()/2 - MeasureText("Máximo de caracteres atingido", 20)/2, nameInputBox.y + nameInputBox.height + 10, 20, GRAY);
                       }
                }
                break;
                */

                case PLAYING:
                {
                    // Elementos do jogo (bomba, seta, sílaba, input box) são desenhados aqui
                    // O background já foi desenhado no início da seção BeginDrawing.

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
                         // Este log pode ser removido ou ajustado se a transição para GAME_OVER for imediata
                         TraceLog(LOG_WARNING, "PLAYING: game.currentPlayer is NULL but numPlayers > 0. (Drawing)");
                    }


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

                    // A linha DrawTextureRec(wizardLittle, frameRec, wizardLittlePosition, WHITE); não é mais necessária aqui
                    // pois o wizard será desenhado dentro do DrawPlayerInfo para cada jogador.

                } break;

                case GAME_OVER:
                {
                    ClearBackground(RAYWHITE); // Desenha um fundo branco para o Game Over.

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
                    // O background do leaderboard é desenhado aqui
                     ClearBackground(RAYWHITE); // Ou outra cor/textura para o leaderboard
                    // --- Draw Leaderboard using the function from leaderboard.c ---
                    DrawLeaderboard(textFont, currentActualWidth, currentActualHeight);
                    // --- End Draw Leaderboard ---

                    // The back key handling is already in the update section for LEADERBOARD state
                } break;

                case CREDITS:
                {
                    // O background dos créditos é desenhado aqui
                    ClearBackground(GRAY);
                    DrawText("CREDITOS", GetScreenWidth()/2 - MeasureText("CREDITOS", 40)/2, GetScreenHeight()/3, 40, BLACK);
                    DrawText("<- Voltar (BACKSPACE)", 20, GetScreenHeight() - 30, 20, DARKGRAY);
                } break;

                default: break;
            }

            DrawFPS(10, currentActualHeight - 20);

        EndDrawing();
    }

    // Ensure game resources are shut down on window close
    ShutdownGame(&game);

    UnloadWordList(&wordList);

    if (customFont.texture.id != 0 && textFont.texture.id != GetFontDefault().texture.id)
    {
        UnloadFont(customFont);
    }

    if (bombTexture.id != 0) UnloadTexture(bombTexture);
    if (sparkTexture.id != 0) UnloadTexture(sparkTexture);
    if (arrowTexture.id != 0) UnloadTexture(arrowTexture);
    if (normalModeBackgroundTexture.id != 0) UnloadTexture(normalModeBackgroundTexture); // Descarregar a textura de background
    if (wizardLittle.id != 0) UnloadTexture(wizardLittle); // Descarregar a textura global

    CloseWindow();

    return 0;
}