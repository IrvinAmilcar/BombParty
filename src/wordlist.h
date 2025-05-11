#ifndef WORDLIST_H
#define WORDLIST_H

#include <stdlib.h>
#include <stdbool.h>

#include "game.h"

typedef struct {
    char **words;
    int count;
} WordList;

WordList LoadWordList(const char *filePath);
void UnloadWordList(WordList *list);
const char *SelectRandomSyllable(const WordList *list);

//NOVA FUNÇÃO DE VALIDAÇÃO! (aqui nós apenas declaramos ela, vá pra wordlist.c)
bool checkWord(const char *playerInput, const char *currentSyllable, WordList *wordList);

//Nova função pra resetar o arquivo txt temporario (palavras_usadas.txt)
void ResetUsedWordList();
#endif
