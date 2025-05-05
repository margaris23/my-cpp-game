#include "raylib.h"
#include "scenes.h"
#include <iostream>
#include <ostream>
#include "ecs.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static const int screenWidth = 800;
static const int screenHeight = 450;

static void UpdateDrawFrame(void);

Scene g_currentScene = Scene::NONE;

static void LoadScene(Scene scene);
static void UpdateCurrentScene(float delta);
static void DrawCurrentScene();

int main(void) {
  InitWindow(screenWidth, screenHeight, "My Game");
  InitAudioDevice();

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
  SetTargetFPS(60);

  ECS::Init();
  LoadScene(Scene::INTRO);

  // Main game loop
  while (!WindowShouldClose()) {
    UpdateDrawFrame();
  }
#endif

  CloseAudioDevice();
  CloseWindow();

  return 0;
}

// Update and draw game frame
static void UpdateDrawFrame(void) {
  float delta = GetFrameTime();

  // UPDATE PHASE
  UpdateCurrentScene(delta);

  // DRAW PHASE
  BeginDrawing();

  ClearBackground(RAYWHITE);
  DrawCurrentScene();

  EndDrawing();
}

static void LoadScene(Scene scene) {
  if (scene != g_currentScene) {
    // UNLOAD
    switch (g_currentScene) {
    case Scene::INTRO:
      UnloadIntro();
      break;
    case Scene::MENU:
      break;
    case Scene::GAME:
      break;
    default:
      break;
    }

    // LOAD
    switch (scene) {
    case Scene::INTRO:
      LoadIntro();
      break;
    case Scene::MENU:
      break;
    case Scene::GAME:
      break;
    default:
      break;
    }

    g_currentScene = scene;
  } else {
    std::cerr << "Scene already loaded!\n";
  }
}

static void UpdateCurrentScene(float delta) {
  switch (g_currentScene) {
  case Scene::INTRO:
    UpdateIntro(delta);
    break;
  case Scene::MENU:
    break;
  case Scene::GAME:
    break;
  default:
    break;
  }
}

static void DrawCurrentScene() {
  switch (g_currentScene) {
  case Scene::INTRO:
    DrawIntro();
    break;
  case Scene::MENU:
    break;
  case Scene::GAME:
    break;
  default:
    break;
  }
}
