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
static bool s_IsFocused = false;

static std::unique_ptr<ECS::Registry> s_Registry;

// ENTROPY
static std::random_device rd;
static std::mt19937 gen(rd());

// STATE
enum class GameState { PLAY, PAUSE, WON, LOST };
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
static std::vector<ECS::Entity> s_cores;

constexpr static size_t FRAME_MAX_COUNTER = 3600;
static size_t s_frame = 0;

void LoadGame() {
  float screen_cw = GetScreenWidth() / 2.f;
  float screen_ch = GetScreenHeight() / 2.f;
  float s_meteors_offset = MAX_METEOR_SIZE + METEORS_WINDOW_PADDING;

  s_Registry = std::make_unique<ECS::Registry>();

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

  for (int i = 0; i < s_meteors.capacity(); i++) {
    float posX = rnd_x(gen);
    float posY = rnd_y(gen);
    float velX = rnd_velocity(gen);
    float velY = rnd_velocity(gen);
    float radius = rnd_size(gen);

    // Meteor Core
    s_cores.push_back(s_Registry->CreateEntity());
    ECS::Entity core = s_cores.back();
    s_Registry->Add<ECS::PositionComponent>(core, posX, posY);
    s_Registry->Add<ECS::VelocityComponent>(core, velX, velY);
    s_Registry->Add<ECS::RenderComponent>(core, ECS::LAYER::SUB, ECS::Shape::CIRCLE,
                                          DARKGREEN, METEOR_CORE_SIZE);
    s_Registry->Add<ECS::HealthComponent>(core, METEOR_CORE_HEALTH);
    // NO Collider until core revealed

    // Main Meteor
    s_meteors.push_back(s_Registry->CreateEntity());
    ECS::Entity meteor = s_meteors.back();
    s_Registry->Add<ECS::PositionComponent>(meteor, posX, posY);
    // TODO: bigger asteroids should move slower
    s_Registry->Add<ECS::VelocityComponent>(meteor, velX, velY);
    s_Registry->Add<ECS::RenderComponent>(meteor, ECS::LAYER::GROUND, ECS::Shape::CIRCLE,
                                          BLACK, radius);
    s_Registry->Add<ECS::ColliderComponent>(meteor, radius);
    s_Registry->Add<ECS::HealthComponent>(meteor, radius); // bigger means more health
    s_Registry->Add<ECS::DmgComponent>(meteor, METEOR_DMG);
  }

  // Our Hero
  s_spaceShip = s_Registry->CreateEntity();
  s_Registry->Add<ECS::PositionComponent>(s_spaceShip, screen_cw, screen_ch);
  s_Registry->Add<ECS::RenderComponent>(s_spaceShip, ECS::LAYER::GROUND,
                                        ECS::Shape::ELLIPSE, BLACK, SPACESHIP_SIZE.x,
                                        SPACESHIP_SIZE.y);
  s_Registry->Add<ECS::ColliderComponent>(s_spaceShip, SPACESHIP_SIZE.x,
                                          SPACESHIP_SIZE.y);
  s_Registry->Add<ECS::DmgComponent>(s_spaceShip, 0.1f);
  s_Registry->Add<ECS::HealthComponent>(s_spaceShip,
                                        SPACESHIP_INITIAL_HEALTH); // for collisions
  s_Registry->Add<ECS::ForceComponent>(s_spaceShip, 0.f, 0.f); // Initialize empty force

  // State: Spaceship health (UI Entity)
  s_spaceshipHealth = s_Registry->CreateEntity();
  s_Registry->Add<ECS::GameStateComponent>(s_spaceshipHealth, SPACESHIP_INITIAL_HEALTH);
  s_Registry->Add<ECS::UIComponent>(s_spaceshipHealth, ECS::UIElement::BAR);
  s_Registry->Add<ECS::PositionComponent>(s_spaceshipHealth, 10.f, 10.f);

  s_spaceshipLives = s_Registry->CreateEntity();
  s_Registry->Add<ECS::GameStateComponent>(s_spaceshipLives, SPACESHIP_INITIAL_LIVES);
  s_Registry->Add<ECS::TextComponent>(s_spaceshipLives, "3 Lives", MAROON);
  s_Registry->Add<ECS::PositionComponent>(
      s_spaceshipLives, GetScreenWidth() - MeasureText(" Lives", 20) - 20.f, 10.f);

  // Scores
  s_score = s_Registry->CreateEntity();
  s_Registry->Add<ECS::GameStateComponent>(s_score, 0.f);
  s_Registry->Add<ECS::TextComponent>(s_score, "0", BROWN);
  s_Registry->Add<ECS::PositionComponent>(s_score, screen_cw - 10.f, 10.f);

  s_coresCount = s_Registry->CreateEntity();
  s_Registry->Add<ECS::GameStateComponent>(s_coresCount, 0.f);
  s_Registry->Add<ECS::TextComponent>(s_coresCount, "0 Cores", DARKGREEN);
  s_Registry->Add<ECS::PositionComponent>(s_coresCount, screen_cw * 1.5f, 10.f);

  // Spaceship's Mining Beam
  // TODO: create before spaceship (order matters when rendering)
  s_miningBeam = s_Registry->CreateEntity();
  s_Registry->Add<ECS::PositionComponent>(s_miningBeam, 0.f, 0.f);
  s_Registry->Add<ECS::WeaponComponent>(s_miningBeam, s_spaceShip, WEAPON_MAX_DISTANCE);
  s_Registry->Add<ECS::DmgComponent>(s_miningBeam, WEAPON_DMG);

  fmt::println("GAME LOADED!");
  s_IsFocused = true;
}

void UpdateGame(float delta) {
  // check if round is won or lost
  if (s_cores.size() == 0) {
    s_state = GameState::WON;
  } else {
    auto lives = s_Registry->Get<ECS::GameStateComponent>(s_spaceshipLives);
    if (lives->m_value == 0) {
      s_state = GameState::LOST;
    }
  }

  // HANDLE INPUT only when in FOCUS
  if (s_IsFocused) {
    if (IsKeyPressed(KEY_ESCAPE)) {
      // s_Event = SceneEvent::EXIT;
      s_state = GameState::PAUSE;
      s_Event = SceneEvent::PAUSE;
      return;
    }

    // TODO: implement a force accumulator
    auto force = s_Registry->Get<ECS::ForceComponent>(s_spaceShip);
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

    // WEAPON FIRING
    if (IsKeyDown(KEY_SPACE)) {
      const auto weapon = s_Registry->Get<ECS::WeaponComponent>(s_miningBeam);
      if (!s_isFiring) {
        s_isFiring = true;
        s_firingDuration = 0;
        s_Registry->Add<ECS::ColliderComponent>(s_miningBeam, WEAPON_SIZE, 10.f);
        s_Registry->Add<ECS::RenderComponent>(s_miningBeam, ECS::LAYER::GROUND,
                                       ECS::Shape::RECTANGLE, BROWN, WEAPON_SIZE, 10.f);
      } else {
        auto beam_collider = s_Registry->Get<ECS::ColliderComponent>(s_miningBeam);

        if (beam_collider) {
          if (!beam_collider->m_collided_with.has_value() && s_firingDuration < 24.f) {
            ++s_firingDuration;
          }

          // Ease(now, start, max_change, duration)
          float tween =
              EaseLinearIn(s_firingDuration, 10.f, WEAPON_MAX_DISTANCE - 10.f, 24.f);
          beam_collider->m_dimensions.y = tween;

          auto beam_render = s_Registry->Get<ECS::RenderComponent>(s_miningBeam);
          if (beam_render) {
            beam_render->m_dimensions.y = tween;
          }
        }
      }
    } else if (s_isFiring) {
      s_isFiring = false;
      s_firingDuration = 0;
      s_Registry->Remove<ECS::RenderComponent>(s_miningBeam);
      s_Registry->Remove<ECS::ColliderComponent>(s_miningBeam);
    }
  }

  if (GameState::PAUSE == s_state) {
    return;
  }

  s_frame = (s_frame + 1) % FRAME_MAX_COUNTER;

  // Reset spaceship in case collider was removed
  if (!s_Registry->Get<ECS::ColliderComponent>(s_spaceShip)) {
    auto health = s_Registry->Get<ECS::HealthComponent>(s_spaceShip);
    health->m_value = SPACESHIP_INITIAL_HEALTH;
    s_Registry->Add<ECS::ColliderComponent>(s_spaceShip, SPACESHIP_SIZE.x,
                                            SPACESHIP_SIZE.y);
  }

  s_Registry->PositionSystem();
  s_Registry->CollisionDetectionSystem();

  // SCORE SYSTEM
  // CORES
  auto cores_count = s_Registry->Get<ECS::GameStateComponent>(s_coresCount);
  for (const auto &core : s_cores) {
    const auto &collider = s_Registry->Get<ECS::ColliderComponent>(core);
    if (collider && collider->m_collided_with.has_value()) {
      ++cores_count->m_value;
    }
  }
  auto coresCount_text = s_Registry->Get<ECS::TextComponent>(s_coresCount);
  coresCount_text->m_value = fmt::format("{} Cores", cores_count->m_value);
  // SCORE
  auto score = s_Registry->Get<ECS::GameStateComponent>(s_score);
  for (const auto &meteor : s_meteors) {
    const auto &collider = s_Registry->Get<ECS::ColliderComponent>(meteor);
    if (collider && collider->m_collided_with.has_value()) {
      // Let's earn 1 point for every hit for now....
      ++score->m_value;
    }
  }
  auto score_text = s_Registry->Get<ECS::TextComponent>(s_score);
  score_text->m_value = fmt::format("{}", score->m_value);

  s_Registry->CollisionResolutionSystem();

  // Kill s_cores with zero health
  for (auto core_it = s_cores.begin(); core_it != s_cores.end();) {
    const auto core_health = s_Registry->Get<ECS::HealthComponent>(*core_it);
    if (core_health && core_health->m_value == 0) {
      s_Registry->DeleteEntity(*core_it);
      core_it = s_cores.erase(core_it);
    } else {
      ++core_it;
    }
  }

  // Meteors Updates
  for (auto it = s_meteors.begin(); it != s_meteors.end();) {
    // Kill s_meteors with zero health
    const auto meteor_health = s_Registry->Get<ECS::HealthComponent>(*it);
    if (meteor_health->m_value < 10.f) {
      // every core is considered to be before to a meteor
      const ECS::Entity core = *it - 1;

      s_Registry->DeleteEntity(*it);
      it = s_meteors.erase(it);

      // Enable core
      auto collider = s_Registry->Get<ECS::ColliderComponent>(core);
      if (collider == nullptr) {
        // fmt::println("Activating {} for {}", core, meteor);
        // activate core
        auto core_velocity = s_Registry->Get<ECS::VelocityComponent>(core);
        if (core_velocity) {
          core_velocity->m_value.x *= -1;
          if (core_velocity->m_value.y < 0) {
            core_velocity->m_value.y *= -1;
          }
        }
        auto core_pos = s_Registry->Get<ECS::PositionComponent>(core);
        if (core_pos) {
          core_pos->m_value.x += 10.f;
          core_pos->m_value.y += 10.f;
        }
        s_Registry->Add<ECS::ColliderComponent>(core, METEOR_CORE_SIZE);
      }
      continue;
    }

    // Update size based on health
    auto render = s_Registry->Get<ECS::RenderComponent>(*it);
    render->m_dimensions.x = meteor_health->m_value;

    ++it;
  }

  // UISystem-UPDATE ????
  // Sync state with components i.e Spaceship.health -> SpaceShipHealth.state
  const auto health = s_Registry->Get<ECS::HealthComponent>(s_spaceShip);
  auto state = s_Registry->Get<ECS::GameStateComponent>(s_spaceshipHealth);
  state->m_value = health->m_value;

  // Sync Spaceship health/lives (HealthSystem ???)
  if (health->m_value == 0) {
    auto spaceship_lives = s_Registry->Get<ECS::GameStateComponent>(s_spaceshipLives);
    --spaceship_lives->m_value;

    // remove collider so that on next cycle we will reset spaceship
    s_Registry->Remove<ECS::ColliderComponent>(s_spaceShip);

    if (spaceship_lives->m_value == 0) {
      s_state = GameState::LOST;

      // TODO: add LOST TEXT entity
    }

    auto text = s_Registry->Get<ECS::TextComponent>(s_spaceshipLives);
    text->m_value = fmt::format("{} Lives", spaceship_lives->m_value);
  }
}

void DrawGame() {
  s_Registry->UISystem();

  // TEST PLANETS
  // DrawCircleLinesV({300.f, 300.f}, 80.f, BLACK);
  // DrawCircleSectorLines({500.f, 200.f}, 60.f, 0, 180.f, 16, BLACK);
  // DrawSphereWires({100.f, 100.f, 0.f}, 20.f, 16, 4, BLACK);
  // DrawSphereEx({200.f, 200.f, 0.f}, 50.f, 16, 4, BLACK);

  // DrawLineStrip(shipPoints.data(), shipPoints.size(), BLACK);

  s_Registry->RenderSystem();

  // if (GameState::PLAY != s_state) {
  //   // TODO: implement menu/overlay handling
  //   DrawText(TextFormat("%i", s_state), GetScreenWidth() / 2, GetScreenHeight() / 2,
  //   20,
  //            BLACK);
  //
  //   if (GameState::PAUSE == s_state) {
  //     s_Event = SceneEvent::PAUSE;
  //   }
  // }

  // DrawEllipseLines(100.f, 100.f, 20.f, 10.f, BLACK);

  // // DEBUG MINING BEAM
  // auto beam_collider = s_Registry->Get<ECS::ColliderComponent>(s_miningBeam);
  // if (beam_collider) {
  //   DrawText(
  //       TextFormat("%i %f", beam_collider->m_collided_with.has_value(),
  //       s_firingDuration), 0.f, GetScreenHeight() - 30.f, 20, RED);
  // }

  // OVERLAY
  // DrawRectangleLines(5.f, 5.f, GetScreenWidth() - 10.f, GetScreenHeight() - 10.f,
  // BLACK); DrawRectangle(15.f, 15.f, GetScreenWidth() - 30.f, GetScreenHeight() - 30.f,
  // RED);

  // Render Meteors info for DEBUG
  // for (const auto &meteor : s_meteors) {
  //   const auto &health = s_Registry->Get<ECS::HealthComponent>(meteor);
  //   const auto &pos = s_Registry->Get<ECS::PositionComponent>(meteor);
  //   const auto &render = s_Registry->Get<s_Registry->RenderComponent>(meteor);
  //   DrawText(TextFormat("%i", meteor), pos->m_value.x,
  //            pos->m_value.y + render->m_dimensions.x, 20, GREEN);
  //   // DrawRectangle(pos->m_value.x + render->m_dimensions.x,
  //   //               pos->m_value.y + render->m_dimensions.x, health->m_value, 10,
  //   BLACK);
  // }

  // Render Cores info for DEBUG
  // for (const auto &core : s_cores) {
  //   const auto &pos = s_Registry->Get<ECS::PositionComponent>(core);
  //   const auto &render = s_Registry->Get<s_Registry->RenderComponent>(core);
  //   DrawText(TextFormat("%i", core), pos->m_value.x,
  //            pos->m_value.y + render->m_dimensions.x, 20, RED);
  //   // DrawRectangle(pos->m_value.x + render->m_dimensions.x,
  //   //               pos->m_value.y + render->m_dimensions.x, health->m_value, 10,
  //   BLACK);
  // }

  // DEBUG
  // const auto &collider = s_Registry->Get<ECS::ColliderComponent>(s_miningBeam);
  // if (collider) {
  //   DrawText(TextFormat("Col(%i) - w = %f, h = %f", collider->m_shape,
  //                       collider->m_dimensions.x, collider->m_dimensions.y),
  //            10, 50, 20, RED);
  // }
  //
  // DrawText(TextFormat("%i", ECS::colliders.dense.size()), 10, 70, 20, RED);
  // const auto &beam_pos = s_Registry->Get<ECS::PositionComponent>(s_miningBeam);
  // DrawText(TextFormat("Beam: %f, %f", beam_pos->m_value.x, beam_pos->m_value.y), 10,
  // 50,
  //          20, RED);
}

void UnloadGame() {
  s_Event = SceneEvent::NONE;
  s_Registry.reset();
}

void SetGameFocus(bool focus) {
  if (focus) {
    s_state = GameState::PLAY;
    s_Event = SceneEvent::NONE;
  }
  s_IsFocused = focus;
}

SceneEvent OnGameEvent() { return s_Event; }
