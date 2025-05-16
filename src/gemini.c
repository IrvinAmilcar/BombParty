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

void generate_word_list(const char *theme, const char *db_file, const char *output_file) {
    char prompt[MAX_RESPOSTA];
    snprintf(prompt, sizeof(prompt), "Liste palavras relacionadas ao tema '%s'.", theme);
    
    char response[MAX_RESPOSTA] = {0};
    respt(prompt, response);
    
    FILE *out = fopen(output_file, "w");
    if (!out) {
        fprintf(stderr, "Erro ao abrir o arquivo de saída.\n");
        return;
    }
    
    // Divide a resposta em palavras e salva no arquivo
    char *token = strtok(response, ", ");
    int count = 0;
    while (token != NULL) {
        fprintf(out, "%s\n", token);
        count++;
        token = strtok(NULL, ", ");
    }
    fclose(out);
    printf("%d palavras relacionadas ao tema '%s' foram salvas em '%s'.\n", count, theme, output_file);
}
