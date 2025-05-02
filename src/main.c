// includes
//---------------------------------------------------------------
#include "raylib.h"
#include <stddef.h>
#include <stdio.h> 
//---------------------------------------------------------------

// structs
//---------------------------------------------------------------
typedef enum GameState {
    MENU = 0,
    PLAYING,
    GAME_OVER
} GameState;

typedef struct {
    char **words;
    int count;
} WordList;
//---------------------------------------------------------------

// Prototipos das func()
//---------------------------------------------------------------
WordList LoadWordList(const char *filePath);
void UnloadWordList(WordList *list);
const char *SelectRandomSyllable(const WordList *list);
//---------------------------------------------------------------

// constantes
//---------------------------------------------------------------
#define MAX_WORD_LENGH 64
#define MAX_SYLLABLE_LENGH 3
#define MIN_SYLLABLE_LENGH 1
//---------------------------------------------------------------

// ponto de entrada principal
//---------------------------------------------------------------
int main(void)
{
    // inicialização
    //---------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "BombParty");

    Font customFont = LoadFontEx("../../resources/fonts/Montserrat-Regular.ttf", 40, NULL, 0);
    Font textFont;

    if (customFont.texture.id == 0)
    {
        TraceLog(LOG_WARNING, "Failed to load font: ../../resources/fonts/Montserrat-Regular.ttf. Using default font.");
        textFont = GetFontDefault();
    }
    else
    {
        TraceLog(LOG_INFO, "Successfully loaded font: ../../resources/fonts/Montserrat-Regular.ttf");
        textFont = customFont;
    }

    Texture2D bombTexture = { 0 }; 
    Texture2D sparkTexture = { 0 }; 
    Texture2D arrowTexture = { 0 }; 

    Image bombImage = LoadImage("../../resources/textures/bomb.png");
    Image sparkImage = LoadImage("../../resources/textures/spark.png");
    Image arrowImage = LoadImage("../../resources/textures/arrow.png");

    if (bombImage.data == NULL || sparkImage.data == NULL || arrowImage.data == NULL)
    {
          TraceLog(LOG_ERROR, "Failed to load one or more images!");
    }
    else
    {
        bombTexture = LoadTextureFromImage(bombImage);
        sparkTexture = LoadTextureFromImage(sparkImage);
        arrowTexture = LoadTextureFromImage(arrowImage);

        UnloadImage(bombImage);
        UnloadImage(sparkImage);
        UnloadImage(arrowImage);
    }

    GameState currentGameState = MENU;

    float bombTimer = 0.0f;
    float initialBombTime = 30.0f;

    SetTargetFPS(60);
    //---------------------------------------------------------------

    // loop principal do game
    //---------------------------------------------------------------
    while (!WindowShouldClose())
    {
        switch (currentGameState)
        {
            case MENU:
            {
                if (IsKeyPressed(KEY_ONE))
                {
                    currentGameState = PLAYING;
                    // TODO: Inicializar outras variaveis do jogo (jogadores, silaba, etc.)
                    bombTimer = initialBombTime; 
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
                }


                // - Processar input do jogador (digitacao)
                // - Checar regras do jogo (palavra valida, passar a bomba)
                // - Verificar condicoes de fim de jogo (jogador eliminado - que tambem leva a GAME_OVER)

            } break;

            case GAME_OVER:
            {
                if (IsKeyPressed(KEY_THREE))
                {
                    currentGameState = MENU;
                    // TODO: Reinicializar todas as variaveis do jogo para comecar de novo (incluindo bombTimer se não inicializar no MENU->PLAYING)
                }

            } break;

            default: break;
        }

        // desenho
        //---------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            switch (currentGameState)
            {
                case MENU:
                {
                    DrawTextEx(textFont, "Bomb Party", (Vector2){screenWidth/2 - MeasureTextEx(textFont, "Bomb Party", 40, 0).x/2, screenHeight/3}, 40, 0, GRAY);
                    DrawTextEx(textFont, "Pressione 1 para Comecar", (Vector2){screenWidth/2 - MeasureTextEx(textFont, "Pressione 1 para Comecar", 20, 0).x/2, screenHeight/2}, 20, 0, DARKGRAY);
                } break;

                case PLAYING:
                {
                    DrawTextEx(textFont, "Estado: JOGANDO", (Vector2){10, 10}, 20, 0, BLACK);
                    // TODO: Desenhar outros elementos do jogo (silaba, input, jogadores)

                    DrawTextEx(textFont, TextFormat("Timer: %.1f", bombTimer), (Vector2){screenWidth - 150, 10}, 25, 0, (bombTimer <= 10.0f ? RED : DARKGRAY)); // Exemplo: fica vermelho nos ultimos 10s

                    if (bombTexture.id != 0)
                    {
                         DrawTexture(bombTexture, screenWidth/2 - bombTexture.width/2, screenHeight/2 - bombTexture.height/2, WHITE);
                    }


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
        //---------------------------------------------------------------
    }

    // --- Unload Resources ---
    if (customFont.texture.id != 0 && textFont.texture.id != GetFontDefault().texture.id) // Unload only if custom font was successfully loaded and is not the default
    {
        UnloadFont(customFont);
    }

    // Unload Textures from GPU memory (only if they were successfully loaded)
    if (bombTexture.id != 0) UnloadTexture(bombTexture);
    if (sparkTexture.id != 0) UnloadTexture(sparkTexture);
    if (arrowTexture.id != 0) UnloadTexture(arrowTexture);

    CloseWindow();

    return 0;
}
//---------------------------------------------------------------