#include "player.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

Vector2 CalculatePlayerPosition(int playerIndex, int totalPlayers, Vector2 center, float radius) {
    if (totalPlayers <= 0) {
        return center;
    }

    float angle_degrees = (360.0f / totalPlayers) * playerIndex + 180.0f;
    float angle_radians = angle_degrees * DEG2RAD;

    Vector2 position;
    position.x = center.x + radius * cosf(angle_radians);
    position.y = center.y + radius * sinf(angle_radians);

    return position;
}

void DrawPlayerInfo(const Player* player, Font font, Color textColor, Color lifeColor) {
    if (player == NULL) {
        return;
    }

    float nameWidth = MeasureTextEx(font, player->name, 20, 0).x;
    Vector2 namePos = { player->screenPosition.x - nameWidth / 2.0f, player->screenPosition.y - 25 };

    DrawTextEx(font, player->name, namePos, 20, 0, textColor);

    char livesText[32];
    snprintf(livesText, sizeof(livesText), "Vidas: %d", player->lives);

    float livesWidth = MeasureTextEx(font, livesText, 18, 0).x;
    Vector2 livesPos = { player->screenPosition.x - livesWidth / 2.0f, player->screenPosition.y + 10 };

    DrawTextEx(font, livesText, livesPos, 18, 0, lifeColor);
}

Player *CreatePlayer(const char* name, int lives, Vector2 position){
    Player *newPlayer = malloc(sizeof(Player));

    if (newPlayer == NULL){
        return NULL;
    }

    strncpy(newPlayer->name, name, MAX_PLAYER_NAME_LEN);
    newPlayer->name[MAX_PLAYER_NAME_LEN - 1] = '\0';

    newPlayer -> lives = lives;
    newPlayer -> screenPosition = position;
    newPlayer -> next = NULL;
    newPlayer -> prev = NULL;

    return newPlayer;
}

void AddPlayer(Player** head, Player* newPlayer){

    //Primeiro caso (Nenhum elemento adicionado a lista!)
    if ((*head) == NULL){
        *head = newPlayer;
        newPlayer -> next = newPlayer;
        newPlayer -> prev = newPlayer;

    } else {
        // Lista já possui pelo menos um elemento
        Player* ultimo = (*head)->prev;

        // Ajusta os ponteiros para inserir no início
        newPlayer->next = *head;
        newPlayer->prev = ultimo;
        (*head)->prev = newPlayer;
        ultimo->next = newPlayer;

        // Atualiza o head para o novo jogador
        *head = newPlayer;
    }
}

void RemoveAllPlayers(Player **head) {
    if (*head == NULL) {
        return; // Lista já vazia
    }

    Player *current = *head;
    Player *temp;

    // Percorrer todos os jogadores da lista
    do {
        temp = current;
        current = current->next;

        // Liberar a memória do jogador atual
        free(temp);
    } while (current != *head); // Termina quando volta para o primeiro jogador

    // Definir o head como NULL para indicar que a lista está vazia
    *head = NULL;
}
