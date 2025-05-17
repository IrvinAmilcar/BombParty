#include "leaderboard.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Leaderboard Data (defined here) ---
LeaderboardEntry leaderboard[MAX_LEADERBOARD_ENTRIES] = {0};
int leaderboardCount = 0;
// --- End Leaderboard Data ---


// Function prototypes for internal use
static void SortLeaderboard(); // Internal helper function


// Loads leaderboard entries from the file
void LoadLeaderboard() {
    FILE* file = fopen(LEADERBOARD_FILE, "r");
    if (file == NULL) {
        TraceLog(LOG_WARNING, TextFormat("Leaderboard file not found or error opening: %s. Starting with empty leaderboard.", LEADERBOARD_FILE));
        leaderboardCount = 0; // Start with an empty leaderboard if the file doesn't exist
        return;
    }

    leaderboardCount = 0;
    while (leaderboardCount < MAX_LEADERBOARD_ENTRIES &&
           fscanf(file, "%s %d", leaderboard[leaderboardCount].name, &leaderboard[leaderboardCount].score) == 2) {
        leaderboardCount++;
    }

    fclose(file);
    TraceLog(LOG_INFO, TextFormat("Leaderboard loaded with %d entries.", leaderboardCount));
}

// Saves the current leaderboard entries from memory to the file (overwriting previous content)
void SaveLeaderboard() {
    FILE* file = fopen(LEADERBOARD_FILE, "w"); // Use "w" to overwrite the file with the current top 10
    if (file == NULL) {
        TraceLog(LOG_ERROR, TextFormat("Error saving leaderboard file: %s", LEADERBOARD_FILE));
        return;
    }

    for (int i = 0; i < leaderboardCount; ++i) {
        fprintf(file, "%s %d\n", leaderboard[i].name, leaderboard[i].score);
    }

    fclose(file);
    TraceLog(LOG_INFO, TextFormat("Leaderboard saved with %d entries.", leaderboardCount));
}

// Adds a new score to the leaderboard and keeps it sorted (top MAX_LEADERBOARD_ENTRIES)
void AddToLeaderboard(const char* playerName, int playerScore) {
    // Load the existing leaderboard first
    LoadLeaderboard(); // Ensure we have the current state before adding

    // Create a temporary entry for the new score
    LeaderboardEntry newEntry;
    strncpy(newEntry.name, playerName, MAX_PLAYER_NAME_LEN - 1);
    newEntry.name[MAX_PLAYER_NAME_LEN - 1] = '\0';
    newEntry.score = playerScore;

    // If the new score is lower than the lowest score in a full leaderboard, no need to add
    if (leaderboardCount == MAX_LEADERBOARD_ENTRIES && playerScore <= leaderboard[leaderboardCount - 1].score) {
         TraceLog(LOG_INFO, TextFormat("New score %d from %s is not high enough to enter the top %d.", playerScore, playerName, MAX_LEADERBOARD_ENTRIES));
         return; // Exit early if score doesn't qualify
    }


    // Find the correct position to insert the new entry (insertion sort logic)
    int insertIndex = leaderboardCount;
    while (insertIndex > 0 && newEntry.score > leaderboard[insertIndex - 1].score) {
        insertIndex--;
    }

    // Shift entries down to make space for the new entry
    // We shift only if there's space or if we are replacing the lowest score
    if (leaderboardCount < MAX_LEADERBOARD_ENTRIES) {
        // Shift elements down from the end to the insertIndex
        for (int i = leaderboardCount; i > insertIndex; --i) {
            leaderboard[i] = leaderboard[i - 1];
        }
        leaderboardCount++; // Increment count as a new entry is added
    } else {
        // If the leaderboard is full, we only shift down until the insertIndex
        // The lowest score is effectively discarded
        for (int i = MAX_LEADERBOARD_ENTRIES - 1; i > insertIndex; --i) {
             leaderboard[i] = leaderboard[i - 1];
        }
    }


    // Insert the new entry at the correct position
    if (insertIndex < MAX_LEADERBOARD_ENTRIES) {
        leaderboard[insertIndex] = newEntry;
        TraceLog(LOG_INFO, TextFormat("New score %d from %s inserted into leaderboard at position %d.", playerScore, playerName, insertIndex + 1));
    } else {
        // This case should ideally not be reached if the initial check is correct,
        // but as a safeguard:
        TraceLog(LOG_WARNING, TextFormat("Attempted to insert score outside leaderboard bounds. Score: %d, Player: %s", playerScore, playerName));
    }


    // Save the updated leaderboard
    SaveLeaderboard();
}


// Sorts the leaderboard entries by score in descending order
// This function is used internally by AddToLeaderboard, but can be called separately if needed.
static void SortLeaderboard() {
    // Simple bubble sort
    for (int i = 0; i < leaderboardCount - 1; ++i) {
        for (int j = 0; j < leaderboardCount - i - 1; ++j) {
            if (leaderboard[j].score < leaderboard[j + 1].score) {
                // Swap
                LeaderboardEntry temp = leaderboard[j];
                leaderboard[j] = leaderboard[j + 1];
                leaderboard[j + 1] = temp;
            }
        }
    }
     TraceLog(LOG_INFO, "Leaderboard sorted.");
}

// Draws the leaderboard on the screen
void DrawLeaderboard(Font font, int screenWidth, int screenHeight) {
    ClearBackground(LIGHTGRAY);
    DrawText("LEADERBOARD", screenWidth/2 - MeasureText("LEADERBOARD", 40)/2, 50, 40, BLACK);

    int startY = 120;
    int textHeight = 30;
    int padding = 10;

    for (int i = 0; i < leaderboardCount; ++i) {
        char entryText[MAX_PLAYER_NAME_LEN + 20]; // Name + " - Score: " + Score
        snprintf(entryText, sizeof(entryText), "%d. %s - Score: %d", i + 1, leaderboard[i].name, leaderboard[i].score);

        int textWidth = MeasureTextEx(font, entryText, textHeight, 0).x;
        Vector2 textPos = { screenWidth/2 - textWidth/2, startY + i * (textHeight + padding) };

        DrawTextEx(font, entryText, textPos, textHeight, 0, DARKGRAY);
    }

    if (leaderboardCount == 0) {
        const char* noEntriesText = "Nenhuma pontuação no leaderboard ainda.";
        int textWidth = MeasureText(noEntriesText, 20);
        DrawText(noEntriesText, screenWidth/2 - textWidth/2, startY, 20, GRAY);
    }

    DrawText("<- Voltar (BACKSPACE)", 20, screenHeight - 30, 20, DARKGRAY);
}