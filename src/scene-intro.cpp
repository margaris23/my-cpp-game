#include "ecs.hpp"
#include "raylib.h"
#include "scenes.hpp"
#include <fmt/core.h>
#include <fmt/format.h>
#include <memory>

static int s_counter;
static SceneEvent s_Event = SceneEvent::NONE;

static std::unique_ptr<ECS::Registry> s_Registry;

using ECS::PositionComponent, ECS::SpriteComponent, ECS::Layer, ECS::Entity;

void LoadIntro() {
  s_Registry = std::make_unique<ECS::Registry>();

  float posX = (float)GetScreenWidth() / 2.f - 218.f;
  float posY = (float)GetScreenHeight() / 2.f - 46.f;
  auto title = s_Registry->CreateEntity();
  s_Registry->Add<SpriteComponent>(title, Layer::SKY, "minoids_logo.png");
  s_Registry->Add<PositionComponent>(title, posX, posY);
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

void SetIntroFocus() { s_Event = SceneEvent::NONE; }

SceneEvent OnIntroEvent() { return s_Event; }
