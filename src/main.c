// includes
//---------------------------------------------------------------
#include "raylib.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <time.h>   
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
#define MAX_WORD_LENGTH 64
#define MAX_SYLLABLE_LENGTH 3
#define MIN_SYLLABLE_LENGTH 2 
//---------------------------------------------------------------


// --- IMPLEMENTAÇÕES DAS SUAS FUNÇÕES ---
WordList LoadWordList(const char *filePath) {
    WordList list = { NULL, 0 };
    FILE* file = fopen(filePath, "r");

    if (file == NULL) {
        TraceLog(LOG_ERROR, TextFormat("Failed to open word list file: %s", filePath));
        return list; 
    }

    char line[MAX_WORD_LENGTH];
    int potential_count = 0; 

    while (fgets(line, sizeof(line), file) != NULL) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--; 
        }
         if (len > 0 && line[len - 1] == '\r') {
            line[len - 1] = '\0';
            len--;
        }

        if (len > 1) {
             potential_count++;
        }
    }

    fseek(file, 0, SEEK_SET);

    if (potential_count > 0) {
         list.words = (char**)malloc(potential_count * sizeof(char*));
         if (list.words == NULL) {
             TraceLog(LOG_ERROR, "Failed to allocate memory for word list pointers.");
             fclose(file);
             return list;
         }
    } else {
         TraceLog(LOG_WARNING, "Word list file is empty or contains only short words.");
         fclose(file);
         return list; 
    }


    int i = 0;

    while (fgets(line, sizeof(line), file) != NULL && i < potential_count) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }
         if (len > 0 && line[len - 1] == '\r') {
            line[len - 1] = '\0';
            len--;
        }

        if (len > 1) {
            list.words[i] = (char*)malloc((len + 1) * sizeof(char));

            if (list.words[i] == NULL) {
                TraceLog(LOG_ERROR, TextFormat("Failed to allocate memory for word '%s' (index %d). Cleaning up previously allocated memory.", line, i));

                for(int j = 0; j < i; ++j) {
                    free(list.words[j]);
                    list.words[j] = NULL; 
                }

                free(list.words);
                list.words = NULL;

                list.count = 0; 

                fclose(file);

                return list;
            }

            strcpy(list.words[i], line);
            i++; 
        }
    }

    list.count = i;

    fclose(file); 

    if (list.count > 0) {
        TraceLog(LOG_INFO, TextFormat("Successfully loaded %d valid words from %s", list.count, filePath));
    } else {
         TraceLog(LOG_WARNING, TextFormat("File %s exists but contains no valid words (length > 1).", filePath));
    }


    return list;
}

void UnloadWordList(WordList* list) {
    if (list == NULL || list->words == NULL) return;

    for (int i = 0; i < list->count; i++) {
        free(list->words[i]); 
        list->words[i] = NULL; 
    }
    free(list->words); 
    list->words = NULL;
    list->count = 0;
    TraceLog(LOG_INFO, "Word list unloaded.");
}

const char* SelectRandomSyllable(const WordList* list) {
    if (list == NULL || list->words == NULL || list->count == 0) {
        TraceLog(LOG_WARNING, "Word list is empty or not loaded.");
        return "err";
    }

    const char* selectedWord = NULL;
    int wordLength = 0;
    int tries = 0;
    const int maxTries = 100;

    while (selectedWord == NULL && tries < maxTries) {
        int wordIndex = GetRandomValue(0, list->count - 1);
        selectedWord = list->words[wordIndex];
        wordLength = (selectedWord != NULL) ? strlen(selectedWord) : 0;

        if (wordLength < MIN_SYLLABLE_LENGTH) {
             selectedWord = NULL; 
        }
        tries++;
    }

    if (selectedWord == NULL) {
         TraceLog(LOG_WARNING, TextFormat("Could not find a word long enough (min length %d) after %d tries.", MIN_SYLLABLE_LENGTH, maxTries));
         return "fail"; 
    }

    int syllableLength;
    int startIndex;

    if (wordLength < MAX_SYLLABLE_LENGTH) {
        syllableLength = MIN_SYLLABLE_LENGTH; 
    } else {
        syllableLength = GetRandomValue(MIN_SYLLABLE_LENGTH, MAX_SYLLABLE_LENGTH);
    }


    int maxStartIndex = wordLength - syllableLength;
    startIndex = GetRandomValue(0, maxStartIndex);


    static char randomSyllable[MAX_SYLLABLE_LENGTH + 1];
    strncpy(randomSyllable, selectedWord + startIndex, syllableLength);
    randomSyllable[syllableLength] = '\0';

    TraceLog(LOG_INFO, TextFormat("Selected word: '%s' (len %d), Syllable: '%s' (start: %d, len: %d)", selectedWord, wordLength, randomSyllable, startIndex, syllableLength));


    return randomSyllable;
}
//---------------------------------------------------------------


// ponto de entrada principal
//---------------------------------------------------------------
int main(void)
{
    // inicialização
    //---------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    srand(time(NULL));

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
         // TODO: Lidar com falha no carregamento de assets (talvez sair ou mostrar tela de erro)
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

    WordList wordList = LoadWordList("../../resources/data/palavras.txt");

    const char* currentSyllable = NULL;

    GameState currentGameState = MENU;

    float bombTimer = 0.0f;
    float initialBombTime = 15.0f;

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
                const char* menuText = "Pressione 1 para Comecar";
                char wordCountText[64];

                if (wordList.count > 0) {
                    snprintf(wordCountText, sizeof(wordCountText), " (%d palavras carregadas)", wordList.count);
                    menuText = TextFormat("Pressione 1 para Comecar%s", wordCountText);
                } else {
                    menuText = "Erro: Lista de palavras nao carregada!";
                    // Opcional: Desabilitar o inicio do jogo se a lista não carregar
                }


                if (IsKeyPressed(KEY_ONE))
                {
                    // --- INICIALIZAÇÃO DO JOGO (AGORA COM VERIFICAÇÃO DA LISTA) ---
                    if (wordList.count > 0) {
                        currentGameState = PLAYING;
                        bombTimer = initialBombTime;
                        currentSyllable = SelectRandomSyllable(&wordList);
                        TraceLog(LOG_INFO, TextFormat("Game started with syllable: %s", currentSyllable));
                    } else {
                        TraceLog(LOG_ERROR, "Cannot start game: Word list not loaded or empty.");
                        // Opcional: Não mudar de estado ou mostrar mensagem de erro na tela
                    }
                    // --- FIM INICIALIZAÇÃO DO JOGO ---
                }

                DrawTextEx(textFont, "Bomb Party", (Vector2){screenWidth/2 - MeasureTextEx(textFont, "Bomb Party", 40, 0).x/2, screenHeight/3}, 40, 0, GRAY);
                DrawTextEx(textFont, menuText, (Vector2){screenWidth/2 - MeasureTextEx(textFont, menuText, 20, 0).x/2, screenHeight/2}, 20, 0, DARKGRAY);


            } break;

            case PLAYING:
            {

                bombTimer -= GetFrameTime();

                if (bombTimer <= 0.0f)
                {
                    bombTimer = 0.0f;
                    currentGameState = GAME_OVER;
                    TraceLog(LOG_INFO, "Bomb exploded! Game Over.");
                     // TODO: Limpar a sílaba atual se ela fosse alocada dinamicamente (com a versão estática, não precisa liberar)
                     // currentSyllable = NULL; // Opcional, apenas para indicar que não há sílaba ativa
                }


                // - Processar input do jogador (digitacao)
                // - Checar regras do jogo (palavra valida, passar a bomba)
                // - Verificar condicoes de fim de jogo (jogador eliminado - que tambem leva a GAME_OVER)


                // Exemplo de como mudar para GAME_OVER com outra tecla para teste (pode remover depois)
                 if (IsKeyPressed(KEY_TWO))
                 {
                  currentGameState = GAME_OVER;
                  // TODO: Salvar pontuacao final ou preparar tela de Game Over
                 }

            } break;

            case GAME_OVER:
            {
                if (IsKeyPressed(KEY_THREE))
                {
                    currentGameState = MENU;
                    // TODO: Reinicializar todas as variaveis do jogo para comecar de novo
                    // (currentSyllable será selecionada novamente ao entrar em PLAYING)
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

                case PLAYING:
                {
                    DrawTextEx(textFont, "Estado: JOGANDO", (Vector2){10, 10}, 20, 0, BLACK);

                    if (currentSyllable != NULL && wordList.count > 0) {
                        Vector2 syllablePos = { screenWidth/2 - MeasureTextEx(textFont, currentSyllable, 60, 0).x/2, screenHeight/2 - 80 };
                        DrawTextEx(textFont, currentSyllable, syllablePos, 60, 0, BLUE);
                    } else if (currentSyllable == NULL && currentGameState == PLAYING) {
                        DrawTextEx(textFont, "Sem Silaba!", (Vector2){screenWidth/2 - MeasureTextEx(textFont, "Sem Silaba!", 30, 0).x/2, screenHeight/2 - 80}, 30, 0, RED);
                    }
                    // TODO: Desenhar input do jogador, pontuacoes, jogadores, etc.

                    DrawTextEx(textFont, TextFormat("Timer: %.1f", bombTimer), (Vector2){screenWidth - 150, 10}, 25, 0, (bombTimer <= 5.0f ? RED : DARKGRAY));

                    // Desenha assets visuais. Ajuste as posições ou lógica de desenho conforme necessário.
                    // Você pode querer que a bomba, faísca e flecha mudem de posição ou estado.
                    DrawTexture(arrowTexture, screenWidth/2 - arrowTexture.width/2, screenHeight/2 - arrowTexture.height/2 + 50, WHITE); 
                    DrawTexture(bombTexture, screenWidth/2 - bombTexture.width/2, screenHeight/2 - bombTexture.height/2, WHITE);
                    // DrawTexture(sparkTexture, ...); // Desenhar a faísca talvez condicionalmente ou animada


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
    UnloadWordList(&wordList);

    if (customFont.texture.id != 0 && textFont.texture.id != GetFontDefault().texture.id) 
    {
        UnloadFont(customFont);
    }

    if (bombTexture.id != 0) UnloadTexture(bombTexture);
    if (sparkTexture.id != 0) UnloadTexture(sparkTexture);
    if (arrowTexture.id != 0) UnloadTexture(arrowTexture);
    //---------------------------------------------------------------

    CloseWindow();

    return 0;
}
//---------------------------------------------------------------