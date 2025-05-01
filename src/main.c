#include "raylib.h"

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

    Font customFont = LoadFontEx("resources/fonts/Montserrat-Regular.ttf", 40, NULL, 0);
    Font textFont;

    if (customFont.texture.id == 0)
    {
        TraceLog(LOG_ERROR, "Failed to load font: resources/fonts/Montserrat-Regular.ttf");
        textFont = GetFontDefault();
    }
    else
    {
        TraceLog(LOG_INFO, "Successfully loaded font: resources/fonts/Montserrat-Regular.ttf");
        textFont = customFont;
    }

    Imgage bomb = LoadImage("../resources/textures/bomb.png");
    Imgage spark = LoadImage("../resources/textures/spark.png");
    Imgage arrow = LoadImage("../resources/textures/arrow.png");

    GameState currentGameState = MENU;
    
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) 
    {
        switch (currentGameState)
        {
            case MENU:
            {
                if (IsKeyPressed(KEY_1))
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

                // Exemplo simples de transicao para GAME_OVER (pressione 2 no modo PLAYING):
                if (IsKeyPressed(KEY_2))
                {
                    currentGameState = GAME_OVER;
                     // TODO: Salvar pontuacao final ou preparar tela de Game Over
                }

                // TODO: Implementar logica do timer e explosao da bomba, que tambem leva a GAME_OVER

            } break;

            case GAME_OVER:
            {
                // Logica de Update para o Game Over:
                // - Esperar por input para reiniciar o jogo (ex: 3)
                if (IsKeyPressed(KEY_3))
                {
                    currentGameState = MENU; // Muda de volta para o estado de MENU
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
                    DrawText("Bomb Party", screenWidth/2 - MeasureText("Bomb Party", 40)/2, screenHeight/3, 40, GRAY);
                    DrawText("Pressione 1 para Comecar", screenWidth/2 - MeasureText("Pressione 1 para Comecar", 20)/2, screenHeight/2, 20, DARKGRAY);

                } break;

                case PLAYING:
                {
                    DrawText("Estado: JOGANDO", 10, 10, 20, BLACK);
                    // TODO: Desenhar elementos do jogo (timer, silaba, input, bomba, jogadores)

                } break;

                case GAME_OVER:
                {
                     DrawText("Fim de Jogo!", screenWidth/2 - MeasureText("Fim de Jogo!", 40)/2, screenHeight/3, 40, DARKGRAY);
                     DrawText("Pressione 3 para Reiniciar", screenWidth/2 - MeasureText("Pressione 3 para Reiniciar", 20)/2, screenHeight/2, 20, GRAY);
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
    // TODO: Descarregar quaisquer assets (texturas, sons, fontes) que foram carregados
    if (customFont.texture.id != 0)
    {
        UnloadFont(customFont); 
    }
    
    // Note: bombImage was already unloaded after creating the texture.
    UnloadTexture(bombTexture);
    UnloadTexture()


    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}