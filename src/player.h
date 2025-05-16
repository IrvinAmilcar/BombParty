#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

#define MAX_PLAYER_NAME_LEN 32

typedef struct Player {
    char name[MAX_PLAYER_NAME_LEN];
    int lives;
    Vector2 screenPosition;
    int originalIndex;

    int powerUP;

    struct Player *next;
    struct Player *prev;

} Player;

Vector2 CalculatePlayerPosition(int playerIndex, int totalPlayers, Vector2 center, float radius);

void DrawPlayerInfo(const Player* player, Font font, Color textColor, Color lifeColor);

//Novinhas funny ações:

Player* CreatePlayer(const char* name, int lives, Vector2 position);

void AddPlayer(Player** head, Player* newPlayer);

void RemoveAllPlayers(Player** head);

#endif