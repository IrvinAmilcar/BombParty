#ifndef WORDLIST_H
#define WORDLIST_H

typedef struct {
    char **words;
    int count;
} WordList;

WordList LoadWordList(const char *filePath);
void UnloadWordList(WordList *list);
const char *SelectRandomSyllable(const WordList *list);

#endif
