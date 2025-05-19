#include "fmt/core.h"
#include "raylib.h"
#include "scenes.hpp"
#include <iostream>
#include <ostream>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static constexpr int SCREEN_WIDTH = 800;
static constexpr int SCREEN_HEIGHT = 450;

static void UpdateDrawFrame();
static void HandleSceneEvent();
static void LoadScene(Scene scene);
static void UpdateCurrentScene(float delta);
static void DrawCurrentScene();
static void UnloadCurrentScene();

Scene g_currentScene = Scene::NONE;
static bool s_AppShouldExit = false;
static bool s_OverlayMenu = false;

int main(void) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "My Game");
  SetExitKey(KEY_NULL); // disable Esc key
  InitAudioDevice();

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
  SetTargetFPS(60);

  LoadScene(Scene::INTRO);

  // Main game loop
  while (!WindowShouldClose() && !s_AppShouldExit) {
    UpdateDrawFrame();
  }
#endif

  UnloadCurrentScene();
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
  // int posX = 10, posY = GetScreenHeight() - 20, i = 0;
  // DrawText("EID:", posX, posY, 20, RED);
  // for (const auto &entity : ECS::entities) {
  //   DrawText(TextFormat("%i", entity), posX + 50 + i++ * 30, posY, 20, RED);
  // }
  //
  // i = 0;
  // DrawText("POS:", posX, posY - 20, 20, RED);
  // for (const auto &pos : ECS::positions.dense) {
  //   DrawText(TextFormat("%i", pos.m_entity), posX + 50 + i++ * 30, posY - 20, 20, RED);
  // }
  // DrawFPS(GetScreenWidth() - 80, GetScreenHeight() - 30);

  EndDrawing();
}

static void UnloadCurrentScene() {
  switch (g_currentScene) {
  case Scene::INTRO:
    UnloadIntro();
    break;
  case Scene::GAME:
    UnloadGame();
    break;
  default:
    break;
  }
  g_currentScene = Scene::NONE;
}

static void LoadScene(Scene scene) {
  if (scene != g_currentScene) {
    UnloadCurrentScene();

    g_currentScene = scene;

    switch (scene) {
    case Scene::INTRO:
      LoadIntro();
      break;
    case Scene::GAME:
      LoadGame();
      break;
    default:
      break;
    }

  } else {
    std::cerr << "Scene already loaded!\n";
  }
}

static void UpdateCurrentScene(float delta) {
  switch (g_currentScene) {
  case Scene::INTRO:
    UpdateIntro(delta);
    break;
  case Scene::GAME:
    UpdateGame(delta);
    break;
  default:
    break;
  }

  if (s_OverlayMenu) {
    UpdateMenu(delta);
  }
}

static void DrawCurrentScene() {
  switch (g_currentScene) {
  case Scene::INTRO:
    DrawIntro();
    break;
  case Scene::GAME:
    DrawGame();
    break;
  default:
    break;
  }

  if (s_OverlayMenu) {
    DrawMenu();
  }
}

static void HandleSceneEvent() {
  // Handle Overlay in focus
  if (s_OverlayMenu) {
    SceneEvent event = OnMenuEvent();
    if (event == SceneEvent::EXIT) {
      UnloadMenu();
      s_OverlayMenu = false;
      UnloadCurrentScene();
      s_AppShouldExit = true;
    } else if (event == SceneEvent::CONTINUE) {
      UnloadMenu();
      s_OverlayMenu = false;
      if (Scene::GAME == g_currentScene) {
        SetGameFocus(true);
      } else if (Scene::INTRO == g_currentScene) {
        SetIntroFocus();
      }
    } else if (event == SceneEvent::NEW_GAME) {
      UnloadMenu();
      s_OverlayMenu = false;
      UnloadCurrentScene();
      LoadScene(Scene::GAME);
    }
    return;
  }

  // Rest of Scenes
  switch (g_currentScene) {
  case Scene::INTRO: {
    SceneEvent event = OnIntroEvent();
    if (event == SceneEvent::PAUSE) {
      if (IsMenuUnloaded()) {
        LoadMenu({{"New Game", SceneEvent::NEW_GAME},
                  {"Settings", SceneEvent::SETTINGS},
                  {"Help", SceneEvent::HELP},
                  {"Exit", SceneEvent::EXIT}});
        s_OverlayMenu = true;
      }
    }
  } break;
  case Scene::GAME: {
    SceneEvent event = OnGameEvent();
    if (event == SceneEvent::EXIT) {
      s_AppShouldExit = true;
    } else if (SceneEvent::PAUSE == event) {
      if (IsMenuUnloaded()) {
        LoadMenu({{"Continue", SceneEvent::CONTINUE},
                  {"Restart", SceneEvent::RESTART},
                  {"Exit", SceneEvent::EXIT}});
        s_OverlayMenu = true;
        SetGameFocus(false);
      }
    }
  } break;
  default:
    break;
  }
}
