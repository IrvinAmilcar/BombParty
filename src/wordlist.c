#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"   //Necessário para TraceLog, TextFormat, GetRandomValue, LOG_* macros

//Os includes criados por nós:
#include "wordlist.h"
#include "game.h"



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