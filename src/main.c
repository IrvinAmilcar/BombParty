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
int currentPlayerIndex = 0;
Vector2 playerPositionsCenter; // Centro para posicionar os jogadores
float playerPositionsRadius = 250.0f; // Raio do círculo dos jogadores

void InitializePlayers() {
    // Nomes de exemplo, você pode querer carregar isso de algum lugar ou pedir input
    char playerNames[NUM_PLAYERS][MAX_PLAYER_NAME_LEN] = {"Jogador 1", "Jogador 2"};

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        strncpy(players[i].name, playerNames[i], MAX_PLAYER_NAME_LEN - 1);
        players[i].name[MAX_PLAYER_NAME_LEN - 1] = '\0'; // Garante null termination
        players[i].lives = 2; // Começa com 2 vidas
        // A posição é calculada mais tarde, no loop principal se a tela mudar,
        // ou pode ser calculada aqui uma vez se a tela for fixa após InitWindow
    }
    currentPlayerIndex = 0; // Começa com o primeiro jogador
}


int main(void)
{
    int display = GetCurrentMonitor();
    // Use GetScreenWidth() e GetScreenHeight() dentro do loop principal
    // para pegar o tamanho atual da janela, caso ela seja redimensionada.
    // As variáveis screenWidth e screenHeight do monitor físico são boas para InitWindow,
    // mas o tamanho da janela atual pode ser diferente se não for fullscreen nativo ou se for redimensionada.
    // Manter GetScreenWidth/Height no loop de desenho/update é mais robusto.
    const int initialScreenWidth = GetMonitorPhysicalWidth(display);
    const int initialScreenHeight = GetMonitorPhysicalHeight(display); 


    srand(time(NULL));

    InitWindow(initialScreenWidth, initialScreenHeight, "BombParty"); 
    ToggleFullscreen();

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
    int selectPlayers = 2;
    typedef enum GameMode{
        NORMAL,
        LOUCO,
    } GameMode;
    //GameMode selectedMode = NORMAL;

    float bombTimer = 0.0f;
    float initialBombTime = 15.0f;

    int menuOption = 0;
    const int maxMenuOptions = 3;

    int selectedPlayers = 0;           // 0 para 2 jogadores, 1 para 3, 2 para 4
    const int totalPlayerOptions = 3;  // 3 opções: 2, 3, 4 jogadores

    int selectedMode = 0;             // 0 = Normal, 1 = Louco
    const int totalModeOptions = 2;

    bool modoLouco = false;



    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        int currentActualWidth = GetScreenWidth();
        int currentActualHeight = GetScreenHeight();

        playerPositionsCenter = (Vector2){ currentActualWidth / 2.0f, currentActualHeight / 2.0f };


        switch (currentGameState)
        {
            case MENU:
            {
                /*if (IsKeyPressed(KEY_ONE) && wordList.count > 0)
                {
                    currentGameState = PLAYING;
                    InitializePlayers(); 
                    bombTimer = initialBombTime;
                    currentSyllable = SelectRandomSyllable(&wordList);
                    TraceLog(LOG_INFO, TextFormat("Game started with syllable: %s", currentSyllable));

                    // Raygui: Ativa o modo de edição ao entrar no estado PLAYING
                    playerInputEditMode = true;
                    playerInput[0] = '\0'; // Limpa o buffer
                    GuiSetState(STATE_NORMAL); // Raygui: Garante que o estado visual do controle está normal

                    ResetUsedWordList(); //Reseta o arquivo txt ao iniciar um novo jogo!!!!
                }*/

                

                // Pressionar seta para baixo aumenta o índice
                if (IsKeyPressed(KEY_DOWN)) {
                    menuOption = (menuOption + 1) % maxMenuOptions;
                }

                // Pressionar seta para cima diminui o índice
                if (IsKeyPressed(KEY_UP)) {
                    menuOption = (menuOption - 1 + maxMenuOptions) % maxMenuOptions;
                }

                // Pressionar ENTER seleciona a opção
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
                if (IsKeyPressed(KEY_DOWN)) {
                    selectedPlayers = (selectedPlayers + 1) % totalPlayerOptions;
                }

                if (IsKeyPressed(KEY_UP)) {
                    selectedPlayers = (selectedPlayers - 1 + totalPlayerOptions) % totalPlayerOptions;
                }

                if (IsKeyPressed(KEY_ENTER)) {
                    // Aqui você pode salvar o número real de jogadores se quiser (2 + selectedPlayers)
                    currentGameState = SELECT_MODE;
                }

                // Voltar para o menu pressionando ESC
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    currentGameState = MENU;
                }
            } break;

            case SELECT_MODE:
            {
                if (IsKeyPressed(KEY_DOWN)) {
                    selectedMode = (selectedMode + 1) % totalModeOptions;
                }

                if (IsKeyPressed(KEY_UP)) {
                    selectedMode = (selectedMode - 1 + totalModeOptions) % totalModeOptions;
                }

                if (IsKeyPressed(KEY_ENTER)) {
                    // Aqui você pode armazenar o modo e ir para o gameplay
                    modoLouco = (selectedMode == 1);  // Salva o modo escolhido
                    currentGameState = PLAYING;
                }

                if (IsKeyPressed(KEY_BACKSPACE)) {
                    // Volta para seleção de jogadores
                    currentGameState = SELECT_PLAYERS;
                }
            } break;



            case PLAYING:
            {
                bombTimer -= GetFrameTime();

                if (bombTimer <= 0.0f)
                {
                    bombTimer = 0.0f;
                    TraceLog(LOG_INFO, TextFormat("Tempo esgotado para %s!", players[currentPlayerIndex].name));
                    players[currentPlayerIndex].lives--; // Jogador atual perde uma vida

                    if (players[currentPlayerIndex].lives <= 0) {
                         TraceLog(LOG_INFO, TextFormat("%s foi eliminado!", players[currentPlayerIndex].name));
                         // Lógica para eliminar o jogador ou verificar fim de jogo
                         // Por enquanto, apenas registra no log. Você precisaria de um mecanismo
                         // para pular jogadores eliminados na troca de turno ou terminar o jogo.
                         // Simplificado: Apenas perde a vida, mas o jogo continua.
                         // Uma lógica mais completa verificaria se sobrou apenas 1 jogador.
                    }

                    // Passa a bomba para o próximo jogador
                    currentPlayerIndex = (currentPlayerIndex + 1) % NUM_PLAYERS;
                    bombTimer = initialBombTime; // Reseta o timer para o próximo jogador
                    currentSyllable = SelectRandomSyllable(&wordList); // Nova sílaba para o próximo jogador
                    TraceLog(LOG_INFO, TextFormat("Turno de %s. Nova silaba: %s", players[currentPlayerIndex].name, currentSyllable));
                    playerInput[0] = '\0'; // Limpa o input para o novo turno
                }

                if (IsKeyPressed(KEY_TWO))
                {
                   currentGameState = GAME_OVER; // Sai do jogo manualmente para testar
                }

            } break;

            case GAME_OVER:
            {
                if (IsKeyPressed(KEY_THREE))
                {
                    // Resetar o arquivo txt se iniciarmos um novo jogo após o jogo anteriorS
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
                // Use cores diferentes para o jogador atual ou jogadores eliminados, se desejar
                Color nameColor = (i == currentPlayerIndex) ? DARKBLUE : DARKGRAY;
                Color lifeColor = (players[i].lives <= 1) ? RED : BLACK;
                DrawPlayerInfo(&players[i], textFont, nameColor, lifeColor);
            }

            switch (currentGameState)
            {
                case MENU:
                {
                    /*const char* menuText = "Pressione 1 para Comecar";
                    char wordCountText[64];

                    if (wordList.count > 0) {
                        snprintf(wordCountText, sizeof(wordCountText), " (%d palavras carregadas)", wordList.count);
                        menuText = TextFormat("Pressione 1 para Comecar%s", wordCountText);
                    } else {
                        menuText = "Erro: Lista de palavras nao carregada!";
                    }
                    DrawTextEx(textFont, "Bomb Party", (Vector2){currentActualWidth/2 - MeasureTextEx(textFont, "Bomb Party", 40, 0).x/2, currentActualHeight/3}, 40, 0, GRAY);
                    DrawTextEx(textFont, menuText, (Vector2){currentActualWidth/2 - MeasureTextEx(textFont, menuText, 20, 0).x/2, currentActualHeight/2}, 20, 0, DARKGRAY);
                    */
                    ClearBackground(DARKGRAY);
                    DrawText("MENU PRINCIPAL", 20, 20, 40, RAYWHITE);

                    const char *options[] = { "JOGAR", "LEADERBOARD", "CRÉDITOS" };

                    for (int i = 0; i < maxMenuOptions; i++)
                    {
                        Color color = (i == menuOption) ? YELLOW : RAYWHITE;
                        DrawText(options[i], 100, 100 + i * 40, 30, color);
                    }
                } break;

                case SELECT_PLAYERS:
                {
                    ClearBackground(BLACK);
                    DrawText("Selecione o número de jogadores", 100, 50, 30, RAYWHITE);

                    const char *options[] = { "2 Jogadores", "3 Jogadores", "4 Jogadores" };

                    for (int i = 0; i < totalPlayerOptions; i++)
                    {
                        Color color = (i == selectedPlayers) ? YELLOW : GRAY;
                        DrawText(options[i], 120, 120 + i * 40, 25, color);
                    }
                } break;

                case SELECT_MODE:
                {
                    ClearBackground(DARKGRAY);
                    DrawText("Selecione o modo de jogo", 100, 50, 30, RAYWHITE);

                    const char *modes[] = { "Normal", "Louco" };

                    for (int i = 0; i < totalModeOptions; i++)
                    {
                        Color color = (i == selectedMode) ? SKYBLUE : LIGHTGRAY;
                        DrawText(modes[i], 120, 120 + i * 40, 25, color);
                    }
                } break;



                case PLAYING:
                {
                    // DrawTextEx(textFont, "Estado: JOGANDO", (Vector2){10, 10}, 20, 0, BLACK); // (explicacao 15) Removido, player info já indica

                    if (currentSyllable != NULL && wordList.count > 0) {
                        Vector2 syllablePos = {currentActualWidth/2 - MeasureTextEx(textFont, currentSyllable, 60, 0).x/2, currentActualHeight/2 - 80 };
                        DrawTextEx(textFont, currentSyllable, syllablePos, 60, 0, BLUE);
                    } else {
                            DrawTextEx(textFont, "Sem Silaba!", (Vector2){currentActualWidth/2 - MeasureTextEx(textFont, "Sem Silaba!", 30, 0).x/2, currentActualHeight/2 - 80}, 30, 0, RED);
                    }


                    // Raygui: Define a área do campo de input
                    Rectangle inputBounds = {currentActualWidth/2 - 150, currentActualHeight - 80, 300, 40 };
                    // Raygui: Desenha o campo de input E processa o input do teclado se playerInputEditMode for true
                    // Retorna true quando Enter é pressionado E está em modo de edição
                    if (GuiTextBox(inputBounds, playerInput, MAX_PLAYER_INPUT_CHARS, playerInputEditMode)) {
                        TraceLog(LOG_INFO, TextFormat("Player submitted: '%s'", playerInput));

                        bool isValid = checkWord(playerInput, currentSyllable, &wordList);

                        if (isValid){
                            TraceLog(LOG_INFO, TextFormat("Palavra '%s' valida!", playerInput));
                            // Palavra válida: Passa o turno
                             currentPlayerIndex = (currentPlayerIndex + 1) % NUM_PLAYERS;
                             bombTimer = initialBombTime; // Reseta o timer para o próximo jogador
                             currentSyllable = SelectRandomSyllable(&wordList); // Nova sílaba para o próximo jogador
                             TraceLog(LOG_INFO, TextFormat("Turno de %s. Nova silaba: %s", players[currentPlayerIndex].name, currentSyllable));

                        } else {
                             TraceLog(LOG_INFO, TextFormat("Palavra '%s' invalida!", playerInput));
                            // Palavra inválida: Jogador atual perde uma vida
                             players[currentPlayerIndex].lives--;
                              TraceLog(LOG_INFO, TextFormat("%s perdeu uma vida. Vidas restantes: %d", players[currentPlayerIndex].name, players[currentPlayerIndex].lives));

                             if (players[currentPlayerIndex].lives <= 0) {
                                 TraceLog(LOG_INFO, TextFormat("%s foi eliminado!", players[currentPlayerIndex].name));
                                 // Aqui você implementaria a lógica de fim de jogo se apenas 1 jogador sobrar
                                 // Por enquanto, apenas registra no log e o jogo continua
                             }

                            // Mesmo com palavra inválida, o turno geralmente passa no Bomb Party original
                            // ou o jogador perde a vida e o turno continua para ele se ele tiver vidas?
                            // Vamos seguir a regra de passar o turno após tentar, válida ou não (como no tempo esgotado)
                             currentPlayerIndex = (currentPlayerIndex + 1) % NUM_PLAYERS;
                             bombTimer = initialBombTime; // Reseta o timer para o próximo jogador
                             currentSyllable = SelectRandomSyllable(&wordList); // Nova sílaba
                             TraceLog(LOG_INFO, TextFormat("Turno de %s. Nova silaba: %s", players[currentPlayerIndex].name, currentSyllable));
                        }

                         playerInput[0] = '\0'; // Reseta o input do usuário após a tentativa
                    }

                    if (arrowTexture.id != 0 && NUM_PLAYERS > 0) { // Garante que a textura existe e há jogadores
                         Vector2 arrowPivot = playerPositionsCenter; // Seta pivoteia no centro
                         Vector2 targetPlayerPos = players[currentPlayerIndex].screenPosition;

                         Vector2 direction = {
                            targetPlayerPos.x - arrowPivot.x,
                            targetPlayerPos.y - arrowPivot.y
                         };

                         // Calcula o ângulo em radianos
                         float angle_radians = atan2f(direction.y, direction.x);
                         float angle_degrees = angle_radians * RAD2DEG; // Raylib define RAD2DEG, não RAD2RAD, correção: usar RAD2DEG

                         //float angle_degrees = angle_radians * (180.0f / PI); // Alternativa manual

                         // Ajusta a rotação se a textura da seta não aponta para a direita (0 graus) por padrão.
                         // Se sua seta aponta para cima na textura, adicione -90.0f. Se aponta para baixo, +90.0f.
                         // Se aponta para a esquerda, +180.0f.
                         float arrowDrawingRotation = angle_degrees; // + OFFSET_DA_SUA_TEXTURA; // Exemplo: +0.0f se aponta para direita

                         float arrowScale = 0.4f; // Mantém a escala definida antes

                         // Define source, dest e origin para DrawTexturePro
                         Rectangle sourceRecArrow = { 0.0f, 0.0f, (float)arrowTexture.width, (float)arrowTexture.height };
                         // O destRec.x e destRec.y são a posição do PIVOT (center)
                         Rectangle destRecArrow = { arrowPivot.x, arrowPivot.y, arrowTexture.width * arrowScale, arrowTexture.height * arrowScale };
                         // O origin é o centro da seta escalonada
                         Vector2 originArrow = { (arrowTexture.width * arrowScale) / 2.0f, (arrowTexture.height * arrowScale) / 2.0f };

                         // Desenha a seta usando DrawTexturePro para controle total
                         DrawTexturePro(arrowTexture, sourceRecArrow, destRecArrow, originArrow, arrowDrawingRotation, WHITE);

                    }


                    float bombScale = 0.3f;
                    float bombRotation = 0.0f;

                    Vector2 bombPosition = {
                        currentActualWidth / 2 - (bombTexture.width * bombScale) / 2,
                        currentActualHeight / 2 - (bombTexture.height * bombScale) / 2
                    };

                    float sparkScale = 0.05f;
                    float sparkRotation = -30.0f; // * DEG2RAD; // Removido * DEG2RAD pois a rotação em DrawTextureEx/Pro é em GRAUS


                    Vector2 sparkPosition = {
                        bombPosition.x + 100,
                        bombPosition.y + 5
                    };

                    if (bombTexture.id != 0) DrawTextureEx(bombTexture, bombPosition, bombRotation, bombScale, WHITE);
                    if (sparkTexture.id != 0) DrawTextureEx(sparkTexture, sparkPosition, sparkRotation, sparkScale, WHITE); // Usando sparkRotation em graus agora


                    DrawTextEx(textFont, TextFormat("Timer: %.1f", bombTimer), (Vector2){currentActualWidth - 150, 10}, 25, 0, (bombTimer <= 5.0f ? RED : DARKGRAY));

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