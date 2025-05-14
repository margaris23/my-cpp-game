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

constexpr static Vector2 SPACESHIP_SIZE{40.f, 20.f}; // TODO: change dimensions
constexpr static float SPACESHIP_INITIAL_HEALTH = 10.f;
constexpr static float WEAPON_SIZE = 10.f;

static SceneEvent s_Event = SceneEvent::NONE;

// ENTROPY
static std::random_device rd;
static std::mt19937 gen(rd());

// STATE
enum class GameState { PLAY, PAUSE, WIN, LOST };

static ECS::Entity s_spaceshipHealth;
static ECS::Entity s_spaceshipLives;
static bool s_isFiring = false;
static GameState s_state = GameState::PLAY;

// Objects
static ECS::Entity s_spaceShip;
static std::vector<ECS::Entity> meteors;
static ECS::Entity s_miningBeam;

void LoadGame() {
  float screen_cw = GetScreenWidth() / 2.f;
  float screen_ch = GetScreenHeight() / 2.f;

  // Randomizers
  std::uniform_int_distribution<int> num_of_meteors(1, MAX_METEORS);
  std::uniform_real_distribution<float> dist_x(MAX_METEOR_SIZE,
                                               GetScreenWidth() - MAX_METEOR_SIZE);
  std::uniform_real_distribution<float> dist_y(MAX_METEOR_SIZE,
                                               GetScreenHeight() - MAX_METEOR_SIZE);
  std::uniform_real_distribution<float> dist_size(MIN_METEOR_SIZE, MAX_METEOR_SIZE);
  // std::uniform_int_distribution<int> dist_color(0, 50);

  // Our Hero
  s_spaceShip = ECS::CreateEntity();
  ECS::Add<ECS::PositionComponent>(s_spaceShip, screen_cw, screen_ch);
  ECS::Add<ECS::RenderComponent>(s_spaceShip, SPACESHIP_SIZE.x, SPACESHIP_SIZE.y);
  ECS::Add<ECS::ColliderComponent>(s_spaceShip, SPACESHIP_SIZE.x, SPACESHIP_SIZE.y);
  ECS::Add<ECS::DmgComponent>(s_spaceShip, 0.1f);
  ECS::Add<ECS::HealthComponent>(s_spaceShip, SPACESHIP_INITIAL_HEALTH); // for collisions

  // State: Spaceship health (UI Entity)
  s_spaceshipHealth = ECS::CreateEntity();
  ECS::Add<ECS::GameStateComponent>(s_spaceshipHealth, SPACESHIP_INITIAL_HEALTH);
  ECS::Add<ECS::UIComponent>(s_spaceshipHealth, ECS::UIElement::BAR);
  ECS::Add<ECS::PositionComponent>(s_spaceshipHealth, 10.f, 10.f);

  s_spaceshipLives = ECS::CreateEntity();
  ECS::Add<ECS::GameStateComponent>(s_spaceshipLives, 3.f);
  ECS::Add<ECS::TextComponent>(s_spaceshipLives, "3 Lives");
  ECS::Add<ECS::PositionComponent>(
      s_spaceshipLives, GetScreenWidth() - MeasureText(" Lives", 20) - 20.f, 10.f);

  // Spaceship's Mining Beam
  // TODO: create before spaceship (order matters when rendering)
  s_miningBeam = ECS::CreateEntity();
  ECS::Add<ECS::PositionComponent>(s_miningBeam, 0.f, 0.f);
  float weapon_max_distance = 60.f;
  float weapon_fire_dmg = 1.f;
  ECS::Add<ECS::WeaponComponent>(s_miningBeam, s_spaceShip, weapon_max_distance);
  ECS::Add<ECS::DmgComponent>(s_miningBeam, weapon_fire_dmg);

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

  if (GameState::PLAY != s_state) {
    // TODO: implement menu/overlay handling
    return;
  }

  // INPUT Testing
  // TODO: update force in order to support diagonal movement
  // TODO: implementa force accumulator
  if (IsKeyDown(KEY_RIGHT)) {
    ECS::Add<ECS::ForceComponent>(s_spaceShip, 5.f, 0.f);
  } else if (IsKeyDown(KEY_LEFT)) {
    ECS::Add<ECS::ForceComponent>(s_spaceShip, -5.f, 0.f);
  } else if (IsKeyDown(KEY_UP)) {
    ECS::Add<ECS::ForceComponent>(s_spaceShip, 0.f, -5.f);
  } else if (IsKeyDown(KEY_DOWN)) {
    ECS::Add<ECS::ForceComponent>(s_spaceShip, 0.f, 5.f);
  } else {
    ECS::Remove<ECS::ForceComponent>(s_spaceShip);
  }

  if (IsKeyDown(KEY_SPACE)) {
    if (!s_isFiring) {
      s_isFiring = true;
      const auto &weapon = ECS::Get<ECS::WeaponComponent>(s_miningBeam);
      ECS::Add<ECS::ColliderComponent>(s_miningBeam, WEAPON_SIZE, weapon->m_max_length);
      ECS::Add<ECS::RenderComponent>(s_miningBeam, WEAPON_SIZE, weapon->m_max_length);
    }
  } else if (s_isFiring) {
    s_isFiring = false;
    ECS::Remove<ECS::RenderComponent>(s_miningBeam);
    ECS::Remove<ECS::ColliderComponent>(s_miningBeam);
  }

  // Reset spaceship in case collider was removed
  if (!ECS::Get<ECS::ColliderComponent>(s_spaceShip)) {
    auto health = ECS::Get<ECS::HealthComponent>(s_spaceShip);
    health->m_value = SPACESHIP_INITIAL_HEALTH;
    ECS::Add<ECS::ColliderComponent>(s_spaceShip, SPACESHIP_SIZE.x, SPACESHIP_SIZE.y);
  }

  ECS::PositionSystem();
  ECS::CollisionDetectionSystem();
  ECS::CollisionResolutionSystem();

  // Sync state with components i.e Spaceship.health -> SpaceShipHealth.state
  const auto health = ECS::Get<ECS::HealthComponent>(s_spaceShip);
  auto state = ECS::Get<ECS::GameStateComponent>(s_spaceshipHealth);
  state->m_value = health->m_value;

  // Sync health/lives
  if (health->m_value == 0) {
    auto spaceship_lives = ECS::Get<ECS::GameStateComponent>(s_spaceshipLives);
    --spaceship_lives->m_value;

    // remove collider so that on next cycle we will reset spaceship
    ECS::Remove<ECS::ColliderComponent>(s_spaceShip);

    if (spaceship_lives->m_value == 0) {
      s_state = GameState::LOST;

      // TODO: add LOST TEXT entity
    }

    auto text = ECS::Get<ECS::TextComponent>(s_spaceshipLives);
    text->m_value = fmt::format("{} Lives", spaceship_lives->m_value);
  }
}

void DrawGame() {
  ECS::UISystem();
  ECS::RenderSystem();

  // Render Meteors Healths for DEBUG
  for (const auto &meteor : meteors) {
    const auto &health = ECS::Get<ECS::HealthComponent>(meteor);
    const auto &pos = ECS::Get<ECS::PositionComponent>(meteor);
    const auto &render = ECS::Get<ECS::RenderComponent>(meteor);
    DrawRectangle(pos->m_value.x + render->m_dimensions.x,
                  pos->m_value.y + render->m_dimensions.x, health->m_value, 10, BLACK);
  }

  // DEBUG
  // const auto &collider = ECS::Get<ECS::ColliderComponent>(s_miningBeam);
  // if (collider) {
  //   DrawText(TextFormat("Col(%i) - w = %f, h = %f", collider->m_shape,
  //                       collider->m_dimensions.x, collider->m_dimensions.y),
  //            10, 50, 20, RED);
  // }
  //
  // DrawText(TextFormat("%i", ECS::colliders.dense.size()), 10, 70, 20, RED);
  // const auto &beam_pos = ECS::Get<ECS::PositionComponent>(s_miningBeam);
  // DrawText(TextFormat("Beam: %f, %f", beam_pos->m_value.x, beam_pos->m_value.y), 10,
  // 50,
  //          20, RED);
}

void UnloadGame() {
  s_Event = SceneEvent::NONE;

  ECS::DeleteEntity(s_spaceShip);
  ECS::DeleteEntity(s_spaceshipHealth);
  ECS::DeleteEntity(s_spaceshipLives);
  ECS::DeleteEntity(s_miningBeam);
  std::for_each(meteors.begin(), meteors.end(),
                [](auto meteor) { ECS::DeleteEntity(meteor); });
}

SceneEvent OnGameEvent() { return s_Event; }
