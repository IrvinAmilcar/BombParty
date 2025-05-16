#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"
#include "gemini.h"

#define API_KEY "AIzaSyDzZ5GN8H9oiNeNPPK_rD9WOaJACJ2AF2I"
#define MAX_RESPOSTA  1024
#define MAX_PALAVRA   100

typedef struct {
    char *ptr;
    size_t len;
} StringBuf;

static void sbInit(StringBuf *s) {
    s->len = 0;
    s->ptr = malloc(1);
    s->ptr[0] = '\0';
}

static size_t sbWrite(void *data, size_t size, size_t nmemb, void *userp) {
    size_t add = size * nmemb;
    StringBuf *s = (StringBuf *)userp;
    char *tmp = realloc(s->ptr, s->len + add + 1);
    if (!tmp) return 0;
    s->ptr = tmp;
    memcpy(s->ptr + s->len, data, add);
    s->len += add;
    s->ptr[s->len] = '\0';
    return add;
}

void generate_word_list(const char *theme, const char *db_file_path, const char *output_file_path) {
    // Abre o arquivo do banco de dados para leitura
    FILE *db_file = fopen(db_file_path, "r");
    if (!db_file) {
        fprintf(stderr, "Erro ao abrir o arquivo de banco de dados '%s'.\n", db_file_path);
        return;
    }

    // Cria o arquivo de saída para as palavras
    FILE *output_file = fopen(output_file_path, "w");
    if (!output_file) {
        fprintf(stderr, "Erro ao abrir o arquivo de saída '%s'.\n", output_file_path);
        fclose(db_file);
        return;
    }

    char line[MAX_RESPOSTA];
    int count = 0;

    // Lê cada linha do banco de dados e verifica se está relacionada ao tema
    while (fgets(line, sizeof(line), db_file)) {
        // Remove o caractere de nova linha, se presente
        line[strcspn(line, "\n")] = '\0';

        // Verifica se a palavra está relacionada ao tema (caso simples, sem análise semântica)
        if (strstr(line, theme) != NULL) {
            fprintf(output_file, "%s\n", line);
            count++;
        }
    }

    fclose(db_file);
    fclose(output_file);
    printf("%d palavras relacionadas ao tema '%s' foram salvas em '%s'.\n", count, theme, output_file_path);
}