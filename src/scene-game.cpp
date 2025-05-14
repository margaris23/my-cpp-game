#include "ecs.hpp"
#include "fmt/core.h"
#include "raylib.h"
#include "scenes.hpp"
#include <algorithm>
#include <random>
#include <vector>

constexpr static int MAX_METEORS = 5;
constexpr static int MIN_METEOR_SIZE = 10;
constexpr static int MAX_METEOR_SIZE = 30;
constexpr static float METEORS_WINDOW_PADDING = 50.f;

constexpr static Vector2 SPACESHIP_SIZE{40.f, 20.f}; // TODO: change dimensions
constexpr static float SPACESHIP_INITIAL_HEALTH = 10.f;
constexpr static float SPACESHIP_INITIAL_LIVES = 3.f;
constexpr static float WEAPON_SIZE = 10.f;

static SceneEvent s_Event = SceneEvent::NONE;

// ENTROPY
static std::random_device rd;
static std::mt19937 gen(rd());

// STATE
enum class GameState { PLAY, PAUSE, WIN, LOST };
static bool s_isFiring = false;
static GameState s_state = GameState::PLAY;

// ENTITIES
static ECS::Entity s_spaceshipHealth;
static ECS::Entity s_spaceshipLives;
static ECS::Entity s_spaceShip;
static std::vector<ECS::Entity> meteors;
static ECS::Entity s_miningBeam;
static ECS::Entity s_score;

void LoadGame() {
  float screen_cw = GetScreenWidth() / 2.f;
  float screen_ch = GetScreenHeight() / 2.f;
  float meteors_offset = MAX_METEOR_SIZE + METEORS_WINDOW_PADDING;

  // Randomizers
  std::uniform_int_distribution<int> num_of_meteors(1, MAX_METEORS);
  std::uniform_real_distribution<float> dist_x(meteors_offset,
                                               GetScreenWidth() - meteors_offset);
  std::uniform_real_distribution<float> dist_y(meteors_offset,
                                               GetScreenHeight() - meteors_offset);
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
  ECS::Add<ECS::GameStateComponent>(s_spaceshipLives, SPACESHIP_INITIAL_LIVES);
  ECS::Add<ECS::TextComponent>(s_spaceshipLives, "3 Lives");
  ECS::Add<ECS::PositionComponent>(
      s_spaceshipLives, GetScreenWidth() - MeasureText(" Lives", 20) - 20.f, 10.f);

  s_score = ECS::CreateEntity();
  ECS::Add<ECS::GameStateComponent>(s_score, 0.f);
  ECS::Add<ECS::TextComponent>(s_score, "0");
  ECS::Add<ECS::PositionComponent>(s_score, screen_cw - 10.f, 10.f);

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

  // Sync meteor health, decide on score (Score System ???)
  auto score = ECS::Get<ECS::GameStateComponent>(s_score);
  for (const auto &meteor : meteors) {
    const auto &collider = ECS::Get<ECS::ColliderComponent>(meteor);
    if (collider->m_collided_with.has_value()) {
      // Let's earn 1 point for every hit for now....
      ++score->m_value;
    }
  }
  auto score_text = ECS::Get<ECS::TextComponent>(s_score);
  score_text->m_value = fmt::format("{}", score->m_value);

  ECS::CollisionResolutionSystem();

  // Kill meteors with zero health
  for (std::vector<ECS::Entity>::iterator it = meteors.begin(); it != meteors.end();) {
    const auto meteor_health = ECS::Get<ECS::HealthComponent>(*it);
    if (meteor_health->m_value == 0) {
      ECS::DeleteEntity(*it);
      it = meteors.erase(it);
    } else {
      ++it;
    }
  }

  // UISystem-UPDATE ????
  // Sync state with components i.e Spaceship.health -> SpaceShipHealth.state
  const auto health = ECS::Get<ECS::HealthComponent>(s_spaceShip);
  auto state = ECS::Get<ECS::GameStateComponent>(s_spaceshipHealth);
  state->m_value = health->m_value;

  // Sync Spaceship health/lives (HealthSystem ???)
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
  ECS::DeleteEntity(s_score);
  ECS::DeleteEntity(s_miningBeam);
  std::for_each(meteors.begin(), meteors.end(),
                [](auto meteor) { ECS::DeleteEntity(meteor); });
}

SceneEvent OnGameEvent() { return s_Event; }
