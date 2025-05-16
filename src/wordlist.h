#ifndef WORDLIST_H
#define WORDLIST_H

#include "raylib.h" // Para tipos como bool, TextFormat, TraceLog
#include <stdbool.h> // Para o tipo bool

// Inclua game.h se ele contiver definições como MAX_WORD_LENGTH
// Assumindo que game.h define:
// MAX_WORD_LENGTH, MAX_SYLLABLE_LENGTH, MIN_SYLLABLE_LENGTH, MAX_PLAYER_INPUT_CHARS


// Estrutura para armazenar a lista de palavras e sílabas usando buffer contínuo
typedef struct {
    char* allWordsBuffer;       // Ponteiro para o grande bloco de memória com todas as palavras
    char** wordPointers;       // Array de ponteiros para o início de cada palavra dentro de allWordsBuffer
    int count;                 // Número de palavras carregadas
} WordList;

// Função para carregar a lista de palavras de um arquivo (agora carrega para buffer contínuo)
WordList LoadWordList(const char *filePath);

// Função para descarregar a lista de palavras e liberar a memória (agora libera o buffer contínuo rapidamente)
void UnloadWordList(WordList* list);

// Função para selecionar uma sílaba aleatória de uma palavra na lista
const char* SelectRandomSyllable(const WordList* list);

// Funções para gerenciar palavras usadas (protótipos se forem usadas fora de wordlist.c)
void ResetUsedWordList(); // Assuming this is used outside

// Note: isWordInList, isWordAreadyUsed, addWordToUsedList são static em wordlist.c,
// não precisam de protótipos públicos aqui, a menos que sejam chamadas de fora.
// Mantenho ResetUsedWordList aqui pois você a chama em main.c.

// Função principal para verificar a palavra do jogador
bool checkWord(const char *playerInput, const char *currentSyllable, WordList *wordList);


#endif // WORDLIST_H