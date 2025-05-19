#include "ecs.hpp"
#include "raylib.h"
#include "scenes.hpp"
#include <fmt/core.h>
#include <fmt/format.h>
#include <memory>

static int s_counter;
static SceneEvent s_Event = SceneEvent::NONE;

static std::unique_ptr<ECS::Registry> s_Registry;

void LoadIntro() {
  float screen_cw = GetScreenWidth() / 2.f;
  float screen_ch = GetScreenHeight() / 2.f;

  s_Registry = std::make_unique<ECS::Registry>();

  ECS::Entity s_Title = s_Registry->CreateEntity();
  s_Registry->Add<ECS::TextComponent>(s_Title, "Minoids");
  s_Registry->Add<ECS::PositionComponent>(
      s_Title, (float)(GetScreenWidth() - MeasureText("Minoids", 20)) / 2.f,
      (float)GetScreenHeight() / 2.f);
}

void UpdateIntro(float delta) {
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

  s_Registry->PositionSystem();
}

void DrawIntro() {
  s_Registry->RenderSystem();
  // ECS::ResetSystem();
}

void UnloadIntro() {
  s_Registry.reset();
  s_Event = SceneEvent::NONE;
}

SceneEvent OnIntroEvent() { return s_Event; }
