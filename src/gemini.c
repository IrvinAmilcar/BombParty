#include <stdio.h>
#include <stdlib.h> // Para free
#include <string.h> // Para funções de string, como strstr (se necessário para parsing mais complexo)

#include <curl/curl.h>
#include <cJSON.h>

// --- Placeholder para sua chave API ---
// ATENÇÃO: Substitua isso pela sua chave API real.
// Em um aplicativo real, evite hardcodar a chave.
#define MAX_PALAVRAS 100
#define API_KEY "AIzaSyDzZ5GN8H9oiNeNPPK_rD9WOaJACJ2AF2I"

// --- Endpoint da API Google Generative Language (Gemini) ---
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
    size_t new_len = sb->len + size * nmemb;

    // Aumenta a capacidade se necessário. Exemplo simples de dobrar a capacidade.
    if (new_len >= sb->cap) {
        size_t new_cap = sb->cap == 0 ? 1024 : sb->cap * 2; // Começa com 1KB ou dobra
        char *new_ptr = realloc(sb->ptr, new_cap + 1); // +1 para o terminador null

        if (new_ptr == NULL) {
            // Falha na realocação
            return 0; // Indica para libcurl que houve um erro
        }
        sb->ptr = new_ptr;
        sb->cap = new_cap;
    }

    memcpy(sb->ptr + sb->len, ptr, size * nmemb);
    sb->len = new_len;
    sb->ptr[sb->len] = '\0'; // Garante que seja null-terminated

    return size * nmemb; // Retorna o número de bytes processados
}
// --- Fim das definições assumidas ---


void generate_word_list(const char *theme, const char *output_file_path) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Erro ao inicializar CURL.\n");
        return;
    }

    StringBuf response;
    sbInit(&response); // Inicializa o buffer para a resposta

    // --- 1. Construir a URL com a chave API ---
    // O endpoint da API Gemini real é diferente
    char url[2048]; // Aumenta o buffer para a URL, se necessário
    snprintf(url, sizeof(url), "%s%s?key=%s", GEMINI_API_BASE_URL, GEMINI_API_ENDPOINT, API_KEY);

    // --- 2. Preparar o corpo da requisição (JSON com o prompt) ---
    // O prompt instrui a IA a gerar palavras relacionadas ao tema.
    // Ajuste o prompt para obter o formato de saída desejado (ex: lista separada por vírgulas).
    char prompt[512]; // Buffer para o prompt
    // Exemplo de prompt: Pedir 10 palavras separadas por vírgulas
    snprintf(prompt, sizeof(prompt), "Liste até %d palavras relacionadas a \"%s\". Separe as palavras por vírgulas. **Retorne apenas as palavras em Português.** Não inclua frases de introdução ou conclusão.", MAX_PALAVRAS, theme); //Prompt pra IA!
    char request_body[1024]; // Buffer para o corpo JSON
    // Estrutura JSON mínima para a API Gemini Generate Content
    snprintf(request_body, sizeof(request_body), "{\"contents\": [{\"parts\": [{\"text\": \"%s\"}]}]}", prompt); // Nota: O tema/prompt aqui não precisa de URL-escape, mas caracteres como " e \ precisam ser escapados em JSON. A função snprintf não faz isso automaticamente para o prompt. Se o tema puder conter ", \ ou quebras de linha, você precisará escapar essas caracteres no prompt antes de colocá-lo no JSON. Uma forma mais robusta seria construir o JSON com cJSON.

    // --- 3. Configurar os cabeçalhos ---
    struct curl_slist *headers = NULL;
    // A API Gemini geralmente não usa 'Authorization: Bearer API_KEY' para chaves simples.
    // A chave já está na URL. Precisamos do Content-Type.
    headers = curl_slist_append(headers, "Content-Type: application/json");
    // Não adicionamos Authorization: Bearer API_KEY aqui para a chave API simples.

    // --- 4. Configurar CURL para POST ---
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body); // Define o corpo da requisição POST
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(request_body)); // Define o tamanho do corpo

    // Configurar a função de escrita para receber a resposta
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, sbWrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // --- 5. Executar a requisição ---
    CURLcode res = curl_easy_perform(curl);

    // --- 6. Tratar erros da requisição ---
    if (res != CURLE_OK) {
        fprintf(stderr, "Erro ao fazer a requisição CURL: %s\n", curl_easy_strerror(res));
        // LIMPEZA em caso de erro CURL
        free(response.ptr); // Libera o buffer da resposta
        curl_slist_free_all(headers); // Libera os cabeçalhos alocados
        curl_easy_cleanup(curl); // Limpa o manipulador CURL
        return;
    }

    // Verifica se a resposta foi recebida no buffer
    if (response.ptr == NULL || response.len == 0) {
        fprintf(stderr, "Resposta da API vazia ou nula.\n");
        // LIMPEZA
        free(response.ptr);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return;
    }


    // --- 7. Processar a resposta (Parsing JSON) ---
    cJSON *json = cJSON_Parse(response.ptr); // response.ptr agora contém o JSON da resposta
    if (!json) {
        // Erro ao analisar o JSON
        const char *parse_error = cJSON_GetErrorPtr();
        if (parse_error) {
            fprintf(stderr, "Erro ao analisar a resposta JSON da API (antes: %s).\n", parse_error);
        } else {
             fprintf(stderr, "Erro desconhecido ao analisar a resposta JSON da API.\n");
        }
        // LIMPEZA em caso de erro JSON parse
        free(response.ptr);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return;
    }

    // --- 8. Navegar na estrutura JSON da resposta da API Gemini ---
    // A resposta típica está em: root -> "candidates" [0] -> "content" -> "parts" [0] -> "text"
    const cJSON *candidates = cJSON_GetObjectItemCaseSensitive(json, "candidates");
    const cJSON *text_part = NULL;
    const char *generated_text = NULL;

    if (cJSON_IsArray(candidates) && cJSON_GetArraySize(candidates) > 0) {
        const cJSON *first_candidate = cJSON_GetArrayItem(candidates, 0);
        if (cJSON_IsObject(first_candidate)) {
            const cJSON *content = cJSON_GetObjectItemCaseSensitive(first_candidate, "content");
            if (cJSON_IsObject(content)) {
                const cJSON *parts = cJSON_GetObjectItemCaseSensitive(content, "parts");
                if (cJSON_IsArray(parts) && cJSON_GetArraySize(parts) > 0) {
                    text_part = cJSON_GetArrayItem(parts, 0);
                    if (cJSON_IsObject(text_part)) {
                        const cJSON *text = cJSON_GetObjectItemCaseSensitive(text_part, "text");
                        if (cJSON_IsString(text) && text->valuestring != NULL) {
                            generated_text = text->valuestring; // Encontrou o texto gerado!
                        }
                    }
                }
            }
        }
    }

    // Verificar se conseguimos extrair o texto gerado
    if (generated_text == NULL) {
        fprintf(stderr, "Não foi possível extrair o texto gerado da resposta JSON.\n");
         // Pode inspecionar o JSON completo aqui para debug:
         // char *json_string = cJSON_Print(json);
         // fprintf(stderr, "Resposta JSON recebida:\n%s\n", json_string);
         // cJSON_Free(json_string); // Use cJSON_Free para strings de cJSON_Print

        // LIMPEZA
        cJSON_Delete(json); // Libera a estrutura JSON
        free(response.ptr);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return;
    }

    // --- 9. Processar o texto gerado e escrever no arquivo ---
    FILE *output_file = fopen(output_file_path, "w"); // Abre o arquivo no modo escrita (cria ou sobrescreve)
    if (!output_file) {
        fprintf(stderr, "Erro ao abrir o arquivo de saída '%s'.\n", output_file_path);
        // LIMPEZA em caso de erro de arquivo
        cJSON_Delete(json);
        free(response.ptr);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return;
    }

    // Aqui, você precisa adaptar como escrever o 'generated_text'.
    // Se o prompt pediu uma lista separada por vírgulas, você pode querer
    // separar a string por vírgulas e escrever cada palavra em uma nova linha.
    // Se o prompt pediu um parágrafo, talvez você queira escrever o parágrafo inteiro.
    // Este é um exemplo SIMPLES que apenas escreve o texto inteiro no arquivo.
    // Implemente a lógica de split/processamento conforme o formato que você pediu no prompt.

    // Exemplo simples: escreve o texto gerado como está
    fprintf(output_file, "%s\n", generated_text);

    // Exemplo (conceitual) de como splitar por vírgulas e escrever linha a linha:
    /*
    char *mutable_text = strdup(generated_text); // Faz uma cópia mutável se precisar modificar
    if (mutable_text) {
        char *token = strtok(mutable_text, ","); // Split por vírgulas
        while (token != NULL) {
            // Limpar espaços em branco ao redor do token, se necessário
            // ... lógica para limpar 'token' ...
            fprintf(output_file, "%s\n", token);
            token = strtok(NULL, ",");
        }
        free(mutable_text); // Libera a cópia
    } else {
         fprintf(stderr, "Erro ao duplicar string para split.\n");
         fprintf(output_file, "%s\n", generated_text); // Escreve o texto original como fallback
    }
    */


    fclose(output_file); // Fecha o arquivo

    printf("Texto gerado pela IA relacionado ao tema '%s' foi salvo em '%s'.\n", theme, output_file_path);

    // --- 10. Limpeza Final (Sucesso) ---
    cJSON_Delete(json); // Libera a estrutura JSON
    free(response.ptr); // Libera o buffer da resposta
    curl_slist_free_all(headers); // Libera os cabeçalhos
    curl_easy_cleanup(curl); // Limpa o manipulador CURL
}