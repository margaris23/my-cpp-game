#include "ecs.hpp"
#include "raylib.h"
#include "scenes.hpp"
#include <memory>

static int s_counter;
static SceneEvent s_Event = SceneEvent::NONE;

static std::unique_ptr<ECS::Registry> s_Registry;

using ECS::PositionComponent, ECS::TextComponent, ECS::Entity;

void LoadNextRound() {
  s_Registry = std::make_unique<ECS::Registry>();

  float posX = (float)(GetScreenWidth() - 60.f) / 2.f;
  float posY = (float)GetScreenHeight() / 2.f;
  auto Title = s_Registry->CreateEntity();
  s_Registry->Add<TextComponent>(Title, "You WIN!");
  s_Registry->Add<PositionComponent>(Title, posX, posY);

  auto message = s_Registry->CreateEntity();
  s_Registry->Add<TextComponent>(message, "Press SPACE or ENTER to Continue");
  s_Registry->Add<PositionComponent>(message, posX - 170.f, posY + 50.f);
}

void UpdateNextRound(float delta) {
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_SPACE) ||
      IsKeyPressed(KEY_ENTER)) {
    s_Event = SceneEvent::NEXT;
    return;
  }

  s_Registry->PositionSystem();
}

void DrawNextRound() { s_Registry->RenderSystem(); }

void UnloadNextRound() {
  s_Registry.reset();
  s_Event = SceneEvent::NONE;
}

SceneEvent OnNextRoundEvent() { return s_Event; }
