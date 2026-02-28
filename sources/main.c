// =============================================================================
//  main.c  –  Merged: scene-switching + collision (ours) & 9 NPCs (theirs)
// =============================================================================
#include "raylib.h"
#include "raymath.h"
#include "scene_manager.h"   // scene switching, currentScene, currentBackground
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

    // Scene manager owns the background texture from here on
    InitScenes();
    Image collisionMaskImage = LoadImage("assets/collision.png");
    Color *collisionMaskPixels = NULL;

    if (collisionMaskImage.data != NULL) {
        collisionMaskPixels = LoadImageColors(collisionMaskImage);
    } else {
        printf("WARNING: Could not load assets/collision.png. Movement will ignore collision mask.\n");
    }

    Character player;
    InitCharacter(&player, (Vector2){ 2500.0f, 1400.0f }, "assets/character.png");

    // --- NPC Initialization ---
    NPC sheriff, garry, dale, susan, kitty, buster, tommy, barry, marley;

    InitNPC(&sheriff, (Vector2){ 500.0f, 300.0f }, "Sheriff Burbrick", "assets/sheriff.png");
    sheriff.questItem = "Lost Badge";

    InitNPC(&garry, (Vector2){ 800.0f, 500.0f }, "Gunslinger Gary", "assets/gary.png");
    garry.questItem = "Lucky Horseshoe";

    InitNPC(&dale, (Vector2){ 260.0f, 270.0f }, "Dynamite Dale", "assets/dale.png");
    dale.questItem = "TNT Plunger";

    InitNPC(&susan, (Vector2){ 800.0f, 480.0f }, "Stable Susan", "assets/susan.png");
    susan.questItem = "Golden Saddle";

    InitNPC(&kitty, (Vector2){ 500.0f, 450.0f }, "Kitty", "assets/kitty.png");
    kitty.questItem = "Feather Boa";

    InitNPC(&buster, (Vector2){ 860.0f, 680.0f }, "Buster the Bandit", "assets/buster.png");
    buster.questItem = "Stolen Loot";

    InitNPC(&tommy, (Vector2){ 350.0f, 680.0f }, "Tommy Treasurer", "assets/tommy.png");
    tommy.questItem = "Ledger";

    InitNPC(&barry, (Vector2){ 640.0f, 620.0f }, "Barry the Barkeep", "assets/barry.png");
    barry.questItem = "Special Whiskey";

    InitNPC(&marley, (Vector2){ 680.0f, 580.0f }, "Marley the Musician", "assets/marley.png");
    marley.questItem = "Tuning Fork";

    // --- Initialize Items (9 total, one per NPC) ---
    Item items[9];
    items[0] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Lost Badge",      true };
    items[1] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Lucky Horseshoe", true };
    items[2] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "TNT Plunger",     true };
    items[3] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Golden Saddle",   true };
    items[4] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Feather Boa",     true };
    items[5] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Stolen Loot",     true };
    items[6] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Ledger",          true };
    items[7] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Special Whiskey", true };
    items[8] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Tuning Fork",     true };

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
            } else {
                // Check which NPC was clicked (all 9)
                NPC* clickedNPC = NULL;
                if      (IsNPCClicked(&sheriff, mouseWorldPos)) clickedNPC = &sheriff;
                else if (IsNPCClicked(&garry,   mouseWorldPos)) clickedNPC = &garry;
                else if (IsNPCClicked(&dale,    mouseWorldPos)) clickedNPC = &dale;
                else if (IsNPCClicked(&susan,   mouseWorldPos)) clickedNPC = &susan;
                else if (IsNPCClicked(&kitty,   mouseWorldPos)) clickedNPC = &kitty;
                else if (IsNPCClicked(&buster,  mouseWorldPos)) clickedNPC = &buster;
                else if (IsNPCClicked(&tommy,   mouseWorldPos)) clickedNPC = &tommy;
                else if (IsNPCClicked(&barry,   mouseWorldPos)) clickedNPC = &barry;
                else if (IsNPCClicked(&marley,  mouseWorldPos)) clickedNPC = &marley;

                if (clickedNPC != NULL) {
                    activeNPCName = clickedNPC->name;
                    isDialogOpen  = true;

                    if (clickedNPC->questCompleted) {
                        snprintf(activeDialogText, sizeof(activeDialogText),
                                 "Much obliged for your help earlier, partner!");
                    } else if (player.heldItem != NULL &&
                               strcmp(player.heldItem, clickedNPC->questItem) == 0) {
                        snprintf(activeDialogText, sizeof(activeDialogText),
                                 "Well I'll be! You found my %s. Thank ye kindly!", clickedNPC->questItem);
                        clickedNPC->questCompleted = true;
                        player.heldItem = NULL;
                    } else {
                        snprintf(activeDialogText, sizeof(activeDialogText), "Hmm...");
                        isWaitingForAI = true;
                        interactingNPC = clickedNPC;
                    }
                } else {
                    // Move player – collision only applies in the main town
                    bool canMove = true;
                    if (collisionMaskPixels != NULL && currentScene == SCENE_MAIN_TOWN) {
                        canMove = !IsBlockedByCollisionMask(mouseWorldPos, collisionMaskPixels,
                                                            collisionMaskImage.width,
                                                            collisionMaskImage.height);
                    }
                    if (canMove) {
                        player.targetPosition = mouseWorldPos;
                    }
                }
            }
        }

        // --- Update Logic ---
        if (!isDialogOpen) {
            UpdateCharacter(&player);

            // Check for a scene transition every frame
            CheckForSceneSwitch(player.position, &player);

            // Item pickup only makes sense in the main town
            if (currentScene == SCENE_MAIN_TOWN) {
                for (int i = 0; i < 9; i++) {
                    if (items[i].active && Vector2Distance(player.position, items[i].position) < 20.0f) {
                        items[i].active = false;
                        player.heldItem = items[i].name;
                    }
                }
            }
        }

        // Camera Logic
        camera.target = player.position;

        // --- Drawing ---
        BeginDrawing();
        ClearBackground(DARKGRAY);

        BeginMode2D(camera);

        if (currentBackground.id != 0) {
            DrawTextureEx(currentBackground, (Vector2){0,0}, 0.0f, 1.0f, WHITE);
        }

        // NPCs, items, and entrance markers are only in the main town
        if (currentScene == SCENE_MAIN_TOWN) {
            // Draw active items (yellow squares)
            for (int i = 0; i < 9; i++) {
                if (items[i].active) {
                    DrawRectangle(items[i].position.x - 5, items[i].position.y - 5, 10, 10, GOLD);
                    DrawText(items[i].name, items[i].position.x - 10, items[i].position.y - 15, 10, RAYWHITE);
                }
            }

            DrawNPC(&sheriff);
            DrawNPC(&garry);
            DrawNPC(&dale);
            DrawNPC(&susan);
            DrawNPC(&kitty);
            DrawNPC(&buster);
            DrawNPC(&tommy);
            DrawNPC(&barry);
            DrawNPC(&marley);

            // Teleport zone markers – walk into these to switch scene
            DrawRectangleLinesEx((Rectangle){ 2383, 1812, 80, 60 }, 2, RED);
            DrawText("[Saloon]",  2386, 1820, 10, RED);
            DrawRectangleLinesEx((Rectangle){ 2903, 1208, 80, 60 }, 2, BLUE);
            DrawText("[Stables]", 2906, 1216, 10, BLUE);
            DrawRectangleLinesEx((Rectangle){ 3499, 1502, 80, 60 }, 2, GREEN);
            DrawText("[Range]",   3502, 1510, 10, GREEN);
        } else {
            // Exit zone marker – position differs per sub-scene
            Rectangle exitRect = { 0 };
            if (currentScene == SCENE_SALOON)         exitRect = (Rectangle){  71, 523, 80, 60 };
            else if (currentScene == SCENE_STABLES)   exitRect = (Rectangle){ 194, 647, 80, 60 };
            else if (currentScene == SCENE_SHOOTING_RANGE) exitRect = (Rectangle){ 65, 554, 80, 60 };
            DrawRectangleLinesEx(exitRect, 2, ORANGE);
            DrawText("[Exit]", (int)exitRect.x + 5, (int)exitRect.y + 22, 10, ORANGE);
        }

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

        // Top-Left UI – context-aware
        DrawRectangle(10, 10, 350, 50, Fade(BLACK, 0.7f));
        if (currentScene == SCENE_MAIN_TOWN) {
            DrawText("Click an NPC to talk to them!", 20, 15, 18, RAYWHITE);
            if (player.heldItem) {
                DrawText(TextFormat("Holding: %s", player.heldItem), 20, 35, 16, GOLD);
            } else {
                DrawText("Holding: Nothing", 20, 35, 16, LIGHTGRAY);
            }
        } else {
            const char *sceneName = (currentScene == SCENE_SALOON)         ? "The Saloon" :
                                    (currentScene == SCENE_STABLES)        ? "Stables"    :
                                    (currentScene == SCENE_SHOOTING_RANGE) ? "Practice Range" : "?";
            DrawText(TextFormat("Location: %s", sceneName), 20, 15, 18, YELLOW);
            DrawText("Walk to the orange [Exit] to leave", 20, 35, 14, LIGHTGRAY);
        }

        // Mouse world-coordinate tracker (bottom-right corner)
        DrawRectangle(SCREEN_WIDTH - 220, SCREEN_HEIGHT - 34, 210, 24, Fade(BLACK, 0.7f));
        DrawText(TextFormat("World: %.0f, %.0f", mouseWorldPos.x, mouseWorldPos.y),
                 SCREEN_WIDTH - 215, SCREEN_HEIGHT - 29, 16, LIME);

        EndDrawing();

        // AI call happens after drawing so the "Hmm..." frame renders first
        if (isWaitingForAI && interactingNPC != NULL) {
            GenerateGeminiDialog(interactingNPC->name, interactingNPC->questItem,
                                 activeDialogText, sizeof(activeDialogText));
            WrapText(activeDialogText, 560, 20);
            isWaitingForAI = false;
            interactingNPC = NULL;
        }
    }

    UnloadNPC(&sheriff);
    UnloadNPC(&garry);
    UnloadNPC(&dale);
    UnloadNPC(&susan);
    UnloadNPC(&kitty);
    UnloadNPC(&buster);
    UnloadNPC(&tommy);
    UnloadNPC(&barry);
    UnloadNPC(&marley);
    UnloadCharacter(&player);
    UnloadCurrentSceneTextures(); // scene manager owns the background
    if (collisionMaskPixels != NULL) {
        UnloadImageColors(collisionMaskPixels);
    }
    if (collisionMaskImage.data != NULL) {
        UnloadImage(collisionMaskImage);
    }
    CloseWindow();

    return 0;
}
