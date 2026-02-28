#include "scene_manager.h"
#include <stdio.h>

// -------------------------------------------------------------------------
// Global state – exposed via extern in scene_manager.h
// -------------------------------------------------------------------------
SceneType currentScene = SCENE_MAIN_TOWN;
Texture2D currentBackground = { 0 };

// Cooldown: seconds to ignore teleport zones after a scene switch
static float teleportCooldown = 2.0f;
#define TELEPORT_COOLDOWN_SECS 2.2f

// -------------------------------------------------------------------------
//  TELEPORT ZONES – MAIN TOWN
//  Each zone is a Rectangle on bg.png that the player walks into.
//  *** Adjust x/y/w/h to match the actual building positions on your map! ***
//
//  Layout assumptions (bg.png ~1024x576 world-space):
//    Saloon / Bar   – right side of town, around x=660
//    Stables        – left side of town,  around x=60
//    Practice Range – bottom-right,       around x=750
//
//  spawnPos = where the player re-appears INSIDE that sub-scene
// -------------------------------------------------------------------------
#define TOWN_ZONE_COUNT 3
static TeleportZone townZones[TOWN_ZONE_COUNT] = {
    // { bounds {x,y,w,h},              target scene,          spawn in sub-scene }
    { { 2383, 1812, 80, 60 }, SCENE_SALOON,         { 200, 300 } },  // Tavern  @ 2423,1842
    { { 2903, 1208, 80, 60 }, SCENE_STABLES,        { 200, 300 } },  // Stables @ 2943,1238
    { { 3499, 1502, 80, 60 }, SCENE_SHOOTING_RANGE, { 200, 300 } },  // Range   @ 3539,1532
};

// -------------------------------------------------------------------------
//  EXIT ZONES – each sub-scene
//  Placed at the bottom-left of the flat sub-scene map so the player can
//  walk out.  spawnPos = where the player re-appears in the MAIN TOWN.
// -------------------------------------------------------------------------
#define EXIT_ZONE_COUNT 1
static TeleportZone saloonExitZones[EXIT_ZONE_COUNT] = {
    { {  71, 523, 80, 60 }, SCENE_MAIN_TOWN, { 2423, 1842 } },  // Tavern exit @ 111,553
};
static TeleportZone stablesExitZones[EXIT_ZONE_COUNT] = {
    { { 194, 647, 80, 60 }, SCENE_MAIN_TOWN, { 2943, 1238 } },  // Stables exit @ 234,677
};
static TeleportZone rangeExitZones[EXIT_ZONE_COUNT] = {
    { {  65, 554, 80, 60 }, SCENE_MAIN_TOWN, { 3539, 1532 } },  // Range exit @ 105,584
};

// -------------------------------------------------------------------------
void InitScenes(void) {
    LoadScene(SCENE_MAIN_TOWN);
}

void UnloadCurrentSceneTextures(void) {
    if (currentBackground.id != 0) {
        UnloadTexture(currentBackground);
        currentBackground.id = 0;
    }
}

void LoadScene(SceneType scene) {
    UnloadCurrentSceneTextures();
    currentScene = scene;
    teleportCooldown = TELEPORT_COOLDOWN_SECS; // start cooldown on every scene load

    switch (currentScene) {
        case SCENE_MAIN_TOWN:
            currentBackground = LoadTexture("assets/bg.png");
            break;
        case SCENE_SALOON:
            currentBackground = LoadTexture("assets/Scenes/the_bar.png");
            break;
        case SCENE_STABLES:
            currentBackground = LoadTexture("assets/Scenes/stables.png");
            break;
        case SCENE_SHOOTING_RANGE:
            currentBackground = LoadTexture("assets/Scenes/practice_rage.png");
            break;
        default: break;
    }
}

// -------------------------------------------------------------------------
//  CheckForSceneSwitch
//  Call every frame after UpdateCharacter().
//  Returns true when a teleport happened (caller can skip other update work).
// -------------------------------------------------------------------------
bool CheckForSceneSwitch(Vector2 playerPos, Character *player) {
    // Tick down the cooldown; don't check zones until it expires
    if (teleportCooldown > 0.0f) {
        teleportCooldown -= GetFrameTime();
        return false;
    }

    TeleportZone *zones = NULL;
    int count = 0;

    switch (currentScene) {
        case SCENE_MAIN_TOWN:      zones = townZones;       count = TOWN_ZONE_COUNT;  break;
        case SCENE_SALOON:         zones = saloonExitZones; count = EXIT_ZONE_COUNT;  break;
        case SCENE_STABLES:        zones = stablesExitZones;count = EXIT_ZONE_COUNT;  break;
        case SCENE_SHOOTING_RANGE: zones = rangeExitZones;  count = EXIT_ZONE_COUNT;  break;
        default: break;
    }

    for (int i = 0; i < count; i++) {
        if (CheckCollisionPointRec(playerPos, zones[i].bounds)) {
            LoadScene(zones[i].targetScene);
            player->position      = zones[i].spawnPos;
            player->targetPosition = zones[i].spawnPos;
            return true;
        }
    }
    return false;
}
