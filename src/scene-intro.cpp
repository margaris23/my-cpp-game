#include "ecs.hpp"
#include "raylib.h"
#include "scenes.hpp"
#include <fmt/core.h>
#include <fmt/format.h>

static int s_counter;
static SceneEvent s_Event = SceneEvent::NONE;

static ECS::Entity s_Title;
// static ECS::Entity fps;

void LoadIntro() {
  float screen_cw = GetScreenWidth() / 2.f;
  float screen_ch = GetScreenHeight() / 2.f;

  s_Title = ECS::CreateEntity();
  ECS::Add<ECS::TextComponent>(s_Title, "Minoids");
  ECS::Add<ECS::PositionComponent>(
      s_Title, (float)(GetScreenWidth() - MeasureText("Minoids", 20)) / 2.f,
      (float)GetScreenHeight() / 2.f);
}

void UpdateIntro(float delta) {
  // Order of Systems
  // 1. Input System
  // 2. AI System
  // 3. Physics
  // ...
  // N-1. Render
  // N. Audio

  // auto start = std::chrono::high_resolution_clock::now();

  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_SPACE) ||
      IsKeyPressed(KEY_ENTER)) {
    s_Event = SceneEvent::NEXT;
    return;
  }

  // Input System - TESTING
  // if (IsKeyDown(KEY_RIGHT)) {
  //   ECS::Add<ECS::ForceComponent>(s_Title, 5.f, 0.f);
  // } else if (IsKeyDown(KEY_LEFT)) {
  //   ECS::Add<ECS::ForceComponent>(s_Title, -5.f, 0.f);
  // } else if (IsKeyDown(KEY_UP)) {
  //   ECS::Add<ECS::ForceComponent>(s_Title, 0.f, -5.f);
  // } else if (IsKeyDown(KEY_DOWN)) {
  //   ECS::Add<ECS::ForceComponent>(s_Title, 0.f, 5.f);
  // } else {
  //   ECS::Remove<ECS::ForceComponent>(s_Title);
  // }

  ECS::PositionSystem();
}

void DrawIntro() {
  ECS::RenderSystem();
  // ECS::ResetSystem();
}

void UnloadIntro() { ECS::DeleteEntity(s_Title); }

SceneEvent OnIntroEvent() { return s_Event; }
