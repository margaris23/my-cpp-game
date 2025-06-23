#include "FastNoiseLite.h"
#include "game.hpp"
#include "raylib.h"
#include "scenes.hpp"
#include <cmath>
#include <iostream>
#include <ostream>
#include "resource_dir.h"

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

// static float RADIUS = 150.0f;
// static int POINT_COUNT = 91;
// static float NOISE_AMPLITUDE = 14.5f;
// static constexpr float NOISE_SCALE = 0.1f;

float PerlinNoise1D(float x);

FastNoiseLite noise;

int main(void) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "MINOIDS");
  SetExitKey(KEY_NULL); // disable Esc key
  SearchAndSetResourceDir("resources");
  InitAudioDevice();

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
  SetTargetFPS(60);

  // noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
  // noise.SetFrequency(NOISE_SCALE);

  Game::InitGame();
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

  // Vector2 center = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
  //
  // if (IsKeyDown(KEY_UP)) {
  //   NOISE_AMPLITUDE += 0.1f;
  // } else if (IsKeyDown(KEY_DOWN)) {
  //   NOISE_AMPLITUDE -= 0.1f;
  // }
  //
  // if (IsKeyPressed(KEY_LEFT)) {
  //   POINT_COUNT -= 1;
  // } else if (IsKeyPressed(KEY_RIGHT)) {
  //   POINT_COUNT += 1;
  // }
  //
  // if (IsKeyPressed(KEY_ESCAPE)) {
  //   s_AppShouldExit = true;
  // }
  //
  // DrawText(TextFormat("Points: %d", POINT_COUNT), 10, 10, 20, GREEN);
  // DrawText(TextFormat("Noise Amplitude: %f", NOISE_AMPLITUDE), 10, 30, 20, BLUE);
  //
  // Vector2 points[POINT_COUNT];
  //
  // for (int i = 0; i < POINT_COUNT; i++) {
  //   float angle = (2 * PI * i) / POINT_COUNT;
  //
  //   // Apply noise using angle as x-axis input
  //   float noiseValue = noise.GetNoise((float)i, 0.0f); // 1D noise
  //   float radius = RADIUS + noiseValue * NOISE_AMPLITUDE;
  //
  //   points[i].x = center.x + cosf(angle) * radius;
  //   points[i].y = center.y + sinf(angle) * radius;
  // }
  //
  // // Draw polygon
  // for (int i = 0; i < POINT_COUNT; i++) {
  //   DrawLineV(points[i], points[(i + 1) % POINT_COUNT], DARKBLUE);
  // }

  // Optional: Draw base circle for reference
  // DrawCircleLines((int)center.x, (int)center.y, RADIUS, LIGHTGRAY);

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

// Basic pseudo Perlin noise function using sine
float PerlinNoise1D(float x) { return 0.5f * (sinf(x) + sinf(x * 0.5f + 3.14f)); }

static void UnloadCurrentScene() {
  switch (g_currentScene) {
  case Scene::INTRO:
    UnloadIntro();
    break;
  case Scene::GAME:
    UnloadGame();
    break;
  case Scene::NEXT_ROUND:
    UnloadNextRound();
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
    case Scene::NEXT_ROUND:
      LoadNextRound();
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
  case Scene::NEXT_ROUND:
    UpdateNextRound(delta);
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
  case Scene::NEXT_ROUND:
    DrawNextRound();
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
    } else if (SceneEvent::NEXT == event) {
      LoadScene(Scene::NEXT_ROUND);
    }
  } break;

  case Scene::NEXT_ROUND: {
    SceneEvent event = OnNextRoundEvent();
    if (SceneEvent::NEXT == event) {
      LoadScene(Scene::GAME);
    }
  } break;

  default:
    break;
  }
}
