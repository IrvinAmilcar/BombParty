#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include "raylib.h"
#include "player.h" // Assuming MAX_PLAYER_NAME_LEN is needed here
#include <stdbool.h>

// --- Leaderboard Definitions ---
#define LEADERBOARD_FILE "leaderboard.txt"
#define MAX_LEADERBOARD_ENTRIES 10

typedef struct {
    char name[MAX_PLAYER_NAME_LEN];
    int score;
} LeaderboardEntry;

// Declare the leaderboard array and count (extern because they are defined in the .c file)
extern LeaderboardEntry leaderboard[MAX_LEADERBOARD_ENTRIES];
extern int leaderboardCount;

// Function prototypes
void LoadLeaderboard();
void SaveLeaderboard();
void AddToLeaderboard(const char* playerName, int playerScore);
void DrawLeaderboard(Font font, int screenWidth, int screenHeight); // Function to draw the leaderboard
// --- End Leaderboard Definitions ---

#endif // LEADERBOARD_H