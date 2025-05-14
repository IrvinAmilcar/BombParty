#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "resource_dir.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h> // (explicacao 1)

#include "wordlist.h" // (explicacao 21) Manter a inclusão, mas o uso será desativado
#include "game.h"
#include "player.h" // (explicacao 2)

// (explicacao 3)
#define NUM_PLAYERS 2
Player players[NUM_PLAYERS];
int currentPlayerIndex = 0;
Vector2 playerPositionsCenter; // Centro para posicionar os jogadores
float playerPositionsRadius = 250.0f; // Raio do círculo dos jogadores

// (explicacao 4)
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
    const int initialScreenHeight = GetMonitorPhysicalHeight(display); // (explicacao 5)


    srand(time(NULL));

    InitWindow(initialScreenWidth, initialScreenHeight, "BombParty"); // (explicacao 5)
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

    // (explicacao 22) Desativar o carregamento e a variável WordList
    //WordList wordList = { NULL, 0 };
    //wordList = LoadWordList("resources/data/palavras.txt");

    const char* currentSyllable = "TESTE"; // (explicacao 23) Valor fixo para a sílaba de teste

    char playerInput[MAX_PLAYER_INPUT_CHARS + 1] = { 0 };
    bool playerInputEditMode = false;

    GameState currentGameState = MENU;

    float bombTimer = 0.0f;
    float initialBombTime = 15.0f;

    SetTargetFPS(60);

    // (explicacao 6) Removido: int currentActualWidth = GetScreenWidth();
    // (explicacao 6) Removido: int currentActualHeight = GetScreenHeight();


    while (!WindowShouldClose())
    {
        // (explicacao 7) Obtém as dimensões atuais da janela dentro do loop
        int currentActualWidth = GetScreenWidth();
        int currentActualHeight = GetScreenHeight();

        // (explicacao 8) Calcula a posição central para o layout dos jogadores
        playerPositionsCenter = (Vector2){ currentActualWidth / 2.0f, currentActualHeight / 2.0f };


        switch (currentGameState)
        {
            case MENU:
            {
                // (explicacao 24) Removido a verificação de wordList.count
                if (IsKeyPressed(KEY_ONE)) // && wordList.count > 0)
                {
                    currentGameState = PLAYING;
                    InitializePlayers(); // (explicacao 9) Inicializa os jogadores
                    bombTimer = initialBombTime;
                    // (explicacao 25) Desativado a seleção de sílaba aleatória
                    //currentSyllable = SelectRandomSyllable(&wordList);
                    TraceLog(LOG_INFO, TextFormat("Game started with syllable: %s", currentSyllable));

                    // Raygui: Ativa o modo de edição ao entrar no estado PLAYING
                    playerInputEditMode = true;
                    playerInput[0] = '\0'; // Limpa o buffer
                    GuiSetState(STATE_NORMAL); // Raygui: Garante que o estado visual do controle está normal

                    // (explicacao 26) Desativado o reset da lista de palavras usadas
                    //ResetUsedWordList(); //Reseta o arquivo txt ao iniciar um novo jogo!!!!
                }

            } break;

            case PLAYING:
            {
                bombTimer -= GetFrameTime();

                // (explicacao 10) Lógica para quando o tempo da bomba acaba
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
                    // (explicacao 27) Desativado a seleção de nova sílaba
                    //currentSyllable = SelectRandomSyllable(&wordList); // Nova sílaba para o próximo jogador
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
                    // (explicacao 28) Desativado o reset da lista de palavras usadas
                    //ResetUsedWordList(); //Resetar o arquivo txt se iniciarmos um novo jogo após o jogo anteriorS
                    currentGameState = MENU;
                    // (explicacao 11) Não chama InitializePlayers() aqui, pois isso acontece na transição MENU -> PLAYING
                }

            } break;

            default: break;
        }

        BeginDrawing();

            ClearBackground(RAYWHITE);

            // (explicacao 12) Recalcula as posições dos jogadores a cada frame (útil se a janela puder ser redimensionada)
            for (int i = 0; i < NUM_PLAYERS; ++i) {
                 players[i].screenPosition = CalculatePlayerPosition(i, NUM_PLAYERS, playerPositionsCenter, playerPositionsRadius);
            }

            // (explicacao 13) Desenha as informações de cada jogador
            for (int i = 0; i < NUM_PLAYERS; ++i) {
                // Use cores diferentes para o jogador atual ou jogadores eliminados, se desejar
                Color nameColor = (i == currentPlayerIndex) ? DARKBLUE : DARKGRAY;
                Color lifeColor = (players[i].lives <= 1) ? RED : BLACK;
                DrawPlayerInfo(&players[i], textFont, nameColor, lifeColor);
            }


            // (explicacao 14) A variável 'rotacao' declarada aqui era local e não persistia o estado.
            // A rotação da seta agora será calculada dinamicamente para apontar para o jogador atual.
            // Removido: float rotacao = 90.0f;
            // Removido: arrowRotation += rotacao; // Isso adicionava 90 a cada frame de desenho no estado PLAYING!

            switch (currentGameState)
            {
                case MENU:
                {
                    const char* menuText = "Pressione 1 para Comecar (Teste)"; // (explicacao 29) Texto modificado para indicar modo de teste
                    char wordCountText[64];

                    // (explicacao 30) Desativado o texto de contagem de palavras e a verificação
                    /*
                    if (wordList.count > 0) {
                        snprintf(wordCountText, sizeof(wordCountText), " (%d palavras carregadas)", wordList.count);
                        menuText = TextFormat("Pressione 1 para Comecar%s", wordCountText);
                    } else {
                        menuText = "Erro: Lista de palavras nao carregada!";
                    }
                    */
                     // (explicacao 30) Texto alternativo para quando a lista não é carregada
                     // Como a lista não será carregada, podemos sempre mostrar este texto
                     menuText = "Pressione 1 para Comecar (Teste - Sem palavras)";


                    DrawTextEx(textFont, "Bomb Party", (Vector2){currentActualWidth/2 - MeasureTextEx(textFont, "Bomb Party", 40, 0).x/2, currentActualHeight/3}, 40, 0, GRAY);
                    DrawTextEx(textFont, menuText, (Vector2){currentActualWidth/2 - MeasureTextEx(textFont, menuText, 20, 0).x/2, currentActualHeight/2}, 20, 0, DARKGRAY);

                } break;

                case PLAYING:
                {
                    // DrawTextEx(textFont, "Estado: JOGANDO", (Vector2){10, 10}, 20, 0, BLACK); // (explicacao 15) Removido, player info já indica

                    // (explicacao 31) A sílaba é um valor fixo agora
                    if (currentSyllable != NULL) { // && wordList.count > 0) { // Removido a verificação de wordList.count
                        Vector2 syllablePos = {currentActualWidth/2 - MeasureTextEx(textFont, currentSyllable, 60, 0).x/2, currentActualHeight/2 - 80 };
                        DrawTextEx(textFont, currentSyllable, syllablePos, 60, 0, BLUE);
                    } else {
                            DrawTextEx(textFont, "Sem Silaba!", (Vector2){currentActualWidth/2 - MeasureTextEx(textFont, "Sem Silaba!", 30, 0).x/2, currentActualHeight/2 - 80}, 30, 0, RED);
                    }


                    // Raygui: Define a área do campo de input
                    Rectangle inputBounds = {currentActualWidth/2 - 150, currentActualHeight - 80, 300, 40 };
                    // Raygui: Desenha o campo de input E processa o input do teclado se playerInputEditMode for true
                    // Retorna true quando Enter é pressionado E está em modo de edição
                    // (explicacao 32) Simplificada a lógica do GuiTextBox para apenas passar o turno
                    if (GuiTextBox(inputBounds, playerInput, MAX_PLAYER_INPUT_CHARS, playerInputEditMode)) {
                        TraceLog(LOG_INFO, TextFormat("Player submitted: '%s'", playerInput));

                        // bool isValid = checkWord(playerInput, currentSyllable, &wordList); // (explicacao 32) Desativado a validação

                        // (explicacao 32) Sempre passa o turno no modo de teste
                        TraceLog(LOG_INFO, "Modo de teste: Passando turno sem validar palavra.");
                        currentPlayerIndex = (currentPlayerIndex + 1) % NUM_PLAYERS;
                        bombTimer = initialBombTime; // Reseta o timer para o próximo jogador
                        // currentSyllable = SelectRandomSyllable(&wordList); // (explicacao 32) Desativado a seleção de nova sílaba
                        TraceLog(LOG_INFO, TextFormat("Turno de %s. Silaba: %s", players[currentPlayerIndex].name, currentSyllable));

                        playerInput[0] = '\0'; // Reseta o input do usuário após a tentativa
                    }

                    // (explicacao 17) Cálculo para desenhar a seta apontando para o jogador atual
                    if (arrowTexture.id != 0 && NUM_PLAYERS > 0) { // Garante que a textura existe e há jogadores
                         Vector2 arrowPivot = playerPositionsCenter; // Seta pivoteia no centro
                         Vector2 targetPlayerPos = players[currentPlayerIndex].screenPosition;

                         Vector2 direction = {
                            targetPlayerPos.x - arrowPivot.x,
                            targetPlayerPos.y - arrowPivot.y
                         };

                         // Calcula o ângulo em radianos
                         float angle_radians = atan2f(direction.y, direction.x);
                         float angle_degrees = angle_radians * RAD2DEG; // Raylib define RAD2DEG, está correto.

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

                    // (explicacao 18) Posição da bomba e faísca ainda podem ser calculadas no centro, como antes
                    Vector2 bombPosition = {
                        currentActualWidth / 2 - (bombTexture.width * bombScale) / 2,
                        currentActualHeight / 2 - (bombTexture.height * bombScale) / 2
                    };

                    float sparkScale = 0.05f;
                    // (explicacao 19) sparkRotation é em graus para DrawTextureEx/Pro
                    float sparkRotation = -30.0f;


                    Vector2 sparkPosition = {
                        bombPosition.x + 100,
                        bombPosition.y + 5
                    };

                    // (explicacao 20) Usando DrawTextureEx ou DrawTexture na bomba e faísca (sem rotação/pivot especial)
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

    // (explicacao 33) Desativado o descarregamento da lista de palavras
    //UnloadWordList(&wordList);

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