#include <stdio.h>
#include <stdlib.h>
#include "armazenarTxt.h"

char* lerArquivoParaString(const char* nomeArquivo) {
            FILE* arquivo = fopen(nomeArquivo, "r");
            if (arquivo == NULL) {
                perror("Erro ao abrir o arquivo");
                return NULL; // Retorna NULL em caso de erro
            }

            // Determina o tamanho do arquivo
            fseek(arquivo, 0, SEEK_END);
            long tamanhoArquivo = ftell(arquivo);
            fseek(arquivo, 0, SEEK_SET); // Retorna ao início do arquivo

            // Aloca memória para a string (inclui o terminador nulo)
            char* buffer = (char*)malloc(tamanhoArquivo + 1);
            if (buffer == NULL) {
                perror("Erro ao alocar memória");
                fclose(arquivo);
                return NULL; // Retorna NULL se a alocação falhar
            }

            // Lê o conteúdo do arquivo para o buffer
            size_t bytesLidos = fread(buffer, 1, tamanhoArquivo, arquivo);
            if (bytesLidos != tamanhoArquivo) {
                fprintf(stderr, "Erro ao ler o arquivo: leu %zu de %ld bytes\n", bytesLidos, tamanhoArquivo);
                fclose(arquivo);
                free(buffer);
                return NULL;
            }
            buffer[tamanhoArquivo] = '\0'; // Garante que a string é terminada com nulo

            fclose(arquivo);
            return buffer;
        }