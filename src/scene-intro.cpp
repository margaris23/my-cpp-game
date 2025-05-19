#include "ecs.hpp"
#include "raylib.h"
#include "scenes.hpp"
#include <fmt/core.h>
#include <fmt/format.h>
#include <memory>

static int s_counter;
static SceneEvent s_Event = SceneEvent::NONE;

static std::unique_ptr<ECS::Registry> s_Registry;

using ECS::PositionComponent, ECS::TextComponent, ECS::Entity;

void LoadIntro() {
  s_Registry = std::make_unique<ECS::Registry>();

  float posX = (float)(GetScreenWidth() - MeasureText("Minoids", 20)) / 2.f;
  float posY = (float)GetScreenHeight() / 2.f;
  auto s_Title = s_Registry->CreateEntity();
  s_Registry->Add<TextComponent>(s_Title, "Minoids");
  s_Registry->Add<PositionComponent>(s_Title, posX, posY);
}

void UpdateIntro(float delta) {
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_SPACE) ||
      IsKeyPressed(KEY_ENTER)) {
    s_Event = SceneEvent::PAUSE;
    return;
  }

  s_Registry->PositionSystem();
}

void DrawIntro() { s_Registry->RenderSystem(); }

void UnloadIntro() {
  s_Registry.reset();
  s_Event = SceneEvent::NONE;
}

void SetIntroFocus() {
  s_Event = SceneEvent::NONE;
}

SceneEvent OnIntroEvent() { return s_Event; }
