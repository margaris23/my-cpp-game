#include "ecs.hpp"
#include "fmt/core.h"
#include "raylib.h"
#include "raymath.h"
#include "reasings.h"
#include "scenes.hpp"
#include <algorithm>
#include <random>
#include <unordered_map>
#include <vector>

constexpr static int MIN_METEORS = 3;
constexpr static int MAX_METEORS = 5;
constexpr static int MIN_METEOR_SIZE = 20;
constexpr static int MAX_METEOR_SIZE = 50;
constexpr static float METEORS_WINDOW_PADDING = 50.f;
constexpr static float METEOR_DMG = 0.1f;
constexpr static float METEOR_CORE_SIZE = 10.f;
constexpr static float METEOR_CORE_HEALTH = 0.1f;

constexpr static Vector2 SPACESHIP_SIZE{20.f, 10.f}; // TODO: change dimensions
constexpr static float SPACESHIP_INITIAL_HEALTH = 10.f;
constexpr static float SPACESHIP_INITIAL_LIVES = 3.f;
constexpr static float WEAPON_SIZE = 10.f;
constexpr static float WEAPON_DMG = 1.f;
constexpr static float WEAPON_MAX_DISTANCE = 60.f;

static SceneEvent s_Event = SceneEvent::NONE;

// ENTROPY
static std::random_device rd;
static std::mt19937 gen(rd());

// STATE
enum class GameState { PLAY, PAUSE, WIN, LOST };
static bool s_isFiring = false;
static float s_firingDuration = 0.f;
static GameState s_state = GameState::PLAY;

// ENTITIES
static ECS::Entity s_spaceshipHealth;
static ECS::Entity s_spaceshipLives;
static ECS::Entity s_spaceShip;
static ECS::Entity s_miningBeam;
static ECS::Entity s_score;
static ECS::Entity s_coresCount;
static std::vector<ECS::Entity> s_meteors;

// meteor -> core index
// !@#$% BUG: Strange Rehashing!!! WTF!!!
// item reappears, possible iterator invalidating
// static std::unordered_map<ECS::Entity, size_t> s_cores_map{};
static std::vector<ECS::Entity> s_cores;
// static std::vector<bool> s_core_active;

static size_t s_frame = 0;

void LoadGame() {
  float screen_cw = GetScreenWidth() / 2.f;
  float screen_ch = GetScreenHeight() / 2.f;
  float s_meteors_offset = MAX_METEOR_SIZE + METEORS_WINDOW_PADDING;

  // Randomizers
  std::uniform_int_distribution<int> num_of_s_meteors(MIN_METEORS, MAX_METEORS);
  std::uniform_real_distribution<float> rnd_x(s_meteors_offset,
                                              GetScreenWidth() - s_meteors_offset);
  std::uniform_real_distribution<float> rnd_y(s_meteors_offset,
                                              GetScreenHeight() - s_meteors_offset);
  std::uniform_real_distribution<float> rnd_size(MIN_METEOR_SIZE, MAX_METEOR_SIZE);
  std::uniform_real_distribution<float> rnd_velocity(-1.f, 1.f);

  // Generate Meteors
  s_meteors.reserve(num_of_s_meteors(gen));
  s_cores.reserve(s_meteors.capacity());
  // init cores as inactive
  // s_core_active.resize(s_meteors.capacity(), false);

  for (int i = 0; i < s_meteors.capacity(); i++) {
    float posX = rnd_x(gen);
    float posY = rnd_y(gen);
    float velX = rnd_velocity(gen);
    float velY = rnd_velocity(gen);
    float radius = rnd_size(gen);

    // Meteor Core
    s_cores.push_back(ECS::CreateEntity());
    ECS::Entity core = s_cores.back();
    ECS::Add<ECS::PositionComponent>(core, posX, posY);
    ECS::Add<ECS::VelocityComponent>(core, velX, velY);
    ECS::Add<ECS::RenderComponent>(core, ECS::LAYER::SUB, ECS::Shape::CIRCLE, DARKGREEN,
                                   METEOR_CORE_SIZE);
    ECS::Add<ECS::HealthComponent>(core, METEOR_CORE_HEALTH);
    // NO Collider until core revealed

    // Main Meteor
    s_meteors.push_back(ECS::CreateEntity());
    ECS::Entity meteor = s_meteors.back();
    ECS::Add<ECS::PositionComponent>(meteor, posX, posY);
    // TODO: bigger asteroids should move slower
    ECS::Add<ECS::VelocityComponent>(meteor, velX, velY);
    ECS::Add<ECS::RenderComponent>(meteor, ECS::LAYER::GROUND, ECS::Shape::CIRCLE, BLACK,
                                   radius);
    ECS::Add<ECS::ColliderComponent>(meteor, radius);
    ECS::Add<ECS::HealthComponent>(meteor, radius); // bigger means more health
    ECS::Add<ECS::DmgComponent>(meteor, METEOR_DMG);
  }

  // Our Hero
  s_spaceShip = ECS::CreateEntity();
  ECS::Add<ECS::PositionComponent>(s_spaceShip, screen_cw, screen_ch);
  ECS::Add<ECS::RenderComponent>(s_spaceShip, ECS::LAYER::GROUND, ECS::Shape::ELLIPSE,
                                 BLACK, SPACESHIP_SIZE.x, SPACESHIP_SIZE.y);
  ECS::Add<ECS::ColliderComponent>(s_spaceShip, SPACESHIP_SIZE.x, SPACESHIP_SIZE.y);
  ECS::Add<ECS::DmgComponent>(s_spaceShip, 0.1f);
  ECS::Add<ECS::HealthComponent>(s_spaceShip, SPACESHIP_INITIAL_HEALTH); // for collisions
  ECS::Add<ECS::ForceComponent>(s_spaceShip, 0.f, 0.f); // Initialize empty force

  // State: Spaceship health (UI Entity)
  s_spaceshipHealth = ECS::CreateEntity();
  ECS::Add<ECS::GameStateComponent>(s_spaceshipHealth, SPACESHIP_INITIAL_HEALTH);
  ECS::Add<ECS::UIComponent>(s_spaceshipHealth, ECS::UIElement::BAR);
  ECS::Add<ECS::PositionComponent>(s_spaceshipHealth, 10.f, 10.f);

  s_spaceshipLives = ECS::CreateEntity();
  ECS::Add<ECS::GameStateComponent>(s_spaceshipLives, SPACESHIP_INITIAL_LIVES);
  ECS::Add<ECS::TextComponent>(s_spaceshipLives, "3 Lives", MAROON);
  ECS::Add<ECS::PositionComponent>(
      s_spaceshipLives, GetScreenWidth() - MeasureText(" Lives", 20) - 20.f, 10.f);

  // Scores
  s_score = ECS::CreateEntity();
  ECS::Add<ECS::GameStateComponent>(s_score, 0.f);
  ECS::Add<ECS::TextComponent>(s_score, "0", BROWN);
  ECS::Add<ECS::PositionComponent>(s_score, screen_cw - 10.f, 10.f);

  s_coresCount = ECS::CreateEntity();
  ECS::Add<ECS::GameStateComponent>(s_coresCount, 0.f);
  ECS::Add<ECS::TextComponent>(s_coresCount, "0 Cores", DARKGREEN);
  ECS::Add<ECS::PositionComponent>(s_coresCount, screen_cw * 1.5f, 10.f);

  // Spaceship's Mining Beam
  // TODO: create before spaceship (order matters when rendering)
  s_miningBeam = ECS::CreateEntity();
  ECS::Add<ECS::PositionComponent>(s_miningBeam, 0.f, 0.f);
  ECS::Add<ECS::WeaponComponent>(s_miningBeam, s_spaceShip, WEAPON_MAX_DISTANCE);
  ECS::Add<ECS::DmgComponent>(s_miningBeam, WEAPON_DMG);

  fmt::println("GAME LOADED!");
}

void UpdateGame(float delta) {
  ++s_frame;

  if (IsKeyPressed(KEY_ESCAPE)) {
    s_Event = SceneEvent::EXIT;
    // s_state = GameState::PAUSE == s_state ? GameState::PLAY : GameState::PAUSE;
    return;
  }

  if (GameState::PLAY != s_state) {
    // TODO: implement menu/overlay handling

    // 1. continue
    // 2. settings
    // 3. exit
    return;
  }

  // INPUT Testing
  // TODO: implement a force accumulator
  auto force = ECS::Get<ECS::ForceComponent>(s_spaceShip);
  force->m_value.x = 0;
  force->m_value.y = 0;

  if (IsKeyDown(KEY_RIGHT)) {
    force->m_value.x = 1.f;
  }
  if (IsKeyDown(KEY_LEFT)) {
    force->m_value.x = -1.f;
  }
  if (IsKeyDown(KEY_UP)) {
    force->m_value.y = -1.f;
  }
  if (IsKeyDown(KEY_DOWN)) {
    force->m_value.y = 1.f;
  }
  force->m_value = Vector2Normalize(force->m_value);
  force->m_value.x *= 5.f;
  force->m_value.y *= 5.f;

  if (IsKeyDown(KEY_SPACE)) {
    const auto weapon = ECS::Get<ECS::WeaponComponent>(s_miningBeam);
    if (!s_isFiring) {
      s_isFiring = true;
      s_firingDuration = 0;
      ECS::Add<ECS::ColliderComponent>(s_miningBeam, WEAPON_SIZE, 10.f);
      ECS::Add<ECS::RenderComponent>(s_miningBeam, ECS::LAYER::GROUND,
                                     ECS::Shape::RECTANGLE, BROWN, WEAPON_SIZE, 10.f);
    } else if (s_firingDuration < 24.f) {
      ++s_firingDuration;
      // Ease(now, start, max_change, duration)
      float tween =
          EaseLinearIn(s_firingDuration, 10.f, WEAPON_MAX_DISTANCE - 10.f, 24.f);
      auto beam_collider = ECS::Get<ECS::ColliderComponent>(s_miningBeam);
      if (beam_collider)
        beam_collider->m_dimensions.y = tween;
      auto beam_render = ECS::Get<ECS::RenderComponent>(s_miningBeam);
      if (beam_render)
        beam_render->m_dimensions.y = tween;
    }
  } else if (s_isFiring) {
    s_isFiring = false;
    s_firingDuration = 0;
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

  // SCORE SYSTEM
  auto cores_count = ECS::Get<ECS::GameStateComponent>(s_coresCount);
  for (const auto &core : s_cores) {
    const auto &collider = ECS::Get<ECS::ColliderComponent>(core);
    if (collider && collider->m_collided_with.has_value()) {
      ++cores_count->m_value;
    }
  }
  auto coresCount_text = ECS::Get<ECS::TextComponent>(s_coresCount);
  // TODO: convert to icon
  coresCount_text->m_value = fmt::format("{} Cores", cores_count->m_value);

  auto score = ECS::Get<ECS::GameStateComponent>(s_score);
  for (const auto &meteor : s_meteors) {
    const auto &collider = ECS::Get<ECS::ColliderComponent>(meteor);
    if (collider && collider->m_collided_with.has_value()) {
      // Let's earn 1 point for every hit for now....
      ++score->m_value;
    }
  }
  auto score_text = ECS::Get<ECS::TextComponent>(s_score);
  score_text->m_value = fmt::format("{}", score->m_value);

  ECS::CollisionResolutionSystem();

  // Kill s_cores with zero health
  for (auto core_it = s_cores.begin(); core_it != s_cores.end();) {
    const auto core_health = ECS::Get<ECS::HealthComponent>(*core_it);
    if (core_health && core_health->m_value == 0) {
      ECS::DeleteEntity(*core_it);
      core_it = s_cores.erase(core_it);
    } else {
      ++core_it;
    }
  }

  // Meteors Updates
  for (auto it = s_meteors.begin(); it != s_meteors.end();) {
    // Kill s_meteors with zero health
    const auto meteor_health = ECS::Get<ECS::HealthComponent>(*it);
    if (meteor_health->m_value < 10.f) {
      // every core is considered to be before to a meteor
      const ECS::Entity core = *it - 1;

      ECS::DeleteEntity(*it);
      it = s_meteors.erase(it);

      // Enable core
      auto collider = ECS::Get<ECS::ColliderComponent>(core);
      if (collider == nullptr) {
        // fmt::println("Activating {} for {}", core, meteor);
        // activate core
        auto core_velocity = ECS::Get<ECS::VelocityComponent>(core);
        if (core_velocity) {
          core_velocity->m_value.x *= -1;
          if (core_velocity->m_value.y < 0) {
            core_velocity->m_value.y *= -1;
          }
        }
        auto core_pos = ECS::Get<ECS::PositionComponent>(core);
        if (core_pos) {
          core_pos->m_value.x += 10.f;
          core_pos->m_value.y += 10.f;
        }
        ECS::Add<ECS::ColliderComponent>(core, METEOR_CORE_SIZE);
      }
      continue;
    }

    // Update size based on health
    auto render = ECS::Get<ECS::RenderComponent>(*it);
    render->m_dimensions.x = meteor_health->m_value;

    ++it;
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

  // TEST PLANETS
  // DrawCircleLinesV({300.f, 300.f}, 80.f, BLACK);
  // DrawCircleSectorLines({500.f, 200.f}, 60.f, 0, 180.f, 16, BLACK);
  // DrawSphereWires({100.f, 100.f, 0.f}, 20.f, 16, 4, BLACK);
  // DrawSphereEx({200.f, 200.f, 0.f}, 50.f, 16, 4, BLACK);

  // DrawLineStrip(shipPoints.data(), shipPoints.size(), BLACK);

  ECS::RenderSystem();

  // DrawEllipseLines(100.f, 100.f, 20.f, 10.f, BLACK);

  // DrawText(TextFormat("%f", s_firingDuration), 0.f, GetScreenHeight() - 30.f, 20, RED);

  // OVERLAY
  // DrawRectangleLines(5.f, 5.f, GetScreenWidth() - 10.f, GetScreenHeight() - 10.f,
  // BLACK); DrawRectangle(15.f, 15.f, GetScreenWidth() - 30.f, GetScreenHeight() - 30.f,
  // RED);

  // Render Meteors info for DEBUG
  // for (const auto &meteor : s_meteors) {
  //   const auto &health = ECS::Get<ECS::HealthComponent>(meteor);
  //   const auto &pos = ECS::Get<ECS::PositionComponent>(meteor);
  //   const auto &render = ECS::Get<ECS::RenderComponent>(meteor);
  //   DrawText(TextFormat("%i", meteor), pos->m_value.x,
  //            pos->m_value.y + render->m_dimensions.x, 20, GREEN);
  //   // DrawRectangle(pos->m_value.x + render->m_dimensions.x,
  //   //               pos->m_value.y + render->m_dimensions.x, health->m_value, 10,
  //   BLACK);
  // }

  // Render Cores info for DEBUG
  // for (const auto &core : s_cores) {
  //   const auto &pos = ECS::Get<ECS::PositionComponent>(core);
  //   const auto &render = ECS::Get<ECS::RenderComponent>(core);
  //   DrawText(TextFormat("%i", core), pos->m_value.x,
  //            pos->m_value.y + render->m_dimensions.x, 20, RED);
  //   // DrawRectangle(pos->m_value.x + render->m_dimensions.x,
  //   //               pos->m_value.y + render->m_dimensions.x, health->m_value, 10,
  //   BLACK);
  // }

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
  ECS::DeleteEntity(s_coresCount);
  ECS::DeleteEntity(s_miningBeam);
  std::for_each(s_meteors.begin(), s_meteors.end(),
                [](auto meteor) { ECS::DeleteEntity(meteor); });
  std::for_each(s_cores.begin(), s_cores.end(),
                [](auto core) { ECS::DeleteEntity(core); });
}

SceneEvent OnGameEvent() { return s_Event; }
