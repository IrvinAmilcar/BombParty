#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"
#include "gemini.h"

#define API_KEY "AIzaSyDzZ5GN8H9oiNeNPPK_rD9WOaJACJ2AF2I"
#define MAX_PALAVRAS 100

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

void generate_word_list(const char *theme, const char *output_file_path) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Erro ao inicializar CURL.\n");
        return;
    }

    StringBuf response;
    sbInit(&response);

    char url[1024];
    snprintf(url, sizeof(url), "https://api.gemini.com/v1/words?theme=%s&limit=%d", theme, MAX_PALAVRAS);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Authorization: Bearer " API_KEY);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, sbWrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Erro ao fazer a requisição: %s\n", curl_easy_strerror(res));
        free(response.ptr);
        curl_easy_cleanup(curl);
        return;
    }

    // Processa a resposta
    cJSON *json = cJSON_Parse(response.ptr);
    if (!json) {
        fprintf(stderr, "Erro ao analisar a resposta da API.\n");
        free(response.ptr);
        curl_easy_cleanup(curl);
        return;
    }

    const cJSON *words = cJSON_GetObjectItem(json, "words");
    if (cJSON_IsArray(words)) {
        FILE *output_file = fopen(output_file_path, "w");
        if (!output_file) {
            fprintf(stderr, "Erro ao abrir o arquivo de saída '%s'.\n", output_file_path);
            cJSON_Delete(json);
            free(response.ptr);
            curl_easy_cleanup(curl);
            return;
        }

        cJSON *word = NULL;
        cJSON_ArrayForEach(word, words) {
            if (cJSON_IsString(word)) {
                fprintf(output_file, "%s\n", word->valuestring);
            }
        }

        fclose(output_file);
        printf("Palavras relacionadas ao tema '%s' foram salvas em '%s'.\n", theme, output_file_path);
    }

    // Limpeza
    cJSON_Delete(json);
    free(response.ptr);
    curl_easy_cleanup(curl);
}
