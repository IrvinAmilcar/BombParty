#include "player.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

Vector2 CalculatePlayerPosition(int playerIndex, int totalPlayers, Vector2 center, float radius) {
    if (totalPlayers <= 0) {
        return center;
    }

    float angle_degrees = (360.0f / totalPlayers) * playerIndex + 90.0f;
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