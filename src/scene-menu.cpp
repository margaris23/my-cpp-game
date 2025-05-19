#include "ecs.hpp"
#include "fmt/core.h"
#include "raylib.h"
#include "scenes.hpp"
#include <string_view>
#include <vector>

static SceneEvent s_Event = SceneEvent::NONE;

// TODO: change to GameStateComponent when impl
static struct MenuState {
  ECS::Entity selected;
  bool loaded;
} s_State;

constexpr static float PADDING = 10.f;

static std::unique_ptr<ECS::Registry> s_Registry;
static std::vector<ButtonConfig> s_buttonConfigs;

static ECS::Entity s_SelectedBtn;

// Get PosX of Horizontally Screen centered text
static float H_CenterText(const std::string_view &text) {
  return (GetScreenWidth() - MeasureText(text.data(), 20)) / 2.f;
}

void LoadMenu(std::vector<ButtonConfig> &&config) {
  float screen_cw = GetScreenWidth() / 2.f;
  float screen_ch = GetScreenHeight() / 2.f;

  s_buttonConfigs = std::move(config);

  s_Registry = std::make_unique<ECS::Registry>();

  using ECS::PositionComponent, ECS::RenderComponent, ECS::TextComponent, ECS::Layer, ECS::Shape;

  // Menu Stylistic Entities
  float menu_height = (s_buttonConfigs.size() - 1) * 50.f + 25.f;

  auto bkg = s_Registry->CreateEntity();
  s_Registry->Add<PositionComponent>(bkg, screen_cw - 60.f - PADDING, 95.f - 2.f * PADDING);
  s_Registry->Add<RenderComponent>(bkg, Layer::GROUND, Shape::RECTANGLE_SOLID, RAYWHITE,
                                   95.f + 4.f * PADDING, menu_height + 4.f * PADDING);
  auto border1 = s_Registry->CreateEntity();
  s_Registry->Add<PositionComponent>(border1, screen_cw - 60.f - PADDING, 95.f - 2.f * PADDING);
  s_Registry->Add<RenderComponent>(border1, Layer::GROUND, Shape::RECTANGLE, BLACK,
                                   95.f + 4.f * PADDING, menu_height + 4.f * PADDING);
  auto border2 = s_Registry->CreateEntity();
  s_Registry->Add<PositionComponent>(border2, screen_cw - 55.f - PADDING, 100.f - 2.f * PADDING);
  s_Registry->Add<RenderComponent>(border2, Layer::GROUND, Shape::RECTANGLE, BLACK,
                                   95.f + 3.f * PADDING, menu_height + 3.f * PADDING);

  // Menu Buttons + Selection
  s_SelectedBtn = s_Registry->CreateEntity();
  s_Registry->Add<PositionComponent>(s_SelectedBtn, screen_cw - 47.5f - PADDING, 100.f - PADDING);
  s_Registry->Add<RenderComponent>(s_SelectedBtn, Layer::GROUND, Shape::RECTANGLE, BLACK,
                                   95.f + PADDING, 20.f + 2.f * PADDING);

  float btn_index = 0.f;
  for (const auto &config : s_buttonConfigs) {
    ECS::Entity button = s_Registry->CreateEntity();
    s_Registry->Add<PositionComponent>(button, H_CenterText(config.first),
                                       100.f + 50.f * btn_index);
    s_Registry->Add<TextComponent>(button, config.first);
    ++btn_index;
  }

  // UPDATE MENU STATE
  s_State.selected = 0;
  s_State.loaded = true;
}

void UpdateMenu(float delta) {
  s_Registry->PositionSystem();

  // Handle UI here ... for now
  int mod = 0;
  if (IsKeyPressed(KEY_ESCAPE)) {
    s_Event = SceneEvent::CONTINUE;
  } else if (IsKeyPressed(KEY_DOWN)) {
    mod = 1;
  } else if (IsKeyPressed(KEY_UP)) {
    mod = -1;
  } else if (IsKeyPressed(KEY_ENTER)) {
    s_Event = s_buttonConfigs[s_State.selected].second;
    return;
  }

  // Update Selected Btn Position Component
  // s_State.selected = (s_State.selected + mod) % (s_ExitBtn - s_NewGameBtn + 1);
  s_State.selected = (s_State.selected + mod) % s_buttonConfigs.size();

  auto selectedBtnComponent = s_Registry->Get<ECS::PositionComponent>(s_SelectedBtn);
  if (selectedBtnComponent) {
    /* TODO: calculate width somehow */
    selectedBtnComponent->value.x = H_CenterText("           ") - 10.f;
    selectedBtnComponent->value.y = 100.f + s_State.selected * 50.f - 10.f;
  }
}

void DrawMenu() {
  s_Registry->UISystem();
  s_Registry->RenderSystem();
}

void UnloadMenu() {
  s_Event = SceneEvent::NONE;
  s_Registry.reset();
  s_State.loaded = false;
  s_buttonConfigs.clear();
}

bool IsMenuUnloaded() { return !s_State.loaded; }

SceneEvent OnMenuEvent() { return s_Event; }
