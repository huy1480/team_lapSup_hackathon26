#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "raylib.h"
#include "character.h"

// Define the different locations in your town
typedef enum {
    SCENE_MAIN_TOWN,
    SCENE_SALOON,
    SCENE_STABLES,
    SCENE_SHOOTING_RANGE
} SceneType;

// Defines an area that causes a scene change
typedef struct {
    Rectangle bounds;      // Area in the current scene to walk into
    SceneType targetScene; // Scene to load
    Vector2 spawnPos;      // Where the player appears in new scene
} TeleportZone;

// Exposed state – readable anywhere that includes this header
extern SceneType currentScene;
extern Texture2D currentBackground;

// Function declarations
void InitScenes(void);
void UnloadCurrentSceneTextures(void);
void LoadScene(SceneType scene);
bool CheckForSceneSwitch(Vector2 playerPos, Character *player);

#endif