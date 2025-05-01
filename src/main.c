#include "raylib.h"

typedef enum GameState {
    MENU = 0,
    PLAYING,
    GAME_OVER
} GameState;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "BombParty");

    Imgage bomb = LoadImage("../resources/textures/bomb.png");
    Imgage spark = LoadImage("../resources/textures/spark.png");
    Imgage arrow = LoadImage("../resources/textures/arrow.png");

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    GameState currentGameState = MENU;

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        switch (currentGameState)
        {
            case MENU:
            {
                if (IsKeyPressed(KEY_ENTER))
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

                // Exemplo simples de transicao para GAME_OVER (pressione G no modo PLAYING):
                if (IsKeyPressed(KEY_G))
                {
                    currentGameState = GAME_OVER;
                     // TODO: Salvar pontuacao final ou preparar tela de Game Over
                }

                // TODO: Implementar logica do timer e explosao da bomba, que tambem leva a GAME_OVER

            } break;

            case GAME_OVER:
            {
                // Logica de Update para o Game Over:
                // - Esperar por input para reiniciar o jogo (ex: R)
                if (IsKeyPressed(KEY_R))
                {
                    currentGameState = MENU; // Muda de volta para o estado de MENU
                    // TODO: Reinicializar todas as variaveis do jogo para comecar de novo
                }

            } break;

            default: break;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            switch (currentGameState)
            {
                case MENU:
                {
                    // Desenhar a tela de Menu: Titulo, instrucoes, etc.
                    DrawText("Bomb Party", screenWidth/2 - MeasureText("Bomb Party", 40)/2, screenHeight/3, 40, GRAY);
                    DrawText("Pressione ENTER para Comecar", screenWidth/2 - MeasureText("Pressione ENTER para Comecar", 20)/2, screenHeight/2, 20, DARKGRAY);

                } break;

                case PLAYING:
                {
                    // Desenhar a tela de Jogo: Timer, silaba, caixa de input, bomba, etc.
                    DrawText("Estado: JOGANDO", 10, 10, 20, BLACK);
                    // TODO: Desenhar elementos do jogo (timer, silaba, input, bomba, jogadores)

                } break; // Fim do case PLAYING (Draw)

                case GAME_OVER:
                {
                    // Desenhar a tela de Game Over: Mensagem, pontuacao, instrucoes para reiniciar.
                     DrawText("Fim de Jogo!", screenWidth/2 - MeasureText("Fim de Jogo!", 40)/2, screenHeight/3, 40, DARKGRAY);
                     DrawText("Pressione R para Reiniciar", screenWidth/2 - MeasureText("Pressione R para Reiniciar", 20)/2, screenHeight/2, 20, GRAY);
                    // TODO: Desenhar pontuacao final

                } break; // Fim do case GAME_OVER (Draw)

                default: break; // Estado padrao, nao deve acontecer
            }

            // Elementos que sao desenhados SEMPRE, independentemente do estado (ex: FPS)
            DrawFPS(10, screenHeight - 20);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
     // TODO: Descarregar quaisquer assets (texturas, sons, fontes) que foram carregados
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}