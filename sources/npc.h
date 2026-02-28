#ifndef NPC_H
#define NPC_H

#include "raylib.h"
#include "scene_manager.h"

typedef struct NPC {
    Vector2 position;
    Texture2D texture;
    const char* name;
    Rectangle bounds;    // The clickable area around the NPC
    const char *questItem;
    bool questCompleted;
    SceneType scene;     // Which scene this NPC lives in
} NPC;

void InitNPC(NPC *npc, Vector2 pos, const char* name, const char* texturePath);
void DrawNPC(NPC *npc);
bool IsNPCClicked(NPC *npc, Vector2 mouseWorldPos);
void UnloadNPC(NPC *npc);

#endif
