#include "ecs.h"
#include "raylib.h"
#include "scenes.h"
#include <fmt/core.h>
#include <fmt/format.h>

#include <cassert>

static SceneEvent event = SceneEvent::NONE;

static ECS::Entity s_NewGameBtn;
static ECS::Entity s_SettingsBtn;
static ECS::Entity s_HelpBtn;
static ECS::Entity s_ExitBtn;

float CenterText(const char *text) {
  return (GetScreenWidth() - MeasureText(text, 20)) / 2.f;
}

void LoadMenu() {
  float screen_cw = GetScreenWidth() / 2.f;
  float screen_ch = GetScreenHeight() / 2.f;

  s_NewGameBtn = ECS::CreateEntity();
  ECS::Add<ECS::PositionComponent>(s_NewGameBtn, CenterText("New Game"), 100.f);
  ECS::Add<ECS::TextComponent>(s_NewGameBtn, "New Game");

  s_SettingsBtn = ECS::CreateEntity();
  ECS::Add<ECS::PositionComponent>(s_SettingsBtn, CenterText("Settings"),
                                   150.f);
  ECS::Add<ECS::TextComponent>(s_SettingsBtn, "Settings");

  s_HelpBtn = ECS::CreateEntity();
  ECS::Add<ECS::PositionComponent>(s_HelpBtn, CenterText("Help"), 200.f);
  ECS::Add<ECS::TextComponent>(s_HelpBtn, "Help");

  s_ExitBtn = ECS::CreateEntity();
  ECS::Add<ECS::PositionComponent>(s_ExitBtn, CenterText("Exit"), 250.f);
  ECS::Add<ECS::TextComponent>(s_ExitBtn, "Exit");

  std::cout << "MENU: LOADED\n";
}

void UpdateMenu(float delta) { ECS::PositionSystem(); }

void DrawMenu() { ECS::RenderSystem(); }

void UnloadMenu() {
  ECS::DeleteEntity(s_NewGameBtn);
  ECS::DeleteEntity(s_SettingsBtn);
  ECS::DeleteEntity(s_HelpBtn);
  ECS::DeleteEntity(s_ExitBtn);
}

SceneEvent OnMenuEvent() { return event; }
