#include "player.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

void DrawPlayerInfo(const Player* player, Font font, Color textColor, Color lifeColor, Texture2D wizardTexture, Rectangle frameRec) {
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

    if (wizardTexture.id != 0) 
    {

        float offset_y = 0;

        Vector2 wizardDrawPos = {
            player->screenPosition.x - frameRec.width / 1.35f,
            namePos.y + MeasureTextEx(font, player->name, 20, 0).y + offset_y
        };

        DrawTextureRec(wizardTexture, frameRec, wizardDrawPos, WHITE);
    }
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
    newPlayer -> powerUP = 0;
    newPlayer -> originalIndex = 0;
    newPlayer -> score = 0;

    return newPlayer;
}

void AddPlayer(Player** head, Player* newPlayer){

    if ((*head) == NULL){
        *head = newPlayer;
        newPlayer -> next = newPlayer;
        newPlayer -> prev = newPlayer;

    } else {
        Player* ultimo = (*head)->prev;

        newPlayer->next = *head;
        newPlayer->prev = ultimo;
        (*head)->prev = newPlayer;
        ultimo->next = newPlayer;

        *head = newPlayer;
    }
}

void RemoveAllPlayers(Player **head) {
    if (*head == NULL) {
        return; 
    }

    Player *current = *head;
    Player *temp;

    do {
        temp = current;
        current = current->next;

        free(temp);
    } while (current != *head); 

    *head = NULL;
}
