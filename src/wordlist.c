#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"   //Necessário para TraceLog, TextFormat, GetRandomValue, LOG_* macros

//Os includes criados por nós:
#include "wordlist.h"
#include "game.h"

//Novos includes necessários pras novas funções de validação!
#include <stdbool.h>
#include <ctype.h> //pra usar tolower

#define USED_WORDS_FILE "resources/data/palavras_usadas.txt" //Caminho pro arquivo temporário

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

//Função básica para converter toda string pra lower!
static void toLowerString(char *string){
    for (int i = 0; string[i]; i ++){
        string[i] = tolower(string[i]);
    }
}

//1.função: verificar se a palavra está no banco de dados!
static bool isWordInList(const WordList *list, const char *word){
    if (list == NULL || list->words == NULL || word == NULL) {
        return false;
    }

    char lowerWord[MAX_WORD_LENGTH];
    strncpy(lowerWord, word, sizeof(lowerWord) - 1);
    lowerWord[sizeof(lowerWord) - 1] = '\0';
    toLowerString(lowerWord);

    for (int i = 0; i < list->count; i++) {
        char lowerListWord[MAX_WORD_LENGTH];
        strncpy(lowerListWord, list->words[i], sizeof(lowerListWord) - 1);
        lowerListWord[sizeof(lowerListWord) - 1] = '\0';
        toLowerString(lowerListWord);
        if (strcmp(lowerListWord, lowerWord) == 0) {
            return true;
        }
    }
    return false;
}

//Função pra verificar se a palavra está no arquivo txt temporário! (implementar)
static bool isWordAreadyUsed(const char *word){
    FILE *file = fopen(USED_WORDS_FILE, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo de palavras usadas");
        return false; // Se não conseguir abrir, considera que a palavra não foi usada (para evitar erros)
    }

    char line[MAX_WORD_LENGTH + 2]; // +2 para o caractere de nova linha e o nulo
    char lowerWord[MAX_WORD_LENGTH];
    strncpy(lowerWord, word, sizeof(lowerWord) - 1);
    lowerWord[sizeof(lowerWord) - 1] = '\0';
    toLowerString(lowerWord);

    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove a nova linha do final da linha lida
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        char lowerLine[MAX_WORD_LENGTH];
        strncpy(lowerLine, line, sizeof(lowerLine) - 1);
        lowerLine[sizeof(lowerLine) - 1] = '\0';
        toLowerString(lowerLine);

        if (strcmp(lowerLine, lowerWord) == 0) {
            fclose(file);
            return true; // A palavra já foi usada
        }
    }

    fclose(file);
    return false; //Significa que a palavra não foi usada!!
}

//Função pra adicionar a palavra no arquivo txt temporário (Implementar!)
static void addWordToUsedList(const char *word){
    FILE *file = fopen(USED_WORDS_FILE, "a"); // Abre o arquivo em modo de anexar
    if (file == NULL) { //Se der erro pra abrir o arquivo, vai da nao!
        return;
    }

    fprintf(file, "%s\n", word);
    fclose(file);
}

//Nova função pra limpar o arquivo temporário, será chamado sempre que um novo jogo iniciar!
void ResetUsedWordList(){
    FILE *file = fopen(USED_WORDS_FILE, "w"); // Abre o arquivo em modo de escrita (sobrescreve o conteúdo)
    if (file == NULL) { //De novo se der erro só
        return;
    }
    fclose(file);
}

//2.Função principal que vai usar todas essas funções passadas!
bool checkWord(const char *playerInput, const char *currentSyllable, WordList *wordList){
    if (playerInput == NULL || currentSyllable == NULL || wordList == NULL) {
        return false; // Ou talvez um código de erro mais específico
    }

    char lowerInput[MAX_PLAYER_INPUT_CHARS + 1];
    strncpy(lowerInput, playerInput, sizeof(lowerInput) - 1);
    lowerInput[sizeof(lowerInput) - 1] = '\0';
    toLowerString(lowerInput);

    char lowerSyllable[MAX_SYLLABLE_LENGTH + 1];
    strncpy(lowerSyllable, currentSyllable, sizeof(lowerSyllable) - 1);
    lowerSyllable[sizeof(lowerSyllable) - 1] = '\0';
    toLowerString(lowerSyllable);

    //1. Verifica se a silaba está contida na palavra
    if (strstr(lowerInput, lowerSyllable) == NULL) {
        return false; // Indica que a palavra não é válida
    }

    //2. Verifica se a palavra está no db principal
    if (!isWordInList(wordList, playerInput)) {
        return false; // Indica que a palavra não é válida
    }

    //3. Verifica se a palavra já foi usada (arquivo txt temporário!)
    if (isWordAreadyUsed(playerInput)) {
        return false; // Indica que a palavra não é válida
    }

    //Se passou por todos ifs a palavra é valida, então deve ser adicionda ao arquivo txt temporário
    addWordToUsedList(playerInput); // Adiciona a palavra à lista de usadas
    return true;
}