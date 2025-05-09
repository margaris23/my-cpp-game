#include "ecs.hpp"
#include "fmt/core.h"
#include "raylib.h"
#include "scenes.hpp"
#include <algorithm>
#include <random>
#include <vector>

static constexpr int MAX_METEORS = 10;

static SceneEvent s_Event = SceneEvent::NONE;
static struct GameState {
  // TODO: add ...
} s_State;

static std::random_device rd;
static std::mt19937 gen(rd());

static ECS::Entity s_SpaceShip;
static std::vector<ECS::Entity> meteors;

void LoadGame() {
  float screen_cw = GetScreenWidth() / 2.f;
  float screen_ch = GetScreenHeight() / 2.f;

  // Randomizers
  std::uniform_int_distribution<int> num_of_meteors(1, MAX_METEORS);
  std::uniform_real_distribution<float> dist_x(10, GetScreenWidth() - 10);
  std::uniform_real_distribution<float> dist_y(10, GetScreenHeight() - 10);
  std::uniform_real_distribution<float> dist_size(10, 30); // TEMP Until SPRITES impl
  // std::uniform_int_distribution<int> dist_color(0, 50);

  // Our Hero
  s_SpaceShip = ECS::CreateEntity();
  ECS::Add<ECS::PositionComponent>(s_SpaceShip, screen_cw, screen_ch);
  ECS::Add<ECS::RenderComponent>(s_SpaceShip, 20.f, 20.f);

  // Generate Meteors
  for (int i = 0; i < num_of_meteors(gen); i++) {
    meteors.push_back(ECS::CreateEntity());
    ECS::Add<ECS::PositionComponent>(meteors.back(), dist_x(gen), dist_y(gen));
    ECS::Add<ECS::RenderComponent>(meteors.back(), dist_size(gen));
  }

  fmt::println("GAME LOADED!");
}

void UpdateGame(float delta) {
  ECS::UISystem();

  // INPUT Testing
  // TODO: update force in order to support diagonal movement
  if (IsKeyDown(KEY_RIGHT)) {
    ECS::Add<ECS::ForceComponent>(s_SpaceShip, 5.f, 0.f);
  } else if (IsKeyDown(KEY_LEFT)) {
    ECS::Add<ECS::ForceComponent>(s_SpaceShip, -5.f, 0.f);
  } else if (IsKeyDown(KEY_UP)) {
    ECS::Add<ECS::ForceComponent>(s_SpaceShip, 0.f, -5.f);
  } else if (IsKeyDown(KEY_DOWN)) {
    ECS::Add<ECS::ForceComponent>(s_SpaceShip, 0.f, 5.f);
  } else {
    ECS::Remove<ECS::ForceComponent>(s_SpaceShip);
  }

  ECS::PositionSystem();

  // if (IsKeyPressed(KEY_DOWN)) {
  // } else if (IsKeyPressed(KEY_UP)) {
  // } else if (IsKeyPressed(KEY_ENTER)) {
  //   return;
  // }
}

void DrawGame() { ECS::RenderSystem(); }

void UnloadGame() {
  ECS::DeleteEntity(s_SpaceShip);
  std::for_each(meteors.begin(), meteors.end(),
                [](auto meteor) { ECS::DeleteEntity(meteor); });
}

SceneEvent OnGameEvent() { return s_Event; }
