#include "npc.h"
#include <stddef.h>

void InitNPC(NPC *npc, Vector2 pos, const char* name, const char* texturePath) {
    npc->position = pos;
    npc->name = name;
    npc->texture = LoadTexture(texturePath);

    npc->questItem = NULL;
    npc->questCompleted = false;   

    // If texture fails to load, use a 32x32 placeholder size
    float width = npc->texture.id != 0 ? (float)npc->texture.width : 32.0f;
    float height = npc->texture.id != 0 ? (float)npc->texture.height : 32.0f;
    
    // Center the clickable bounds around the NPC's coordinate
    npc->bounds = (Rectangle){ pos.x - width / 2.0f, pos.y - height / 2.0f, width, height };
}

void DrawNPC(NPC *npc) {
    if (npc->texture.id != 0) {
        Vector2 origin = { npc->texture.width / 2.0f, npc->texture.height / 2.0f };
        Rectangle source = { 0, 0, (float)npc->texture.width, (float)npc->texture.height };
        Rectangle dest = { npc->position.x, npc->position.y, (float)npc->texture.width, (float)npc->texture.height };
        DrawTexturePro(npc->texture, source, dest, origin, 0.0f, WHITE);
    } else {
        // Fallback: draw a blue square if the texture is missing
        DrawRectangleRec(npc->bounds, BLUE);
    }
    
    // Draw their name above their head for easy identification!
    int textWidth = MeasureText(npc->name, 10);
    DrawText(npc->name, (int)(npc->position.x - textWidth / 2.0f), (int)(npc->position.y - npc->bounds.height), 10, RAYWHITE);
}

// Checks if the world-space mouse coordinate is inside the NPC's rectangle
bool IsNPCClicked(NPC *npc, Vector2 mouseWorldPos) {
    return CheckCollisionPointRec(mouseWorldPos, npc->bounds);
}

void UnloadNPC(NPC *npc) {
    UnloadTexture(npc->texture);
}
