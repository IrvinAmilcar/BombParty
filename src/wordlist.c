#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>   // Incluir para atan2f, sinf, cosf se usados aqui (não são no seu código atual)
#include <stdbool.h>
#include <ctype.h>

#include "raylib.h"   // Necessário para TraceLog, TextFormat, GetRandomValue, LOG_* macros

// Includes criados por nós:
#include "wordlist.h"
#include "game.h" // Assumindo que MAX_WORD_LENGTH, MAX_SYLLABLE_LENGTH, etc. estão aqui

#define USED_WORDS_FILE "resources/data/palavras_usadas.txt"

// --- Funções Auxiliares (mantidas static) ---
static void toLowerString(char *string);
static bool isWordInList(const WordList *list, const char *word);
static bool isWordAreadyUsed(const char *word);
static void addWordToUsedList(const char *word);

// --- Função LoadWordList (Implementação Refatorada) ---

WordList LoadWordList(const char *filePath) {
    WordList list = { NULL, NULL, 0 }; // Inicializa allWordsBuffer e wordPointers como NULL
    FILE* file = fopen(filePath, "r");

    if (file == NULL) {
        TraceLog(LOG_ERROR, TextFormat("LoadWordList: Failed to open word list file: %s", filePath));
        return list; // Retorna lista vazia
    }

    char line[MAX_WORD_LENGTH + 2]; // +2 para \n e \r se existirem + \0
    int word_count = 0;
    size_t total_buffer_size = 0; // Usar size_t para tamanho do buffer

    // --- Primeira Passagem: Contar palavras válidas e calcular tamanho total do buffer ---
    TraceLog(LOG_INFO, "LoadWordList: Primeira passagem - contando e calculando tamanho...");
    while (fgets(line, sizeof(line), file) != NULL) {
        size_t len = strlen(line);
        // Remover \n e \r
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }
        if (len > 0 && line[len - 1] == '\r') {
            line[len - 1] = '\0';
            len--;
        }

        // Usar MIN_SYLLABLE_LENGTH para filtrar palavras curtas na contagem
        if (len >= MIN_SYLLABLE_LENGTH) {
            word_count++;
            total_buffer_size += len + 1; // +1 para o terminador nulo
        }
    }

    // Voltar para o início do arquivo
    fseek(file, 0, SEEK_SET);

    if (word_count == 0) {
        TraceLog(LOG_WARNING, TextFormat("LoadWordList: File %s exists but contains no valid words (length >= %d).", filePath, MIN_SYLLABLE_LENGTH));
        fclose(file);
        return list; // Retorna lista vazia se nenhuma palavra válida for encontrada
    }

    // --- Alocação de Memória: Buffer contínuo e array de ponteiros ---
    TraceLog(LOG_INFO, TextFormat("LoadWordList: Total de %d palavras válidas encontradas. Tamanho total do buffer: %zu bytes.", word_count, total_buffer_size));
    list.allWordsBuffer = (char*)malloc(total_buffer_size);
    if (list.allWordsBuffer == NULL) {
        TraceLog(LOG_ERROR, "LoadWordList: Failed to allocate memory for allWordsBuffer.");
        fclose(file);
        return list; // Retorna lista vazia em caso de falha na alocação
    }
    TraceLog(LOG_INFO, TextFormat("LoadWordList: allWordsBuffer alocado em %p", (void*)list.allWordsBuffer));


    list.wordPointers = (char**)malloc(word_count * sizeof(char*));
    if (list.wordPointers == NULL) {
        TraceLog(LOG_ERROR, "LoadWordList: Failed to allocate memory for wordPointers.");
        free(list.allWordsBuffer); // Liberar o buffer se a alocação dos ponteiros falhar
        list.allWordsBuffer = NULL;
        fclose(file);
        return list; // Retorna lista vazia em caso de falha na alocação
    }
     TraceLog(LOG_INFO, TextFormat("LoadWordList: wordPointers alocado em %p", (void*)list.wordPointers));


    list.count = 0; // Resetar count para usar na segunda passagem
    char* current_pos_in_buffer = list.allWordsBuffer; // Ponteiro para a posição atual no buffer

    // --- Segunda Passagem: Copiar palavras para o buffer e popular o array de ponteiros ---
    TraceLog(LOG_INFO, "LoadWordList: Segunda passagem - copiando palavras...");
    while (fgets(line, sizeof(line), file) != NULL && list.count < word_count) {
         size_t len = strlen(line);
         // Remover \n e \r novamente
         if (len > 0 && line[len - 1] == '\n') {
             line[len - 1] = '\0';
             len--;
         }
         if (len > 0 && line[len - 1] == '\r') {
             line[len - 1] = '\0';
             len--;
         }


         if (len >= MIN_SYLLABLE_LENGTH) { // Processar apenas palavras válidas novamente
             list.wordPointers[list.count] = current_pos_in_buffer; // Armazenar o ponteiro para o início da palavra
             strcpy(current_pos_in_buffer, line); // Copiar a palavra para o buffer
             current_pos_in_buffer += len + 1; // Avançar o ponteiro no buffer
             list.count++; // Incrementar a contagem de palavras realmente carregadas
         }
    }

    fclose(file);

    TraceLog(LOG_INFO, TextFormat("LoadWordList: %d palavras carregadas no buffer contínuo.", list.count));

    return list;
}

// --- Função UnloadWordList (Implementação Simplificada e Rápida com TraceLogs) ---

void UnloadWordList(WordList* list) {
    TraceLog(LOG_INFO, "UnloadWordList: Iniciando descarregamento (buffer contínuo)."); // TraceLog A

    if (list == NULL) {
        TraceLog(LOG_WARNING, "UnloadWordList: Lista nula fornecida."); // TraceLog B
        return;
    }

    TraceLog(LOG_INFO, TextFormat("UnloadWordList: Endereço da lista: %p", (void*)list)); // TraceLog C
    TraceLog(LOG_INFO, TextFormat("UnloadWordList: Ponteiro allWordsBuffer: %p", (void*)list->allWordsBuffer)); // TraceLog D'
    TraceLog(LOG_INFO, TextFormat("UnloadWordList: Ponteiro wordPointers: %p", (void*)list->wordPointers)); // TraceLog E'
    TraceLog(LOG_INFO, TextFormat("UnloadWordList: Contagem de palavras: %d", list->count)); // TraceLog F'


    // Liberar o array de ponteiros
    if (list->wordPointers != NULL) {
        TraceLog(LOG_INFO, TextFormat("UnloadWordList: Liberando array de ponteiros em %p", (void*)list->wordPointers)); // TraceLog G'
        free(list->wordPointers);
        list->wordPointers = NULL;
        TraceLog(LOG_INFO, "UnloadWordList: Array de ponteiros liberado."); // TraceLog H'
    } else {
        TraceLog(LOG_INFO, "UnloadWordList: Array de ponteiros já era nulo, nada para liberar."); // TraceLog I'
    }

    // Liberar o grande bloco de memória com todas as palavras
    if (list->allWordsBuffer != NULL) {
         TraceLog(LOG_INFO, TextFormat("UnloadWordList: Liberando buffer principal em %p", (void*)list->allWordsBuffer)); // TraceLog J'
        free(list->allWordsBuffer);
        list->allWordsBuffer = NULL;
         TraceLog(LOG_INFO, "UnloadWordList: Buffer principal liberado."); // TraceLog K'
    } else {
         TraceLog(LOG_INFO, "UnloadWordList: Buffer principal já era nulo, nada para liberar."); // TraceLog L'
    }

    list->count = 0;

    TraceLog(LOG_INFO, "UnloadWordList: Descarregamento concluído com sucesso (buffer contínuo)."); // TraceLog M'
}


// --- Funções que usam a lista (Ajustadas para usar wordPointers) ---

const char* SelectRandomSyllable(const WordList* list) {
    if (list == NULL || list->wordPointers == NULL || list->count == 0) {
        TraceLog(LOG_WARNING, "SelectRandomSyllable: Word list is empty or not loaded.");
        return "err";
    }

    const char* selectedWord = NULL;
    int wordLength = 0;
    int tries = 0;
    const int maxTries = 100;
    const char* forbiddenLetters = "KHWYZ";

    while (selectedWord == NULL && tries < maxTries) {
        int wordIndex = GetRandomValue(0, list->count - 1);
        selectedWord = list->wordPointers[wordIndex];

        if (selectedWord == NULL) {
            TraceLog(LOG_WARNING, TextFormat("SelectRandomSyllable: Null pointer found at index %d in the list.", wordIndex));
            tries++;
            continue;
        }

        wordLength = strlen(selectedWord);

        if (wordLength < MIN_SYLLABLE_LENGTH) {
            selectedWord = NULL;
        } else {
            // Verificar se a palavra contém alguma das letras proibidas
            bool containsForbidden = false;
            for (int i = 0; selectedWord[i] != '\0'; i++) {
                for (int j = 0; forbiddenLetters[j] != '\0'; j++) {
                    if (toupper(selectedWord[i]) == forbiddenLetters[j]) {
                        containsForbidden = true;
                        break;
                    }
                }
                if (containsForbidden) {
                    break;
                }
            }
            if (containsForbidden) {
                selectedWord = NULL; // Palavra contém letras proibidas, tentar outra
            }
        }
        tries++;
    }

    if (selectedWord == NULL) {
        TraceLog(LOG_WARNING, TextFormat("SelectRandomSyllable: Could not find a suitable word (min length %d) after %d tries without forbidden letters.", MIN_SYLLABLE_LENGTH, maxTries));
        static char failSyllable[] = "fail";
        return failSyllable;
    }

    int syllableLength;
    int startIndex;

    if (wordLength < MAX_SYLLABLE_LENGTH) {
        syllableLength = MIN_SYLLABLE_LENGTH;
    } else {
        syllableLength = GetRandomValue(MIN_SYLLABLE_LENGTH, MAX_SYLLABLE_LENGTH);
    }

    int maxStartIndex = wordLength - syllableLength;
    startIndex = (maxStartIndex > 0) ? GetRandomValue(0, maxStartIndex) : 0;

    static char randomSyllable[MAX_SYLLABLE_LENGTH + 1];
    strncpy(randomSyllable, selectedWord + startIndex, syllableLength);
    randomSyllable[syllableLength] = '\0';

    // TraceLog(LOG_INFO, TextFormat("SelectRandomSyllable: Selected word: '%s' (len %d), Syllable: '%s' (start: %d, len: %d)", selectedWord, wordLength, randomSyllable, startIndex, syllableLength));

    return randomSyllable;
}

// Função básica para converter toda string pra lower! (Mantida)
static void toLowerString(char *string){
    for (int i = 0; string[i]; i ++){
        string[i] = tolower(string[i]);
    }
}

// 1.função: verificar se a palavra está no banco de dados! (Ajustada para usar wordPointers)
static bool isWordInList(const WordList *list, const char *word){
    if (list == NULL || list->wordPointers == NULL || list->count == 0 || word == NULL) { // Usar wordPointers
        return false;
    }

    char lowerWord[MAX_WORD_LENGTH];
    strncpy(lowerWord, word, sizeof(lowerWord) - 1);
    lowerWord[sizeof(lowerWord) - 1] = '\0';
    toLowerString(lowerWord);

    for (int i = 0; i < list->count; i++) {
        // Usar wordPointers para acessar a palavra
        // Adicionada verificação de ponteiro nulo na lista de ponteiros antes de acessar
        if (list->wordPointers[i] == NULL) {
             TraceLog(LOG_WARNING, TextFormat("isWordInList: Ponteiro nulo encontrado no índice %d da lista.", i));
            continue; // Pula esta entrada nula
        }

        // Compara lowerWord com a palavra na lista (acessada via wordPointers)
        // Converte a palavra da lista para lower case em um buffer temporário para comparação
        char lowerListWord[MAX_WORD_LENGTH];
        strncpy(lowerListWord, list->wordPointers[i], sizeof(lowerListWord) - 1);
        lowerListWord[sizeof(lowerListWord) - 1] = '\0';
        toLowerString(lowerListWord);

        if (strcmp(lowerListWord, lowerWord) == 0) {
            return true;
        }
    }
    return false;
}

// Funções de palavras usadas (Mantidas)
static bool isWordAreadyUsed(const char *word){
    FILE *file = fopen(USED_WORDS_FILE, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo de palavras usadas");
        return false;
    }

    char line[MAX_WORD_LENGTH + 2];
    char lowerWord[MAX_WORD_LENGTH];
    strncpy(lowerWord, word, sizeof(lowerWord) - 1);
    lowerWord[sizeof(lowerWord) - 1] = '\0';
    toLowerString(lowerWord);

    while (fgets(line, sizeof(line), file) != NULL) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        if (len > 0 && line[len - 1] == '\r') {
            line[len - 1] = '\0';
        }
        char lowerLine[MAX_WORD_LENGTH];
        strncpy(lowerLine, line, sizeof(lowerLine) - 1);
        lowerLine[sizeof(lowerLine) - 1] = '\0';
        toLowerString(lowerLine);

        if (strcmp(lowerLine, lowerWord) == 0) {
            fclose(file);
            return true;
        }
    }

    fclose(file);
    return false;
}

static void addWordToUsedList(const char *word){
    FILE *file = fopen(USED_WORDS_FILE, "a");
    if (file == NULL) {
        return;
    }

    fprintf(file, "%s\n", word);
    fclose(file);
}

void ResetUsedWordList(){
    FILE *file = fopen(USED_WORDS_FILE, "w");
    if (file == NULL) {
        return;
    }
    fclose(file);
}

// 2.Função principal que vai usar todas essas funções passadas! (Mantida)
bool checkWord(const char *playerInput, const char *currentSyllable, WordList *wordList){
    if (playerInput == NULL || currentSyllable == NULL || wordList == NULL) {
        return false;
    }

    char lowerInput[MAX_PLAYER_INPUT_CHARS + 1];
    strncpy(lowerInput, playerInput, sizeof(lowerInput) - 1);
    lowerInput[sizeof(lowerInput) - 1] = '\0';
    toLowerString(lowerInput);

    char lowerSyllable[MAX_SYLLABLE_LENGTH + 1];
    strncpy(lowerSyllable, currentSyllable, sizeof(lowerSyllable) - 1);
    lowerSyllable[sizeof(lowerSyllable) - 1] = '\0';
    toLowerString(lowerSyllable);

    // 1. Verifica se a silaba está contida na palavra
    if (strstr(lowerInput, lowerSyllable) == NULL) {
        return false;
    }

    // 2. Verifica se a palavra está no db principal
    if (!isWordInList(wordList, playerInput)) {
        return false;
    }

    // 3. Verifica se a palavra já foi usada
    if (isWordAreadyUsed(playerInput)) {
        return false;
    }

    // Se passou por todos ifs a palavra é valida
    addWordToUsedList(playerInput); // Adiciona a palavra à lista de usadas
    return true;
}