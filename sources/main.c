#include "raylib.h"
#include "raymath.h"
#include "character.h"
#include "npc.h"
#include "item.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #define POPEN _popen
    #define PCLOSE _pclose
    #define PYTHON_CMD "python"
#else
    #define POPEN popen
    #define PCLOSE pclose
    #define PYTHON_CMD "python3"
#endif

#define SCREEN_WIDTH (1024)
#define SCREEN_HEIGHT (576)
#define WINDOW_TITLE "Lego Western Town - AI NPCs!"

// --- Global UI State ---
bool isDialogOpen = false;
char activeDialogText[1024] = ""; // Changed to a buffer so we can write Gemini's response to it
const char* activeNPCName = "";
bool isWaitingForAI = false;
NPC* interactingNPC = NULL;

bool IsBlockedByCollisionMask(Vector2 worldPos, const Color *maskPixels, int maskWidth, int maskHeight) {
    int x = (int)worldPos.x;
    int y = (int)worldPos.y;

    if (x < 0 || y < 0 || x >= maskWidth || y >= maskHeight) {
        return true;
    }

    Color pixel = maskPixels[y * maskWidth + x];
    return (pixel.a > 0 && pixel.r < 20 && pixel.g < 20 && pixel.b < 20);
}

// --- Python Hook Function ---
void GenerateGeminiDialog(const char* npcName, const char* itemName, char* buffer, size_t bufferSize) {
    char command[512];
    
    // Format the command dynamically based on the operating system
    snprintf(command, sizeof(command), "%s gemini_dialog.py \"%s\" \"%s\"", PYTHON_CMD, npcName, itemName);

    // Open the pipe using our cross-platform macro
    FILE *fp = POPEN(command, "r");
    if (fp == NULL) {
        snprintf(buffer, bufferSize, "Error: Could not run Python script.");
        printf("ERROR: Python script failed to execute.\n"); 
        return;
    }

    buffer[0] = '\0'; // Clear buffer
    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        strncat(buffer, line, bufferSize - strlen(buffer) - 1);
    }
    
    // Close the pipe using our cross-platform macro
    PCLOSE(fp);

    // Debug logs
    printf("\n--- GEMINI DIALOG GENERATED ---\n");
    printf("NPC: %s\n", npcName);
    printf("Text: %s\n", buffer);
    printf("-------------------------------\n\n");
}

// --- Text Wrapping Helper ---
// Modifies a string in-place, replacing spaces with newlines to fit a maximum pixel width.
void WrapText(char *text, int maxLineWidth, int fontSize) {
    int length = strlen(text);
    int lineStart = 0;
    int lastSpace = -1;
    char temp[1024];

    for (int i = 0; i < length; i++) {
        if (text[i] == ' ') lastSpace = i;
        if (text[i] == '\n') { // Reset if Gemini happened to generate a newline
            lineStart = i + 1;
            continue;
        }

        // Copy current line into a temporary buffer to measure it
        int currentLength = i - lineStart + 1;
        if (currentLength >= sizeof(temp)) currentLength = sizeof(temp) - 1; // Safegaurd
        
        strncpy(temp, text + lineStart, currentLength);
        temp[currentLength] = '\0';

        // Check if the current chunk of text exceeds our box width
        if (MeasureText(temp, fontSize) > maxLineWidth) {
            if (lastSpace > lineStart) {
                text[lastSpace] = '\n';     // Replace the last space with a newline
                lineStart = lastSpace + 1;  // Update the start of the new line
                i = lineStart - 1;          // Backtrack loop to measure properly from the new line
            } else {
                // Fallback: Force a break if a single word is somehow wider than the whole box
                text[i] = '\n';
                lineStart = i + 1;
            }
        }
    }
}

// Function to handle the interaction logic
void InteractWithNPC(NPC* npc, Character* player) {
    activeNPCName = npc->name;
    
    if (npc->questCompleted) {
        snprintf(activeDialogText, sizeof(activeDialogText), "Much obliged for your help earlier, partner!");
    } 
    else if (player->heldItem != NULL && strcmp(player->heldItem, npc->questItem) == 0) {
        // Player has the item!
        snprintf(activeDialogText, sizeof(activeDialogText), "Well I'll be! You found my %s. Thank ye kindly!", npc->questItem);
        npc->questCompleted = true;
        player->heldItem = NULL; // Consume the item
    } 
    else {
        // Generate dynamic quest dialog using Gemini
        GenerateGeminiDialog(npc->name, npc->questItem, activeDialogText, sizeof(activeDialogText));
    }
    
    WrapText(activeDialogText, 560, 20); // 560 max width, 20 font size
    isDialogOpen = true;
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);
    srand(time(NULL)); // Seed random number generator

    Texture2D background = LoadTexture("assets/bg.png");
    Image collisionMaskImage = LoadImage("assets/collision.png");
    Color *collisionMaskPixels = NULL;

    if (collisionMaskImage.data != NULL) {
        collisionMaskPixels = LoadImageColors(collisionMaskImage);
    } else {
        printf("WARNING: Could not load assets/collision.png. Movement will ignore collision mask.\n");
    }

    Character player;
    InitCharacter(&player, (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f }, "assets/character.png");

    NPC sheriff;
    InitNPC(&sheriff, (Vector2){ 500.0f, 300.0f }, "Sheriff Burbrick", "assets/sheriff.png");
    sheriff.questItem = "Lost Badge"; // Assign quests
    
    NPC garry;
    InitNPC(&garry, (Vector2){ 800.0f, 500.0f }, "Gunslinger Gary", "assets/gary.png");
    garry.questItem = "Lucky Horseshoe";

    // Initialize Random Items
    Item items[2];
    items[0] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Lost Badge", true };
    items[1] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Lucky Horseshoe", true };

    Camera2D camera = { 0 };
    camera.zoom = 2.0f; 
    camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };

    while (!WindowShouldClose())
    {
        Vector2 mouseScreenPos = GetMousePosition();
        Vector2 mouseWorldPos = GetScreenToWorld2D(mouseScreenPos, camera);

        // --- Input Logic ---
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (isDialogOpen) {
                Rectangle okBtn = { SCREEN_WIDTH / 2.0f - 50, SCREEN_HEIGHT / 2.0f + 60, 100, 40 };
                if (CheckCollisionPointRec(mouseScreenPos, okBtn)) {
                    isDialogOpen = false; 
                }
            } 
	    // 2. If Dialog is closed, we check for World interactions
	    else {
		    NPC* clickedNPC = NULL;
		    if (IsNPCClicked(&sheriff, mouseWorldPos)) clickedNPC = &sheriff;
		    else if (IsNPCClicked(&garry, mouseWorldPos)) clickedNPC = &garry;

		    if (clickedNPC != NULL) {
			    activeNPCName = clickedNPC->name;
			    isDialogOpen = true;

			    // Instant Response: Quest is already done
			    if (clickedNPC->questCompleted) {
				    snprintf(activeDialogText, sizeof(activeDialogText), "Much obliged for your help earlier, partner!");
			    } 
			    // Instant Response: Player has the item
			    else if (player.heldItem != NULL && strcmp(player.heldItem, clickedNPC->questItem) == 0) {
				    snprintf(activeDialogText, sizeof(activeDialogText), "Well I'll be! You found my %s. Thank ye kindly!", clickedNPC->questItem);
				    clickedNPC->questCompleted = true;
				    player.heldItem = NULL; // Consume the item
			    } 
			    // Delayed AI Response: Trigger the "Hmm..." loading state
			    else {
				    snprintf(activeDialogText, sizeof(activeDialogText), "Hmm...");
				    isWaitingForAI = true;        // Flag that we need to call Gemini
				    interactingNPC = clickedNPC;  // Remember who we are talking to
			    }
		    } 
		    else {
			    // Only move the player if we didn't click on an NPC
                bool canMoveToTarget = true;

                if (collisionMaskPixels != NULL) {
                    canMoveToTarget = !IsBlockedByCollisionMask(mouseWorldPos, collisionMaskPixels, collisionMaskImage.width, collisionMaskImage.height);
                }

                if (canMoveToTarget) {
                    player.targetPosition = mouseWorldPos;
                }
		    }
	    }
        }

        // --- Update Logic ---
        if (!isDialogOpen) {
            UpdateCharacter(&player);

            // Check for item pickup
            for (int i = 0; i < 2; i++) {
                if (items[i].active && Vector2Distance(player.position, items[i].position) < 20.0f) {
                    items[i].active = false;
                    player.heldItem = items[i].name; 
                }
            }
        }

        // Camera Logic
        camera.target = player.position;

        // --- Drawing ---
        BeginDrawing();
        ClearBackground(DARKGRAY);

        BeginMode2D(camera);

        if (background.id != 0) {
            DrawTextureEx(background, (Vector2){0,0}, 0.0f, 1.0f, WHITE);
        }

        // Draw active items (yellow squares)
        for (int i = 0; i < 2; i++) {
            if (items[i].active) {
                DrawRectangle(items[i].position.x - 5, items[i].position.y - 5, 10, 10, GOLD);
                DrawText(items[i].name, items[i].position.x - 10, items[i].position.y - 15, 10, RAYWHITE);
            }
        }

        DrawNPC(&sheriff);
        DrawNPC(&garry);
        DrawCharacter(&player);

        EndMode2D();

        // -- SCREEN SPACE DRAWING (UI) --
        if (isDialogOpen) {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.6f));

            Rectangle dialogRec = { SCREEN_WIDTH / 2.0f - 300, SCREEN_HEIGHT / 2.0f - 100, 600, 250 };
            DrawRectangleRec(dialogRec, RAYWHITE);
            DrawRectangleLinesEx(dialogRec, 4, DARKGRAY);

            // NPC Name Header
            DrawText(activeNPCName, (int)dialogRec.x + 20, (int)dialogRec.y + 15, 24, DARKBLUE);
            DrawLine((int)dialogRec.x + 20, (int)dialogRec.y + 45, (int)dialogRec.x + 580, (int)dialogRec.y + 45, GRAY);

            // Render the AI Dialog (Using a slightly smaller font so it fits)
            DrawText(activeDialogText, (int)dialogRec.x + 20, (int)dialogRec.y + 60, 20, BLACK);

            Rectangle okBtn = { SCREEN_WIDTH / 2.0f - 50, SCREEN_HEIGHT / 2.0f + 60, 100, 40 };
            bool isHovering = CheckCollisionPointRec(mouseScreenPos, okBtn);
            DrawRectangleRec(okBtn, isHovering ? LIGHTGRAY : GRAY);
            DrawRectangleLinesEx(okBtn, 2, BLACK);
            DrawText("Okay!", (int)okBtn.x + 25, (int)okBtn.y + 10, 20, BLACK);
        }

        // Top-Left UI 
        DrawRectangle(10, 10, 310, 50, Fade(BLACK, 0.7f));
        DrawText("Click an NPC to talk to them!", 20, 15, 18, RAYWHITE);
        
        // Inventory UI
        if (player.heldItem) {
            DrawText(TextFormat("Holding: %s", player.heldItem), 20, 35, 16, GOLD);
        } else {
            DrawText("Holding: Nothing", 20, 35, 16, LIGHTGRAY);
        }

        EndDrawing();

	if (isWaitingForAI && interactingNPC != NULL) {
            // The game will freeze on this line while Python runs
            GenerateGeminiDialog(interactingNPC->name, interactingNPC->questItem, activeDialogText, sizeof(activeDialogText));
            
            // Format the new text to fit the box
            WrapText(activeDialogText, 560, 20); 
            
            // Reset our loading flags
            isWaitingForAI = false;
            interactingNPC = NULL;
        }
    }

    UnloadNPC(&sheriff);
    UnloadNPC(&garry);
    UnloadCharacter(&player);
    UnloadTexture(background); 
    if (collisionMaskPixels != NULL) {
        UnloadImageColors(collisionMaskPixels);
    }
    if (collisionMaskImage.data != NULL) {
        UnloadImage(collisionMaskImage);
    }
    CloseWindow();

    return 0;
}
