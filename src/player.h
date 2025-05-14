#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

#define MAX_PLAYER_NAME_LEN 32

typedef struct {
    char name[MAX_PLAYER_NAME_LEN];
    int lives;
    Vector2 screenPosition;
} Player;

Vector2 CalculatePlayerPosition(int playerIndex, int totalPlayers, Vector2 center, float radius);

void drawPlayerInfo(const Player* player, Font font, Color textColor, Color lifeColor);

#endif