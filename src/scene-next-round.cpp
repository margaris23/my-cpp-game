#include "ecs.hpp"
#include "game.hpp"
#include "raylib.h"
#include "scenes.hpp"
#include <memory>

static int s_counter;
static SceneEvent s_Event = SceneEvent::NONE;

static std::unique_ptr<ECS::Registry> s_Registry;

using ECS::PositionComponent, ECS::TextComponent, ECS::Entity;

extern Game::Game g_Game;

void LoadNextRound() {
  s_Registry = std::make_unique<ECS::Registry>();

  float posX = (float)(GetScreenWidth() - 90.f) / 2.f;
  float posY = (float)GetScreenHeight() / 2.f - 50.f;

  auto status = s_Registry->CreateEntity();
  s_Registry->Add<PositionComponent>(status, posX, posY);

  auto message = s_Registry->CreateEntity();
  s_Registry->Add<PositionComponent>(message, posX - 140.f, posY + 50.f);

  if (Game::IsGameWon()) {
    s_Registry->Add<TextComponent>(status, "YOU WIN!");
    s_Registry->Add<TextComponent>(message, "Press SPACE or ENTER to Continue");

  } else {
    s_Registry->Add<TextComponent>(status, "YOU LOSE!");
    s_Registry->Add<TextComponent>(message, "Press SPACE or ENTER to Replay");
  }
}

void UpdateNextRound(float delta) {
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_SPACE) ||
      IsKeyPressed(KEY_ENTER)) {

    if (Game::IsGameWon()) {
      Game::NextLevel();
    } else {
      Game::LoadLevel(g_Game.level);
    }
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
