#include "raylib.h"
#include <stddef.h>

typedef enum GameState {
    MENU = 0,
    PLAYING,
    GAME_OVER
} GameState;

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "BombParty");

    // --- Font Loading with Fallback ---
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
    // ---------------------------------

    // --- Texture Loading ---
    Texture2D bombTexture;
    Texture2D sparkTexture;
    Texture2D arrowTexture;

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
    // -----------------------

    GameState currentGameState = MENU;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        switch (currentGameState)
        {
            case MENU:
            {
                if (IsKeyPressed(KEY_ONE))
                {
                    currentGameState = PLAYING;
                    // TODO: Inicializar variaveis do jogo (timer, jogadores, etc.)
                }

            } break;

            case PLAYING:
            {
                // Logica de Update para o Jogo:
                // - Atualizar timer da bomba
                // - Processar input do jogador (digitacao)
                // - Checar regras do jogo (palavra valida, passar a bomba)
                // - Verificar condicoes de fim de jogo (timer esgotou, jogador eliminado)

                if (IsKeyPressed(KEY_TWO))
                {
                    currentGameState = GAME_OVER;
                     // TODO: Salvar pontuacao final ou preparar tela de Game Over
                }

                // TODO: Implementar logica do timer e explosao da bomba, que tambem leva a GAME_OVER

            } break;

            case GAME_OVER:
            {
                if (IsKeyPressed(KEY_THREE))
                {
                    currentGameState = MENU; 
                    // TODO: Reinicializar todas as variaveis do jogo para comecar de novo
                }

            } break;

            default: break;
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            switch (currentGameState)
            {
                case MENU:
                {
                    DrawTextEx(textFont, "Bomb Party", (Vector2){screenWidth/2 - MeasureTextEx(textFont, "Bomb Party", 40, 0).x/2, screenHeight/3}, 40, 0, GRAY);
                    DrawTextEx(textFont, "Pressione 1 para Comecar", (Vector2){screenWidth/2 - MeasureTextEx(textFont, "Pressione 1 para Comecar", 20, 0).x/2, screenHeight/2}, 20, 0, DARKGRAY);
                     // You might also draw images/textures here if part of the menu background etc.

                } break;

                case PLAYING:
                {
                    DrawTextEx(textFont, "Estado: JOGANDO", (Vector2){10, 10}, 20, 0, BLACK);
                    // TODO: Desenhar elementos do jogo (timer, silaba, input, bomba(bombTexture), jogadores)
                     if (bombTexture.id != 0)
                     {
                         DrawTexture(bombTexture, 100, 100, WHITE);
                     }


                } break;

                case GAME_OVER:
                {
                    DrawTextEx(textFont, "Fim de Jogo!", (Vector2){screenWidth/2 - MeasureTextEx(textFont, "Fim de Jogo!", 40, 0).x/2, screenHeight/3}, 40, 0, DARKGRAY);
                    DrawTextEx(textFont, "Pressione 3 para Reiniciar", (Vector2){screenWidth/2 - MeasureTextEx(textFont, "Pressione 3 para Reiniciar", 20, 0).x/2, screenHeight/2}, 20, 0, GRAY);
                    // TODO: Desenhar pontuacao final

                } break;

                default: break;
            }

            DrawFPS(10, screenHeight - 20);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    // --- Unload Resources ---
    if (customFont.texture.id != 0)
    {
        UnloadFont(customFont); 
    }

    // Unload Textures from GPU memory (only if they were successfully loaded)
    if (bombTexture.id != 0) UnloadTexture(bombTexture);
    if (sparkTexture.id != 0) UnloadTexture(sparkTexture);
    if (arrowTexture.id != 0) UnloadTexture(arrowTexture);

    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}