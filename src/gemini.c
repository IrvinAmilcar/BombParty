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
// Corrija AQUI o nome do modelo para 'gemini-1.0-pro'
#define GEMINI_API_ENDPOINT "v1beta/models/gemini-1.5-flash-latest:generateContent?key=" API_KEY

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
    sbInit(&response);
    fprintf(stderr, "DEBUG: StringBuf inicializado.\n");

    struct curl_slist *headers = NULL;
    cJSON *request_json = NULL; // Ponteiro para a estrutura JSON da requisição
    char *request_body_str = NULL; // Ponteiro para a string do corpo da requisição


    // --- 1. Construir a URL com a chave API ---
    char url[2048];
    snprintf(url, sizeof(url), "%s/%s", GEMINI_API_BASE_URL, GEMINI_API_ENDPOINT); // Adiciona a barra '/'
    fprintf(stderr, "DEBUG: URL construida: %s\n", url);


    // --- 2. Preparar o corpo da requisição (Construindo JSON com cJSON) ---
    char prompt_text[512];
    snprintf(prompt_text, sizeof(prompt_text), "Liste até %d palavras relacionadas a \"%s\". Separe as palavras por vírgulas. Retorne apenas as palavras em Português. Não inclua frases de introdução ou conclusão.", MAX_PALAVRAS, theme);
    fprintf(stderr, "DEBUG: Prompt construído: %s\n", prompt_text);

    // Construir a estrutura JSON programaticamente
    request_json = cJSON_CreateObject();
    if (!request_json) {
        fprintf(stderr, "DEBUG: Erro ao criar objeto JSON raiz para requisicao.\n");
        // LIMPEZA
        curl_slist_free_all(headers); // headers é NULL aqui, mas boa prática de limpeza
        curl_easy_cleanup(curl);
        free(response.ptr); // response.ptr é NULL aqui, mas boa prática
        fprintf(stderr, "DEBUG: Limpeza apos erro cJSON_CreateObject (raiz).\n");
        return;
    }

    cJSON *contents_array = cJSON_CreateArray();
     if (!contents_array) {
         fprintf(stderr, "DEBUG: Erro ao criar array 'contents' para requisicao.\n");
         // LIMPEZA
         cJSON_Delete(request_json); // Libera o objeto JSON já criado
         curl_slist_free_all(headers);
         curl_easy_cleanup(curl);
         free(response.ptr);
         fprintf(stderr, "DEBUG: Limpeza apos erro cJSON_CreateArray (contents).\n");
         return;
     }
     cJSON_AddItemToObject(request_json, "contents", contents_array);


    cJSON *content_item = cJSON_CreateObject();
      if (!content_item) {
          fprintf(stderr, "DEBUG: Erro ao criar objeto 'content' para requisicao.\n");
          // LIMPEZA
          cJSON_Delete(request_json);
          curl_slist_free_all(headers);
          curl_easy_cleanup(curl);
          free(response.ptr);
          fprintf(stderr, "DEBUG: Limpeza apos erro cJSON_CreateObject (content).\n");
          return;
      }
      cJSON_AddItemToArray(contents_array, content_item);


     cJSON *parts_array = cJSON_CreateArray();
       if (!parts_array) {
           fprintf(stderr, "DEBUG: Erro ao criar array 'parts' para requisicao.\n");
           // LIMPEZA
           cJSON_Delete(request_json);
           curl_slist_free_all(headers);
           curl_easy_cleanup(curl);
           free(response.ptr);
           fprintf(stderr, "DEBUG: Limpeza apos erro cJSON_CreateArray (parts).\n");
           return;
       }
       cJSON_AddItemToObject(content_item, "parts", parts_array);

    cJSON *text_item = cJSON_CreateObject();
      if (!text_item) {
          fprintf(stderr, "DEBUG: Erro ao criar objeto 'text' para requisicao.\n");
          // LIMPEZA
          cJSON_Delete(request_json);
          curl_slist_free_all(headers);
          curl_easy_cleanup(curl);
          free(response.ptr);
          fprintf(stderr, "DEBUG: Limpeza apos erro cJSON_CreateObject (text).\n");
          return;
      }
      // ADICIONA O PROMPT COMO STRING - cJSON CUIDA DA ESCAPAGEM
      cJSON_AddStringToObject(text_item, "text", prompt_text);
      cJSON_AddItemToArray(parts_array, text_item);

    // --- ADICIONA O OBJETO generationConfig E O PARAMETRO maxOutputTokens DENTRO DELE ---
    cJSON *generation_config = cJSON_CreateObject();
    if (!generation_config) {
         fprintf(stderr, "DEBUG: Erro ao criar objeto 'generationConfig' para requisicao.\n");
         // LIMPEZA
         cJSON_Delete(request_json);
         curl_slist_free_all(headers);
         curl_easy_cleanup(curl);
         free(response.ptr);
         fprintf(stderr, "DEBUG: Limpeza apos erro cJSON_CreateObject (generationConfig).\n");
         return;
    }
    cJSON_AddNumberToObject(generation_config, "maxOutputTokens", 800);
    cJSON_AddItemToObject(request_json, "generationConfig", generation_config); // Adiciona o objeto generationConfig ao root


    // Imprime a estrutura JSON para obter a string do corpo da requisição
    request_body_str = cJSON_PrintUnformatted(request_json);
     if (!request_body_str) {
         fprintf(stderr, "DEBUG: Erro ao imprimir estrutura JSON para string.\n");
         // LIMPEZA
         cJSON_Delete(request_json); // Libera a estrutura JSON criada
         curl_slist_free_all(headers);
         curl_easy_cleanup(curl);
         free(response.ptr);
         fprintf(stderr, "DEBUG: Limpeza apos erro cJSON_PrintUnformatted.\n");
         return;
     }
     fprintf(stderr, "DEBUG: Corpo da requisicao JSON construido com cJSON: %s\n", request_body_str);


    // --- 3. Configurar os cabeçalhos ---
    headers = curl_slist_append(headers, "Content-Type: application/json");
    fprintf(stderr, "DEBUG: Cabecalho Content-Type adicionado.\n");


    // --- 4. Configurar CURL para POST ---
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body_str); // Usa a string gerada pelo cJSON
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(request_body_str)); // Define o tamanho

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
        cJSON_Delete(request_json); // Libera a estrutura JSON da requisicao
        cJSON_free(request_body_str); // Libera a string do corpo da requisicao
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
         fprintf(stderr, "DEBUG: Status HTTP indica erro (%ld). A API retornou um erro.\n", http_code);
         // A resposta bruta já foi impressa acima. Se ela for JSON de erro,
         // o parsing/navegação abaixo pode falhar e imprimir mais detalhes.
     }


    // Verifica se a resposta foi recebida no buffer e tem algum conteudo (mesmo que seja JSON de erro)
    if (response.ptr == NULL || response.len == 0) {
         fprintf(stderr, "DEBUG: Resposta da API vazia ou nula apos perform.\n");
         // LIMPEZA
         free(response.ptr); // Ainda tentar liberar, mesmo que NULL
         curl_slist_free_all(headers);
         curl_easy_cleanup(curl);
         cJSON_Delete(request_json);
         cJSON_free(request_body_str);
         fprintf(stderr, "DEBUG: Limpeza apos resposta vazia.\n");
         return;
    }
    fprintf(stderr, "DEBUG: Resposta recebida no buffer (tamanho %zu).\n", response.len);


    // --- 7. Processar a resposta (Parsing JSON) ---
    cJSON *response_json = cJSON_Parse(response.ptr); // response.ptr contém o JSON da resposta
    if (!response_json) {
        const char *parse_error = cJSON_GetErrorPtr();
        if (parse_error) {
            fprintf(stderr, "DEBUG: Erro ao analisar a resposta JSON da API (antes: %s).\n", parse_error);
        } else {
             fprintf(stderr, "DEBUG: Erro desconhecido ao analisar a resposta JSON da API.\n");
        }
        fprintf(stderr, "DEBUG: Resposta bruta que falhou o parsing JSON:\n%s\n", response.ptr); // Imprimir novamente a bruta se o parse falhou

        // LIMPEZA em caso de erro JSON parse
        free(response.ptr);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        cJSON_Delete(request_json); // Libera a estrutura JSON da requisicao
        cJSON_free(request_body_str); // Libera a string do corpo da requisicao
        // response_json é NULL aqui, não precisa deletar
        fprintf(stderr, "DEBUG: Limpeza apos erro JSON parse.\n");
        return;
    }
    fprintf(stderr, "DEBUG: Resposta JSON parseada com sucesso.\n");


    // --- 8. Navegar na estrutura JSON da resposta da API Gemini ---
    // Verifica se eh uma resposta de sucesso com "candidates" ou uma resposta de erro com "error"
    const cJSON *candidates = cJSON_GetObjectItemCaseSensitive(response_json, "candidates");
    const cJSON *error_obj = cJSON_GetObjectItemCaseSensitive(response_json, "error"); // Verifica se tem objeto de erro

    const char *generated_text = NULL;

    if (error_obj && cJSON_IsObject(error_obj)) {
         // A resposta eh um objeto de erro da API
         fprintf(stderr, "DEBUG: A resposta da API eh um objeto de ERRO.\n");
         char *error_string_dbg = cJSON_Print(response_json);
         if(error_string_dbg) {
            fprintf(stderr, "DEBUG: Conteudo do objeto de ERRO:\n%s\n", error_string_dbg);
            cJSON_free(error_string_dbg);
         }
         // Não há generated_text em uma resposta de erro. Sair.
         // generated_text continua NULL. O código abaixo vai para o tratamento de generated_text == NULL.

    } else if (cJSON_IsArray(candidates) && cJSON_GetArraySize(candidates) > 0) {
        // A resposta eh de sucesso e contem 'candidates' array
        fprintf(stderr, "DEBUG: A resposta da API contem 'candidates' array valido.\n");
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
                             fprintf(stderr, "DEBUG: Texto gerado extraido com sucesso (primeiras 100 chars): '%.100s%s'\n", generated_text, strlen(generated_text) > 100 ? "..." : "");
                         } else {
                             fprintf(stderr, "DEBUG: Nao foi possivel encontrar/extrair campo 'text' ou nao eh string valida no primeiro part.\n");
                         }
                     } else {
                          fprintf(stderr, "DEBUG: Nao foi possivel encontrar/extrair primeiro item de 'parts' como objeto.\n");
                     }
                 } else {
                      fprintf(stderr, "DEBUG: Nao foi possivel encontrar/extrair 'parts' array valido ou vazio no content.\n");
                 }
            } else {
                 fprintf(stderr, "DEBUG: Nao foi possivel encontrar/extrair 'content' object valido no primeiro candidate.\n");
            }
        } else {
             fprintf(stderr, "DEBUG: Nao foi possivel encontrar/extrair primeiro item de 'candidates' como objeto.\n");
        }

    } else {
        // Resposta JSON valida, mas nao eh erro E nao eh sucesso esperado (candidates)
        fprintf(stderr, "DEBUG: Resposta JSON parseada, mas nao eh objeto de erro nem resposta de sucesso esperada ('candidates' ausente ou invalido).\n");
        char *json_string_dbg = cJSON_Print(response_json);
         if(json_string_dbg) {
            fprintf(stderr, "DEBUG: Conteudo do JSON inesperado:\n%s\n", json_string_dbg);
            cJSON_free(json_string_dbg);
         }
    }


    // Verificar se conseguimos extrair o texto gerado (so entra aqui se generated_text ainda for NULL)
    if (generated_text == NULL) {
        fprintf(stderr, "DEBUG: generated_text final eh NULL. Nao foi possivel extrair o texto gerado.\n");

        // LIMPEZA
        cJSON_Delete(response_json); // Libera a estrutura JSON da resposta
        free(response.ptr);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        cJSON_Delete(request_json); // Libera a estrutura JSON da requisicao
        cJSON_free(request_body_str); // Libera a string do corpo da requisicao
        fprintf(stderr, "DEBUG: Limpeza apos generated_text ser NULL.\n");
        return;
    }


    // --- 9. Processar o texto gerado e escrever no arquivo ---
    fprintf(stderr, "DEBUG: Tentando abrir o arquivo de saida '%s' no modo 'w'.\n", output_file_path);
    FILE *output_file = fopen(output_file_path, "w"); // Abre o arquivo no modo escrita (cria ou sobrescreve)
    if (!output_file) {
        fprintf(stderr, "DEBUG: Erro fatal ao abrir o arquivo de saida '%s'. Permissao? Caminho?\n", output_file_path);
        // LIMPEZA em caso de erro de arquivo
        cJSON_Delete(response_json); // Libera a estrutura JSON da resposta
        free(response.ptr);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        cJSON_Delete(request_json); // Libera a estrutura JSON da requisicao
        cJSON_free(request_body_str); // Libera a string do corpo da requisicao
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
    cJSON_Delete(response_json); // Libera a estrutura JSON da resposta
    free(response.ptr); // Libera o buffer da resposta
    curl_slist_free_all(headers); // Libera os cabeçalhos
    curl_easy_cleanup(curl); // Limpa o manipulador CURL
    cJSON_Delete(request_json); // Libera a estrutura JSON da requisicao
    cJSON_free(request_body_str); // Libera a string do corpo da requisicao
    fprintf(stderr, "DEBUG: Limpeza final concluida.\n");
}