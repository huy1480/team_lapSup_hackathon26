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
char activeDialogText[1024] = ""; 
const char* activeNPCName = "";
bool isWaitingForAI = false;
NPC* interactingNPC = NULL;

// --- Python Hook Function ---
void GenerateGeminiDialog(const char* npcName, const char* itemName, char* buffer, size_t bufferSize) {
    char command[512];
    
    snprintf(command, sizeof(command), "%s gemini_dialog.py \"%s\" \"%s\"", PYTHON_CMD, npcName, itemName);

    FILE *fp = POPEN(command, "r");
    if (fp == NULL) {
        snprintf(buffer, bufferSize, "Error: Could not run Python script.");
        printf("ERROR: Python script failed to execute.\n"); 
        return;
    }

    buffer[0] = '\0'; 
    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        strncat(buffer, line, bufferSize - strlen(buffer) - 1);
    }
    
    PCLOSE(fp);

    printf("\n--- GEMINI DIALOG GENERATED ---\n");
    printf("NPC: %s\n", npcName);
    printf("Text: %s\n", buffer);
    printf("-------------------------------\n\n");
}

// --- Text Wrapping Helper ---
void WrapText(char *text, int maxLineWidth, int fontSize) {
    int length = strlen(text);
    int lineStart = 0;
    int lastSpace = -1;
    char temp[1024];

    for (int i = 0; i < length; i++) {
        if (text[i] == ' ') lastSpace = i;
        if (text[i] == '\n') { 
            lineStart = i + 1;
            continue;
        }

        int currentLength = i - lineStart + 1;
        if (currentLength >= sizeof(temp)) currentLength = sizeof(temp) - 1; 
        
        strncpy(temp, text + lineStart, currentLength);
        temp[currentLength] = '\0';

        if (MeasureText(temp, fontSize) > maxLineWidth) {
            if (lastSpace > lineStart) {
                text[lastSpace] = '\n';     
                lineStart = lastSpace + 1;  
                i = lineStart - 1;          
            } else {
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
        snprintf(activeDialogText, sizeof(activeDialogText), "Well I'll be! You found my %s. Thank ye kindly!", npc->questItem);
        npc->questCompleted = true;
        player->heldItem = NULL; 
    } 
    else {
        GenerateGeminiDialog(npc->name, npc->questItem, activeDialogText, sizeof(activeDialogText));
    }
    
    WrapText(activeDialogText, 560, 20); 
    isDialogOpen = true;
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);
    srand(time(NULL)); 

    Texture2D background = LoadTexture("assets/bg.png"); // Corrected to match your jpg file

    Character player;
    InitCharacter(&player, (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f }, "assets/character.png");

    // --- NPC Initialization ---
    NPC sheriff, garry, dale, susan, kitty, buster, tommy, barry, marley;

    InitNPC(&sheriff, (Vector2){ 500.0f, 300.0f }, "Sheriff Burbrick", "assets/sheriff.png");
    sheriff.questItem = "Lost Badge"; 
    
    InitNPC(&garry, (Vector2){ 800.0f, 500.0f }, "Gunslinger Gary", "assets/gary.png");
    garry.questItem = "Lucky Horseshoe";

    InitNPC(&dale, (Vector2){ 260.0f, 270.0f }, "Dynamite Dale", "assets/dale.png"); // Top left near mine
    dale.questItem = "TNT Plunger";

    InitNPC(&susan, (Vector2){ 800.0f, 480.0f }, "Stable Susan", "assets/susan.png"); // Right side near stables
    susan.questItem = "Golden Saddle";

    InitNPC(&kitty, (Vector2){ 500.0f, 450.0f }, "Kitty", "assets/kitty.png"); // Wandering the street
    kitty.questItem = "Feather Boa";

    InitNPC(&buster, (Vector2){ 860.0f, 680.0f }, "Buster the Bandit", "assets/buster.png"); // Near the shooting range targets
    buster.questItem = "Stolen Loot";

    InitNPC(&tommy, (Vector2){ 350.0f, 680.0f }, "Tommy Treasurer", "assets/tommy.png"); // Bottom left grey building
    tommy.questItem = "Ledger";

    InitNPC(&barry, (Vector2){ 640.0f, 620.0f }, "Barry the Barkeep", "assets/barry.png"); // Inside Saloon
    barry.questItem = "Special Whiskey";

    InitNPC(&marley, (Vector2){ 680.0f, 580.0f }, "Marley the Musician", "assets/marley.png"); // Near piano in Saloon
    marley.questItem = "Tuning Fork";

    // --- Initialize Random Items (Expanded to 9) ---
    Item items[9];
    items[0] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Lost Badge", true };
    items[1] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Lucky Horseshoe", true };
    items[2] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "TNT Plunger", true };
    items[3] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Golden Saddle", true };
    items[4] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Feather Boa", true };
    items[5] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Stolen Loot", true };
    items[6] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Ledger", true };
    items[7] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Special Whiskey", true };
    items[8] = (Item){ (Vector2){ rand() % 800 + 100, rand() % 400 + 100 }, "Tuning Fork", true };

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
            else {
                NPC* clickedNPC = NULL;
                if (IsNPCClicked(&sheriff, mouseWorldPos)) clickedNPC = &sheriff;
                else if (IsNPCClicked(&garry, mouseWorldPos)) clickedNPC = &garry;
                else if (IsNPCClicked(&dale, mouseWorldPos)) clickedNPC = &dale;
                else if (IsNPCClicked(&susan, mouseWorldPos)) clickedNPC = &susan;
                else if (IsNPCClicked(&kitty, mouseWorldPos)) clickedNPC = &kitty;
                else if (IsNPCClicked(&buster, mouseWorldPos)) clickedNPC = &buster;
                else if (IsNPCClicked(&tommy, mouseWorldPos)) clickedNPC = &tommy;
                else if (IsNPCClicked(&barry, mouseWorldPos)) clickedNPC = &barry;
                else if (IsNPCClicked(&marley, mouseWorldPos)) clickedNPC = &marley;

                if (clickedNPC != NULL) {
                    activeNPCName = clickedNPC->name;
                    isDialogOpen = true;

                    if (clickedNPC->questCompleted) {
                        snprintf(activeDialogText, sizeof(activeDialogText), "Much obliged for your help earlier, partner!");
                    } 
                    else if (player.heldItem != NULL && strcmp(player.heldItem, clickedNPC->questItem) == 0) {
                        snprintf(activeDialogText, sizeof(activeDialogText), "Well I'll be! You found my %s. Thank ye kindly!", clickedNPC->questItem);
                        clickedNPC->questCompleted = true;
                        player.heldItem = NULL; 
                    } 
                    else {
                        snprintf(activeDialogText, sizeof(activeDialogText), "Hmm...");
                        isWaitingForAI = true;        
                        interactingNPC = clickedNPC;  
                    }
                } 
                else {
                    player.targetPosition = mouseWorldPos;
                }
            }
        }

        // --- Update Logic ---
        if (!isDialogOpen) {
            UpdateCharacter(&player);

            for (int i = 0; i < 9; i++) {
                if (items[i].active && Vector2Distance(player.position, items[i].position) < 20.0f) {
                    items[i].active = false;
                    player.heldItem = items[i].name; 
                }
            }
        }

        camera.target = player.position;

        // --- Drawing ---
        BeginDrawing();
        ClearBackground(DARKGRAY);

        BeginMode2D(camera);

        if (background.id != 0) {
            DrawTextureEx(background, (Vector2){0,0}, 0.0f, 1.0f, WHITE);
        }

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
        
        DrawCharacter(&player);

        EndMode2D();

        // -- SCREEN SPACE DRAWING (UI) --
        if (isDialogOpen) {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.6f));

            Rectangle dialogRec = { SCREEN_WIDTH / 2.0f - 300, SCREEN_HEIGHT / 2.0f - 100, 600, 250 };
            DrawRectangleRec(dialogRec, RAYWHITE);
            DrawRectangleLinesEx(dialogRec, 4, DARKGRAY);

            DrawText(activeNPCName, (int)dialogRec.x + 20, (int)dialogRec.y + 15, 24, DARKBLUE);
            DrawLine((int)dialogRec.x + 20, (int)dialogRec.y + 45, (int)dialogRec.x + 580, (int)dialogRec.y + 45, GRAY);

            DrawText(activeDialogText, (int)dialogRec.x + 20, (int)dialogRec.y + 60, 20, BLACK);

            Rectangle okBtn = { SCREEN_WIDTH / 2.0f - 50, SCREEN_HEIGHT / 2.0f + 60, 100, 40 };
            bool isHovering = CheckCollisionPointRec(mouseScreenPos, okBtn);
            DrawRectangleRec(okBtn, isHovering ? LIGHTGRAY : GRAY);
            DrawRectangleLinesEx(okBtn, 2, BLACK);
            DrawText("Okay!", (int)okBtn.x + 25, (int)okBtn.y + 10, 20, BLACK);
        }

        DrawRectangle(10, 10, 310, 50, Fade(BLACK, 0.7f));
        DrawText("Click an NPC to talk to them!", 20, 15, 18, RAYWHITE);
        
        if (player.heldItem) {
            DrawText(TextFormat("Holding: %s", player.heldItem), 20, 35, 16, GOLD);
        } else {
            DrawText("Holding: Nothing", 20, 35, 16, LIGHTGRAY);
        }

        EndDrawing();

        if (isWaitingForAI && interactingNPC != NULL) {
            GenerateGeminiDialog(interactingNPC->name, interactingNPC->questItem, activeDialogText, sizeof(activeDialogText));
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
    UnloadTexture(background); 
    CloseWindow();

    return 0;
}
