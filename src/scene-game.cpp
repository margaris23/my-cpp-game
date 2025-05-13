#include "ecs.hpp"
#include "fmt/core.h"
#include "raylib.h"
#include "scenes.hpp"
#include <algorithm>
#include <random>
#include <vector>

static constexpr int MAX_METEORS = 5;
static constexpr int MIN_METEOR_SIZE = 10;
static constexpr int MAX_METEOR_SIZE = 30;

static SceneEvent s_Event = SceneEvent::NONE;

// ENTROPY
static std::random_device rd;
static std::mt19937 gen(rd());

// STATE
static ECS::Entity s_SpaceshipHealth;
// static ECS::Entity s_HeroLife;

// Objects
static ECS::Entity s_SpaceShip;
static std::vector<ECS::Entity> meteors;

void LoadGame() {
  float screen_cw = GetScreenWidth() / 2.f;
  float screen_ch = GetScreenHeight() / 2.f;

  // Randomizers
  std::uniform_int_distribution<int> num_of_meteors(1, MAX_METEORS);
  std::uniform_real_distribution<float> dist_x(10, GetScreenWidth() - 10);
  std::uniform_real_distribution<float> dist_y(10, GetScreenHeight() - 10);
  std::uniform_real_distribution<float> dist_size(MIN_METEOR_SIZE, MAX_METEOR_SIZE);
  // std::uniform_int_distribution<int> dist_color(0, 50);

  // Our Hero
  s_SpaceShip = ECS::CreateEntity();
  ECS::Add<ECS::PositionComponent>(s_SpaceShip, screen_cw, screen_ch);
  Vector2 dimension{40.f, 20.f}; // TODO: change dimensions
  ECS::Add<ECS::RenderComponent>(s_SpaceShip, dimension.x, dimension.y);
  ECS::Add<ECS::ColliderComponent>(s_SpaceShip, dimension.x, dimension.y);
  ECS::Add<ECS::DmgComponent>(s_SpaceShip, 0.1f);
  ECS::Add<ECS::HealthComponent>(s_SpaceShip, 10.f); // for collisions

  // State: Spaceship health (UI Entity)
  ECS::Add<ECS::GameStateComponent>(s_SpaceshipHealth, 10.f);
  ECS::Add<ECS::UIComponent>(s_SpaceshipHealth, ECS::UIElement::BAR);
  ECS::Add<ECS::PositionComponent>(s_SpaceshipHealth, 10.f, 10.f);

  // Generate Meteors
  for (int i = 0; i < num_of_meteors(gen); i++) {
    meteors.push_back(ECS::CreateEntity());
    ECS::Entity meteor = meteors.back();
    ECS::Add<ECS::PositionComponent>(meteor, dist_x(gen), dist_y(gen));
    float radius = dist_size(gen);
    ECS::Add<ECS::RenderComponent>(meteor, radius);
    ECS::Add<ECS::ColliderComponent>(meteor, radius);
    ECS::Add<ECS::HealthComponent>(meteor, radius); // bigger means more health
    ECS::Add<ECS::DmgComponent>(meteor, 0.1f);
  }

  fmt::println("GAME LOADED!");
}

void UpdateGame(float delta) {
  if (IsKeyDown(KEY_ESCAPE)) {
    s_Event = SceneEvent::EXIT;
    return;
  }

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
  ECS::CollisionDetectionSystem();
  ECS::CollisionResolutionSystem();

  // Sync state with components i.e Spaceship.health -> SpaceShipHealth.state
  const auto health = ECS::Get<ECS::HealthComponent>(s_SpaceShip);
  auto state = ECS::Get<ECS::GameStateComponent>(s_SpaceshipHealth);
  state->m_value = health->m_value;
}

void DrawGame() {
  ECS::UISystem();
  ECS::RenderSystem();

  // Render Meteors Health for DEBUG
  for (const auto &meteor : meteors) {
    const auto &health = ECS::Get<ECS::HealthComponent>(meteor);
    const auto &pos = ECS::Get<ECS::PositionComponent>(meteor);
    const auto &render = ECS::Get<ECS::RenderComponent>(meteor);
    DrawRectangle(pos->m_value.x + render->m_dimensions.x,
                  pos->m_value.y + render->m_dimensions.x, health->m_value, 10, BLACK);
  }
}

void UnloadGame() {
  s_Event = SceneEvent::NONE;

  ECS::DeleteEntity(s_SpaceShip);
  ECS::DeleteEntity(s_SpaceshipHealth);
  std::for_each(meteors.begin(), meteors.end(),
                [](auto meteor) { ECS::DeleteEntity(meteor); });
}

SceneEvent OnGameEvent() { return s_Event; }
