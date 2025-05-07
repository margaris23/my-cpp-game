#include "ecs.h"
#include "fmt/core.h"
#include "raylib.h"
#include "scenes.h"
#include <iostream>
#include <ostream>
#include <string>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static const int screenWidth = 800;
static const int screenHeight = 450;

static void UpdateDrawFrame();
static void HandleSceneEvent();

Scene g_currentScene = Scene::NONE;
static bool s_AppShouldExit = false;

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
  while (!WindowShouldClose() && !s_AppShouldExit) {
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

  // SCENE EVENT PHASE
  HandleSceneEvent();

  // UPDATE PHASE
  UpdateCurrentScene(delta);

  // DRAW PHASE
  BeginDrawing();

  ClearBackground(RAYWHITE);
  DrawCurrentScene();

  // DEBUG PRINT ECS STATE
  int posX = 10, posY = GetScreenHeight() - 20, i = 0;
  for (const auto &entity : ECS::entities) {
    DrawText(TextFormat("%i", entity), posX + i++ * 30, posY, 20, RED);
  }

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
      LoadMenu();
      break;
    case Scene::GAME:
      break;
    default:
      break;
    }

    g_currentScene = scene;
    fmt::println("NEW SCENE: {}", static_cast<int>(g_currentScene));
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
    UpdateMenu(delta);
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
    DrawMenu();
    break;
  case Scene::GAME:
    break;
  default:
    break;
  }
}

static void HandleSceneEvent() {
  switch (g_currentScene) {
  case Scene::INTRO: {
    SceneEvent event = OnIntroEvent();
    if (event == SceneEvent::NEXT) {
      LoadScene(Scene::MENU);
    }
  } break;
  case Scene::MENU: {
    SceneEvent event = OnMenuEvent();
    if (event == SceneEvent::EXIT) {
      UnloadMenu();
      s_AppShouldExit = true;
    }
  } break;
  case Scene::GAME:
    break;
  default:
    break;
  }
}
