#include <stdio.h>
#include <stdlib.h> // Para free
#include <string.h> // Para funções de string, como strstr (se necessário para parsing mais complexo)
#include <string.h>
#include <curl/include/curl/curl.h>
#include "cJSON.h"
#include "gemini.h"

// --- Placeholder para sua chave API ---
// ATENÇÃO: Substitua isso pela sua chave API real.
// Em um aplicativo real, evite hardcodar a chave.
#define MAX_PALAVRAS 100
#define API_KEY "AIzaSyDzZ5GN8H9oiNeNPPK_rD9WOaJACJ2AF2I"

// --- Endpoint da API Google Generative Language (Gemini) ---
#define GEMINI_API_BASE_URL "https://generativelanguage.googleapis.com"
#define GEMINI_API_ENDPOINT "/v1/models/gemini-pro:generateContent" // Ou outro modelo, se preferir

#define GEMINI_API_BASE_URL "https://generativelanguage.googleapis.com"
#define GEMINI_API_ENDPOINT "/v1/models/gemini-pro:generateContent" // Ou outro modelo, se preferir

// --- Definições assumidas (ajuste conforme seu projeto) ---
// struct StringBuf e funções sbInit, sbWrite
// sbWrite deve ter a assinatura: size_t sbWrite(char *ptr, size_t size, size_t nmemb, void *userdata);
// sbInit deve inicializar StringBuf, possivelmente com ptr=NULL, len=0, cap=0.
// Assume-se que sbWrite realoca ptr.
typedef struct {
    char *ptr;
    size_t len; // Current length
    size_t cap; // Current capacity
} StringBuf;

// Exemplo BÁSICO (substitua pela sua implementação real)
void sbInit(StringBuf *sb) {
    sb->ptr = NULL;
    sb->len = 0;
    sb->cap = 0;
}

size_t sbWrite(char *ptr, size_t size, size_t nmemb, void *userdata) {
    StringBuf *sb = (StringBuf *)userdata;
    size_t num_bytes = size * nmemb;
    size_t new_len = sb->len + num_bytes;

    // Aumenta a capacidade se necessário. Exemplo simples de dobrar a capacidade.
    if (new_len >= sb->cap) {
        size_t new_cap = sb->cap == 0 ? 1024 : sb->cap * 2; // Começa com 1KB ou dobra
        // Garante que a nova capacidade seja suficiente para new_len + 1 (null terminator)
        while (new_cap <= new_len) {
             new_cap *= 2;
        }
        char *new_ptr = realloc(sb->ptr, new_cap + 1); // +1 para o terminador null

        if (new_ptr == NULL) {
            // Falha na realocação
            return 0; // Indica para libcurl que houve um erro
        }
        sb->ptr = new_ptr;
        sb->cap = new_cap;
    }

    memcpy(sb->ptr + sb->len, ptr, num_bytes);
    sb->len = new_len;
    sb->ptr[sb->len] = '\0'; // Garante que seja null-terminated

    return num_bytes; // Retorna o número de bytes processados
}
// --- Fim das definições assumidas ---


void generate_word_list(const char *theme, const char *output_file_path) {
    fprintf(stderr, "DEBUG: generate_word_list iniciado para tema '%s' e arquivo '%s'.\n", theme, output_file_path);

    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "DEBUG: Erro fatal ao inicializar CURL.\n");
        return;
    }
    fprintf(stderr, "DEBUG: CURL inicializado com sucesso.\n");

    StringBuf response;
    sbInit(&response); // Inicializa o buffer para a resposta
    fprintf(stderr, "DEBUG: StringBuf inicializado.\n");


    // --- 1. Construir a URL com a chave API ---
    char url[2048]; // Aumenta o buffer para a URL, se necessário
    snprintf(url, sizeof(url), "%s%s?key=%s", GEMINI_API_BASE_URL, GEMINI_API_ENDPOINT, API_KEY);
    fprintf(stderr, "DEBUG: URL construida: %s\n", url);


    // --- 2. Preparar o corpo da requisição (JSON com o prompt e maxOutputTokens) ---
    char prompt[512]; // Buffer para o prompt
    snprintf(prompt, sizeof(prompt), "Liste até %d palavras relacionadas a \"%s\". Separe as palavras por vírgulas. Retorne apenas as palavras em Português. Não inclua frases de introdução ou conclusão.", MAX_PALAVRAS, theme); //Prompt pra IA!
    fprintf(stderr, "DEBUG: Prompt construído: %s\n", prompt);

    // Incluindo maxOutputTokens no corpo da requisição para garantir que a resposta seja longa o suficiente
    // 100 palavras podem precisar de ~400-600 tokens. Usar 800-1000 para ter margem.
    char request_body[2048]; // Aumenta o buffer para o corpo JSON
    snprintf(request_body, sizeof(request_body),
             "{\"contents\": [{\"parts\": [{\"text\": \"%s\"}]}], \"maxOutputTokens\": 800}", prompt);
    fprintf(stderr, "DEBUG: Corpo da requisicao construido: %s\n", request_body);


    // --- 3. Configurar os cabeçalhos ---
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    fprintf(stderr, "DEBUG: Cabecalho Content-Type adicionado.\n");


    // --- 4. Configurar CURL para POST ---
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(request_body));

    // Configurar a função de escrita para receber a resposta
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, sbWrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    fprintf(stderr, "DEBUG: Opcoes CURL configuradas.\n");


    // --- 5. Executar a requisição ---
    fprintf(stderr, "DEBUG: Executando curl_easy_perform...\n");
    CURLcode res = curl_easy_perform(curl);
    fprintf(stderr, "DEBUG: curl_easy_perform finalizado.\n");


    // --- 6. Tratar erros da requisição ---
    if (res != CURLE_OK) {
        fprintf(stderr, "DEBUG: Erro CURL (%d) ao fazer a requisicao: %s\n", res, curl_easy_strerror(res));
        // LIMPEZA em caso de erro CURL
        free(response.ptr);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        fprintf(stderr, "DEBUG: Limpeza apos erro CURL.\n");
        return;
    }
    fprintf(stderr, "DEBUG: Requisicao CURL completa sem erro CURLE_OK.\n");


    // --- Adicione verificação de status HTTP ---
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &http_code);
    fprintf(stderr, "DEBUG: Status HTTP da resposta: %ld\n", http_code);

    // --- Adicione impressão da resposta bruta ---
    fprintf(stderr, "DEBUG: Resposta bruta da API (tamanho %zu):\n%s\n", response.len, response.ptr ? response.ptr : "NULL ou vazia");

     // Opcional: Verificar se o status HTTP indica um erro (4xx ou 5xx) antes de tentar parsear JSON
     if (http_code >= 400) {
         fprintf(stderr, "DEBUG: Status HTTP indica erro (%ld). Tentando parsear para detalhes do erro.\n", http_code);
         // Pode tentar parsear a resposta como JSON aqui para ver se há um objeto de erro detalhado
         // ... (codigo para parsear e imprimir {"error": {...}}) ...
         // Por enquanto, vamos deixar cair no erro de parsing JSON ou navegacao se nao for o formato esperado.
     }


    // Verifica se a resposta foi recebida no buffer
    if (response.ptr == NULL || response.len == 0) {
         fprintf(stderr, "DEBUG: Resposta da API vazia ou nula apos perform.\n");
         // LIMPEZA
         free(response.ptr);
         curl_slist_free_all(headers);
         curl_easy_cleanup(curl);
         fprintf(stderr, "DEBUG: Limpeza apos resposta vazia.\n");
         return;
    }
    fprintf(stderr, "DEBUG: Resposta recebida no buffer (tamanho %zu).\n", response.len);


    // --- 7. Processar a resposta (Parsing JSON) ---
    cJSON *json = cJSON_Parse(response.ptr);
    if (!json) {
        const char *parse_error = cJSON_GetErrorPtr();
        if (parse_error) {
            fprintf(stderr, "DEBUG: Erro ao analisar a resposta JSON da API (antes: %s).\n", parse_error);
        } else {
             fprintf(stderr, "DEBUG: Erro desconhecido ao analisar a resposta JSON da API.\n");
        }
        // LIMPEZA em caso de erro JSON parse
        free(response.ptr);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        fprintf(stderr, "DEBUG: Limpeza apos erro JSON parse.\n");
        return;
    }
    fprintf(stderr, "DEBUG: Resposta JSON parseada com sucesso.\n");


    // --- 8. Navegar na estrutura JSON da resposta da API Gemini ---
    const cJSON *candidates = cJSON_GetObjectItemCaseSensitive(json, "candidates");
    const char *generated_text = NULL;

    if (!cJSON_IsArray(candidates) || cJSON_GetArraySize(candidates) == 0) {
         fprintf(stderr, "DEBUG: JSON nao contem 'candidates' array valido ou vazio.\n");
          // Pode inspecionar o JSON completo aqui para debug:
         char *json_string_dbg = cJSON_Print(json);
         if(json_string_dbg) {
            fprintf(stderr, "DEBUG: Conteudo do JSON parseado:\n%s\n", json_string_dbg);
            cJSON_free(json_string_dbg); // Use cJSON_Free para strings de cJSON_Print
         }

         // LIMPEZA
         cJSON_Delete(json);
         free(response.ptr);
         curl_slist_free_all(headers);
         curl_easy_cleanup(curl);
         fprintf(stderr, "DEBUG: Limpeza apos navegacao JSON falhar (candidates ausente/invalido).\n");
         return;
    }
    fprintf(stderr, "DEBUG: Encontrado 'candidates' array valido.\n");

    // Tentativa de navegar para o texto gerado
    const cJSON *first_candidate = cJSON_GetArrayItem(candidates, 0);
    if (cJSON_IsObject(first_candidate)) {
        const cJSON *content = cJSON_GetObjectItemCaseSensitive(first_candidate, "content");
        if (cJSON_IsObject(content)) {
            const cJSON *parts = cJSON_GetObjectItemCaseSensitive(content, "parts");
             if (cJSON_IsArray(parts) && cJSON_GetArraySize(parts) > 0) {
                 const cJSON *text_part = cJSON_GetArrayItem(parts, 0);
                 if (cJSON_IsObject(text_part)) {
                     const cJSON *text = cJSON_GetObjectItemCaseSensitive(text_part, "text");
                     if (cJSON_IsString(text) && text->valuestring != NULL) {
                         generated_text = text->valuestring; // Encontrou o texto gerado!
                     } else {
                         fprintf(stderr, "DEBUG: Nao foi possivel encontrar/extrair campo 'text' ou nao eh string valida.\n");
                     }
                 } else {
                      fprintf(stderr, "DEBUG: Nao foi possivel encontrar/extrair primeiro item de 'parts' como objeto.\n");
                 }
             } else {
                  fprintf(stderr, "DEBUG: Nao foi possivel encontrar/extrair 'parts' array valido ou vazio.\n");
             }
        } else {
             fprintf(stderr, "DEBUG: Nao foi possivel encontrar/extrair 'content' object valido.\n");
        }
    } else {
         fprintf(stderr, "DEBUG: Nao foi possivel encontrar/extrair primeiro item de 'candidates' como objeto.\n");
    }


    // Verificar se conseguimos extrair o texto gerado
    if (generated_text == NULL) {
        fprintf(stderr, "DEBUG: generated_text final eh NULL. Nao foi possivel extrair o texto gerado da resposta JSON.\n");
        // Pode inspecionar o JSON completo aqui para debug:
        char *json_string_dbg = cJSON_Print(json);
        if(json_string_dbg) {
           fprintf(stderr, "DEBUG: Conteudo do JSON parseado:\n%s\n", json_string_dbg);
           cJSON_free(json_string_dbg); // Use cJSON_Free para strings de cJSON_Print
        }


        // LIMPEZA
        cJSON_Delete(json); // Libera a estrutura JSON
        free(response.ptr);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        fprintf(stderr, "DEBUG: Limpeza apos generated_text ser NULL.\n");
        return;
    }
    fprintf(stderr, "DEBUG: Texto gerado extraido com sucesso (primeiras 100 chars): '%.100s%s'\n", generated_text, strlen(generated_text) > 100 ? "..." : "");


    // --- 9. Processar o texto gerado e escrever no arquivo ---
    fprintf(stderr, "DEBUG: Tentando abrir o arquivo de saida '%s' no modo 'w'.\n", output_file_path);
    FILE *output_file = fopen(output_file_path, "w"); // Abre o arquivo no modo escrita (cria ou sobrescreve)
    if (!output_file) {
        fprintf(stderr, "DEBUG: Erro fatal ao abrir o arquivo de saida '%s'. Permissao? Caminho?\n", output_file_path);
        // LIMPEZA em caso de erro de arquivo
        cJSON_Delete(json);
        free(response.ptr);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        fprintf(stderr, "DEBUG: Limpeza apos erro ao abrir arquivo.\n");
        return;
    }
    fprintf(stderr, "DEBUG: Arquivo '%s' aberto com sucesso. Escrevendo texto gerado...\n", output_file_path);

    // Escreve o texto gerado como está no arquivo
    fprintf(output_file, "%s\n", generated_text);
    fprintf(stderr, "DEBUG: Texto escrito no arquivo.\n");


    fclose(output_file); // Fecha o arquivo
    fprintf(stderr, "DEBUG: Arquivo fechado.\n");


    printf("Texto gerado pela IA relacionado ao tema '%s' foi salvo em '%s'.\n", theme, output_file_path);
    fprintf(stderr, "DEBUG: Mensagem de sucesso impressa.\n");


    // --- 10. Limpeza Final (Sucesso) ---
    cJSON_Delete(json); // Libera a estrutura JSON
    free(response.ptr); // Libera o buffer da resposta
    curl_slist_free_all(headers); // Libera os cabeçalhos
    curl_easy_cleanup(curl); // Limpa o manipulador CURL
    fprintf(stderr, "DEBUG: Limpeza final concluida.\n");
}