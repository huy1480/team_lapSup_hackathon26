#include "raylib.h"
#include "character.h"
#include "npc.h" // Include our new NPCs!
#include <stddef.h>

#define SCREEN_WIDTH (1024)
#define SCREEN_HEIGHT (576)
#define WINDOW_TITLE "Lego Western Town - NPCs & Dialog"

// --- Global UI State ---
bool isDialogOpen = false;
const char* activeDialogText = "";

static bool IsPointWalkableFromMask(const Image *collisionMask, Vector2 worldPos) {
    if (collisionMask == NULL || collisionMask->data == NULL) return true;

    if (worldPos.x < 0.0f || worldPos.y < 0.0f || worldPos.x >= SCREEN_WIDTH || worldPos.y >= SCREEN_HEIGHT) {
        return false;
    }

    int maskX = (int)(worldPos.x * ((float)collisionMask->width / (float)SCREEN_WIDTH));
    int maskY = (int)(worldPos.y * ((float)collisionMask->height / (float)SCREEN_HEIGHT));

    if (maskX < 0 || maskY < 0 || maskX >= collisionMask->width || maskY >= collisionMask->height) {
        return false;
    }

    Color pixel = GetImageColor(*collisionMask, maskX, maskY);
    int brightness = (pixel.r + pixel.g + pixel.b) / 3;
    return brightness >= 200;
}

static bool IsCharacterPositionWalkable(const Image *collisionMask, const Character *player, Vector2 position) {
    float collisionRadius = 6.0f;
    if (player->texture.id != 0) {
        float minDim = (player->texture.width < player->texture.height) ? (float)player->texture.width : (float)player->texture.height;
        collisionRadius = minDim * 0.25f;
    }

    if (collisionRadius < 2.0f) collisionRadius = 2.0f;

    return IsPointWalkableFromMask(collisionMask, position) &&
           IsPointWalkableFromMask(collisionMask, (Vector2){ position.x + collisionRadius, position.y }) &&
           IsPointWalkableFromMask(collisionMask, (Vector2){ position.x - collisionRadius, position.y }) &&
           IsPointWalkableFromMask(collisionMask, (Vector2){ position.x, position.y + collisionRadius }) &&
           IsPointWalkableFromMask(collisionMask, (Vector2){ position.x, position.y - collisionRadius });
}

// The function you requested!
void OpenNPCDialog(const char* npcName) {
    activeDialogText = npcName;
    isDialogOpen = true;
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);

    Texture2D background = LoadTexture("assets/bg.png");
    Image collisionMask = LoadImage("assets/collision.png");
    bool hasCollisionMask = collisionMask.data != NULL;

    Character player;
    InitCharacter(&player, (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f }, "assets/character.png");

    // Initialize our NPCs
    NPC sheriff;
    InitNPC(&sheriff, (Vector2){ 500.0f, 300.0f }, "Sheriff Burbrick", "assets/sheriff.png");
    
    NPC garry;
    InitNPC(&garry, (Vector2){ 800.0f, 500.0f }, "Gunslinger Garry", "assets/gary.png");

    Camera2D camera = { 0 };
    camera.zoom = 3.0f; 
    camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    camera.target = player.position;

    while (!WindowShouldClose())
    {
        Vector2 mouseScreenPos = GetMousePosition();
        Vector2 mouseWorldPos = GetScreenToWorld2D(mouseScreenPos, camera);

        // --- Input Logic ---
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            
            // 1. If Dialog is open, we ONLY check for UI button clicks
            if (isDialogOpen) {
                // We define the button rectangle in screen space
                Rectangle okBtn = { SCREEN_WIDTH / 2.0f - 50, SCREEN_HEIGHT / 2.0f + 30, 100, 40 };
                
                // Did we click inside the "Okay" button?
                if (CheckCollisionPointRec(mouseScreenPos, okBtn)) {
                    isDialogOpen = false; // Close the dialog
                }
            } 
            // 2. If Dialog is closed, we check for World interactions
            else {
                if (IsNPCClicked(&sheriff, mouseWorldPos)) {
                    OpenNPCDialog(sheriff.name);
                } 
                else if (IsNPCClicked(&garry, mouseWorldPos)) {
                    OpenNPCDialog(garry.name);
                } 
                else {
                    // Only move the player if we didn't click on an NPC
                    if (IsCharacterPositionWalkable(hasCollisionMask ? &collisionMask : NULL, &player, mouseWorldPos)) {
                        player.targetPosition = mouseWorldPos;
                    }
                }
            }
        }

        // --- Update Logic ---
        // Don't update the player's movement if they are locked in a conversation
        if (!isDialogOpen) {
            Vector2 previousPosition = player.position;
            UpdateCharacter(&player);

            if (!IsCharacterPositionWalkable(hasCollisionMask ? &collisionMask : NULL, &player, player.position)) {
                player.position = previousPosition;
                player.targetPosition = previousPosition;
            }
        }

        // --- Camera Logic ---
        Vector2 desiredCameraTarget = player.position;
        float minX = (SCREEN_WIDTH / 2.0f) / camera.zoom;
        float maxX = SCREEN_WIDTH - minX;
        float minY = (SCREEN_HEIGHT / 2.0f) / camera.zoom;
        float maxY = SCREEN_HEIGHT - minY;

        if (desiredCameraTarget.x < minX) desiredCameraTarget.x = minX;
        if (desiredCameraTarget.x > maxX) desiredCameraTarget.x = maxX;
        if (desiredCameraTarget.y < minY) desiredCameraTarget.y = minY;
        if (desiredCameraTarget.y > maxY) desiredCameraTarget.y = maxY;

        camera.target.x += (desiredCameraTarget.x - camera.target.x) * 0.05f;
        camera.target.y += (desiredCameraTarget.y - camera.target.y) * 0.05f;

        // --- Drawing ---
        BeginDrawing();
        ClearBackground(DARKGRAY);

        // -- WORLD SPACE DRAWING --
        BeginMode2D(camera);

        if (background.id != 0) {
            Rectangle sourceRec = { 0.0f, 0.0f, (float)background.width, (float)background.height };
            Rectangle destRec = { 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
            DrawTexturePro(background, sourceRec, destRec, (Vector2){0,0}, 0.0f, WHITE);
        }

        // Draw NPCs and Player
        DrawNPC(&sheriff);
        DrawNPC(&garry);
        DrawCharacter(&player);

        EndMode2D();

        // -- SCREEN SPACE DRAWING (UI) --
        
        // Render the Dialog Box on top of everything if it is open
        if (isDialogOpen) {
            // Semi-transparent dark overlay to focus attention on the dialog
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.6f));

            // Main Dialog Box
            Rectangle dialogRec = { SCREEN_WIDTH / 2.0f - 200, SCREEN_HEIGHT / 2.0f - 100, 400, 200 };
            DrawRectangleRec(dialogRec, RAYWHITE);
            DrawRectangleLinesEx(dialogRec, 4, DARKGRAY);

            // Dialog Text
            const char* message = TextFormat("Howdy! My name is %s.", activeDialogText);
            DrawText(message, (int)dialogRec.x + 20, (int)dialogRec.y + 40, 20, BLACK);

            // Okay Button Logic & Rendering
            Rectangle okBtn = { SCREEN_WIDTH / 2.0f - 50, SCREEN_HEIGHT / 2.0f + 30, 100, 40 };
            
            // Highlight button if mouse is hovering over it
            bool isHovering = CheckCollisionPointRec(mouseScreenPos, okBtn);
            DrawRectangleRec(okBtn, isHovering ? LIGHTGRAY : GRAY);
            DrawRectangleLinesEx(okBtn, 2, BLACK);
            
            // Draw button text centered
            DrawText("Okay!", (int)okBtn.x + 25, (int)okBtn.y + 10, 20, BLACK);
        }

        // Standard Top-Left UI 
        DrawRectangle(10, 10, 310, 30, Fade(BLACK, 0.7f));
        DrawText("Click an NPC to talk to them!", 20, 15, 18, RAYWHITE);

        EndDrawing();
    }

    // Unload assets
    UnloadNPC(&sheriff);
    UnloadNPC(&garry);
    UnloadCharacter(&player);
    if (hasCollisionMask) {
        UnloadImage(collisionMask);
    }
    UnloadTexture(background); 
    CloseWindow();

    return 0;
}
