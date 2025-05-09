#include "ecs.h"
#include "fmt/core.h"
#include "raylib.h"
#include "scenes.h"

static SceneEvent s_Event = SceneEvent::NONE;
static struct GameState {
  // TODO: add ...
} s_State;

void LoadGame() {
  float screen_cw = GetScreenWidth() / 2.f;
  float screen_ch = GetScreenHeight() / 2.f;

  fmt::println("GAME LOADED!");
}

void UpdateGame(float delta) {
  ECS::UISystem();
  ECS::PositionSystem();

  // if (IsKeyPressed(KEY_DOWN)) {
  // } else if (IsKeyPressed(KEY_UP)) {
  // } else if (IsKeyPressed(KEY_ENTER)) {
  //   return;
  // }
}

void DrawGame() { ECS::RenderSystem(); }

void UnloadGame() {}

SceneEvent OnGameEvent() { return s_Event; }
