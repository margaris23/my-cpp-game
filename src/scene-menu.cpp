#include "ecs.hpp"
#include "fmt/core.h"
#include "raylib.h"
#include "scenes.hpp"

static SceneEvent s_Event = SceneEvent::NONE;

// TODO: change to GameStateComponent when impl
static struct MenuState {
  ECS::Entity selected;
} s_State;

static ECS::Entity s_NewGameBtn;
static ECS::Entity s_SettingsBtn;
static ECS::Entity s_HelpBtn;
static ECS::Entity s_ExitBtn;
static ECS::Entity s_SelectedBtn;

// Get PosX of Horizontally Screen centered text
static float H_CenterText(const char *text) {
  return (GetScreenWidth() - MeasureText(text, 20)) / 2.f;
}

void LoadMenu() {
  float screen_cw = GetScreenWidth() / 2.f;
  float screen_ch = GetScreenHeight() / 2.f;

  s_SelectedBtn = ECS::CreateEntity();

  int textWidth = MeasureText("New Game", 20);
  float padding = 10.f;
  ECS::Add<ECS::PositionComponent>(s_SelectedBtn, H_CenterText("New Game") - padding,
                                   100.f - padding);
  // Rectangle { width, height }
  ECS::Add<ECS::RenderComponent>(s_SelectedBtn, BLACK, textWidth + 2.f * padding,
                                 20.f + 2 * padding);

  s_NewGameBtn = ECS::CreateEntity();
  ECS::Add<ECS::PositionComponent>(s_NewGameBtn, H_CenterText("New Game"), 100.f);
  ECS::Add<ECS::TextComponent>(s_NewGameBtn, "New Game");

  s_SettingsBtn = ECS::CreateEntity();
  ECS::Add<ECS::PositionComponent>(s_SettingsBtn, H_CenterText("Settings"), 150.f);
  ECS::Add<ECS::TextComponent>(s_SettingsBtn, "Settings");

  s_HelpBtn = ECS::CreateEntity();
  ECS::Add<ECS::PositionComponent>(s_HelpBtn, H_CenterText("Help"), 200.f);
  ECS::Add<ECS::TextComponent>(s_HelpBtn, "Help");

  s_ExitBtn = ECS::CreateEntity();
  ECS::Add<ECS::PositionComponent>(s_ExitBtn, H_CenterText("Exit"), 250.f);
  ECS::Add<ECS::TextComponent>(s_ExitBtn, "Exit");

  // UPDATE MENU STATE
  s_State.selected = 0;
}

void UpdateMenu(float delta) {
  ECS::PositionSystem();

  // Handle UI here ... for now
  int mod = 0;
  if (IsKeyPressed(KEY_DOWN)) {
    mod = 1;
  } else if (IsKeyPressed(KEY_UP)) {
    mod = -1;
  } else if (IsKeyPressed(KEY_ENTER)) {
    ECS::Entity eid = s_NewGameBtn + s_State.selected;
    if (eid == s_NewGameBtn) {
      s_Event = SceneEvent::NEXT;
    } else if (eid == s_ExitBtn) {
      s_Event = SceneEvent::EXIT;
    }
    return;
  }

  // Update Selected Btn Position Component
  s_State.selected = (s_State.selected + mod) % (s_ExitBtn - s_NewGameBtn + 1);

  auto selectedBtnComponent = ECS::Get<ECS::PositionComponent>(s_SelectedBtn);
  if (selectedBtnComponent) {
    /* TODO: calculate width somehow */
    selectedBtnComponent->m_value.x = H_CenterText("           ") - 10.f;
    selectedBtnComponent->m_value.y = 100.f + s_State.selected * 50.f - 10.f;
  }
}

void DrawMenu() {
  ECS::UISystem();
  ECS::RenderSystem();
}

void UnloadMenu() {
  s_Event = SceneEvent::NONE;
  ECS::DeleteEntity(s_NewGameBtn);
  ECS::DeleteEntity(s_SettingsBtn);
  ECS::DeleteEntity(s_HelpBtn);
  ECS::DeleteEntity(s_ExitBtn);
  ECS::DeleteEntity(s_SelectedBtn);
}

SceneEvent OnMenuEvent() { return s_Event; }
