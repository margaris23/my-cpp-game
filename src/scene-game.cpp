#include "ecs.hpp"
#include "fmt/core.h"
#include "game.hpp"
#include "raylib.h"
#include "scenes.hpp"
#include <random>
#include <variant>
#include <vector>

constexpr static float METEORS_WINDOW_PADDING = 50.f;
constexpr static Vector2 SPACESHIP_SIZE{61.f, 29.f}; // matches sprite dimensions
constexpr static float MAX_PUSH_FORCE = 5.f;
constexpr static float PUSH_FORCE_STEP = .2f;
constexpr static float PUSH_FORCE_STEP_HALF = PUSH_FORCE_STEP / 2.f;

constexpr static int METEOR_POINT_COUNT = 80;
constexpr static float METEOR_NOISE_AMPLITUDE = 8.1f;

static SceneEvent s_Event = SceneEvent::NONE;
static bool s_IsFocused = false;

static std::unique_ptr<ECS::Registry> s_Registry;

using ECS::PositionComponent, ECS::RenderComponent, ECS::TextComponent, ECS::VelocityComponent,
    ECS::GameStateComponent, ECS::UIComponent, ECS::ForceComponent, ECS::DmgComponent,
    ECS::ColliderComponent, ECS::WeaponComponent, ECS::HealthComponent, ECS::SpriteComponent,
    ECS::EmitterComponent, ECS::UIElement, ECS::Entity, ECS::Layer, ECS::Shape;

// ENTROPY
static std::random_device rd;
static std::mt19937 gen(rd());

// STATE
enum class GameState { PLAY, PAUSE, WON, LOST };
static bool s_isFiring = false;
static float s_firingDuration = 0.f;
static GameState s_state = GameState::PLAY;

// ENTITIES
static Entity s_spaceshipHealth;
static Entity s_spaceshipLives;
static Entity s_spaceshipFuel;
static Entity s_spaceShip;
static Entity s_miningBeam;
static Entity s_beamParticle;
static Entity s_score;
static Entity s_coresCount;
static Entity s_level;
static std::vector<Entity> s_meteors;
static std::vector<Entity> s_cores;

constexpr static size_t FRAME_MAX_COUNTER = 3600;
static size_t s_frame = 0;

extern Game::Game g_Game;

void LoadGame() {
  float screen_cw = GetScreenWidth() / 2.f;
  float screen_ch = GetScreenHeight() / 2.f;
  float meteors_offset = Game::MAX_METEOR_SIZE + METEORS_WINDOW_PADDING;

  s_Registry = std::make_unique<ECS::Registry>();
  s_Registry->Init();

  // Randomizers
  std::uniform_real_distribution<float> rnd_x(meteors_offset, (float)GetScreenWidth() - meteors_offset);
  std::uniform_real_distribution<float> rnd_y(meteors_offset, (float)GetScreenHeight() - meteors_offset);
  std::uniform_int_distribution<int> rnd_size(g_Game.meteors.min_meteor_size,
                                                 g_Game.meteors.max_meteor_size);
  std::uniform_real_distribution<float> rnd_velocity(g_Game.meteors.meteor_min_velocity,
                                                     g_Game.meteors.meteor_max_velocity);

  // Our Hero
  s_spaceShip = s_Registry->CreateEntity();
  s_Registry->Add<PositionComponent>(s_spaceShip, screen_cw, screen_ch);
  // s_Registry->Add<RenderComponent>(s_spaceShip, Layer::GROUND, Shape::ELLIPSE, BLACK,
  //                                  SPACESHIP_SIZE.x, SPACESHIP_SIZE.y);
  s_Registry->Add<SpriteComponent>(s_spaceShip, Layer::GROUND, "ufo.png");
  s_Registry->Add<ColliderComponent>(s_spaceShip, SPACESHIP_SIZE.x, SPACESHIP_SIZE.y);
  s_Registry->Add<DmgComponent>(s_spaceShip, 0.1f);
  s_Registry->Add<HealthComponent>(s_spaceShip, g_Game.health);
  s_Registry->Add<ForceComponent>(s_spaceShip, 0.f, 0.f); // Initialize empty force
  s_Registry->Add<ECS::InputComponent>(s_spaceShip, PUSH_FORCE_STEP, MAX_PUSH_FORCE);

  // Spaceship's Mining Beam
  s_miningBeam = s_Registry->CreateEntity();
  s_Registry->Add<PositionComponent>(s_miningBeam, 0.f, 0.f);
  s_Registry->Add<WeaponComponent>(s_miningBeam, s_spaceShip, Game::WEAPON_MAX_DISTANCE);
  s_Registry->Add<DmgComponent>(s_miningBeam, Game::WEAPON_DMG);
  // s_Registry->Add<EmitterComponent>(s_miningBeam, 15 /* emmission rate */,
  //                                   50 /* particle lifetime */, Shape::LINE,
  //                                   Vector2{0.f, 1.f} /* particle velocity */);

  // Generate Meteors
  s_meteors = std::vector<Entity>();
  s_meteors.reserve(g_Game.meteors.count);
  s_cores = std::vector<Entity>();
  s_cores.reserve(s_meteors.capacity());

  for (int i = 0; i < s_meteors.capacity(); i++) {
    float posX = rnd_x(gen);
    float posY = rnd_y(gen);
    float velX = rnd_velocity(gen);
    float velY = rnd_velocity(gen);
    float radius = (float)rnd_size(gen);

    // Meteor Core
    s_cores.push_back(s_Registry->CreateEntity());
    Entity core = s_cores.back();
    s_Registry->Add<PositionComponent>(core, posX, posY);
    s_Registry->Add<VelocityComponent>(core, velX, velY);
    s_Registry->Add<RenderComponent>(core, Layer::SUB, Shape::CIRCLE, DARKGREEN, 0.f);
    s_Registry->Add<HealthComponent>(core, Game::METEOR_CORE_HEALTH);
    // NO Collider until core revealed

    // Main Meteor
    s_meteors.push_back(s_Registry->CreateEntity());
    Entity meteor = s_meteors.back();
    s_Registry->Add<PositionComponent>(meteor, posX, posY);
    // TODO: bigger asteroids should move slower
    s_Registry->Add<VelocityComponent>(meteor, velX, velY);
    s_Registry->Add<RenderComponent>(meteor, Layer::GROUND, Shape::METEOR, BLACK, radius,
                                     METEOR_NOISE_AMPLITUDE, METEOR_POINT_COUNT);
    s_Registry->Add<ColliderComponent>(meteor, radius);
    s_Registry->Add<HealthComponent>(meteor, radius); // bigger means more health
    s_Registry->Add<DmgComponent>(meteor, Game::METEOR_DMG);
  }

  // State: Spaceship health (UI Entity)
  s_spaceshipHealth = s_Registry->CreateEntity();
  s_Registry->Add<GameStateComponent>(s_spaceshipHealth, g_Game.health);
  s_Registry->Add<UIComponent>(s_spaceshipHealth, UIElement::BAR, DARKGREEN);
  s_Registry->Add<PositionComponent>(s_spaceshipHealth, 10.f, 10.f);

  // Spaceship Lives (UI Entity)
  s_spaceshipLives = s_Registry->CreateEntity();
  s_Registry->Add<GameStateComponent>(s_spaceshipLives, g_Game.lives);
  s_Registry->Add<TextComponent>(s_spaceshipLives, fmt::format("{} Lives", g_Game.lives), MAROON);
  s_Registry->Add<PositionComponent>(s_spaceshipLives,
                                     GetScreenWidth() - MeasureText(" Lives", 20) - 20.f, 10.f);

  // Fuel (UI Entity)
  // s_spaceshipFuel = s_Registry->CreateEntity();
  // s_Registry->Add<GameStateComponent>(s_spaceshipFuel, g_Game.fuel);
  // s_Registry->Add<UIComponent>(s_spaceshipFuel, UIElement::BAR, BLACK);
  // s_Registry->Add<PositionComponent>(s_spaceshipFuel, 10.f, 30.f);

  // Scores (UI Entity)
  s_score = s_Registry->CreateEntity();
  s_Registry->Add<GameStateComponent>(s_score, g_Game.score);
  s_Registry->Add<TextComponent>(s_score, fmt::format("{}", g_Game.score), BROWN);
  s_Registry->Add<PositionComponent>(s_score, screen_cw - 10.f, 10.f);

  // # Cores (UI Entity)
  s_coresCount = s_Registry->CreateEntity();
  s_Registry->Add<GameStateComponent>(s_coresCount, g_Game.total_cores);
  s_Registry->Add<TextComponent>(s_coresCount, fmt::format("{} Cores", g_Game.total_cores),
                                 DARKGREEN);
  s_Registry->Add<PositionComponent>(s_coresCount, screen_cw * 1.5f, 10.f);

  // Level (UI Entity)
  s_level = s_Registry->CreateEntity();
  s_Registry->Add<GameStateComponent>(s_level, g_Game.level);
  s_Registry->Add<TextComponent>(s_level, fmt::format("L {}", g_Game.level), DARKBLUE);
  s_Registry->Add<PositionComponent>(s_level, 140.f, 10.f);

  s_IsFocused = true;
}

void UpdateGame(float delta) {
  // check if round is won or lost
  if (Game::IsGameWon()) {
    s_state = GameState::WON;
    s_Event = SceneEvent::NEXT;
    return;
  } else {
    auto lives = s_Registry->Get<GameStateComponent>(s_spaceshipLives);
    if (Game::IsGameLost()) {
      s_state = GameState::LOST;
      s_Event = SceneEvent::NEXT;
      return;
    }
  }

  // HANDLE INPUT only when in FOCUS
  if (s_IsFocused) {
    if (IsKeyPressed(KEY_ESCAPE)) {
      s_state = GameState::PAUSE;
      s_Event = SceneEvent::PAUSE;
      return;
    }

    // Input
    s_Registry->InputSystem();
  }

  // Handle Fuel consumption
  // if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN)) {
  //   Game::LoseFuel();
  //   auto fuel = s_Registry->Get<GameStateComponent>(s_spaceshipFuel);
  //   fuel->value = g_Game.fuel;
  // }

  if (GameState::PAUSE == s_state) {
    return;
  }

  s_frame = (s_frame + 1) % FRAME_MAX_COUNTER;

  // Reset spaceship in case collider was removed
  if (!s_Registry->Get<ColliderComponent>(s_spaceShip)) {
    auto health = s_Registry->Get<HealthComponent>(s_spaceShip);
    Game::ResetSpaceship();
    health->value = g_Game.health;
    s_Registry->Add<ColliderComponent>(s_spaceShip, SPACESHIP_SIZE.x, SPACESHIP_SIZE.y);
  }

  // Position + Collision
  s_Registry->PositionSystem();
  s_Registry->CollisionDetectionSystem();

  // Particles after Input + Collision (Not sure if this is ok???)
  // We may have particle generation from weapons, collisions, but what about positioning
  s_Registry->ParticleSystem();

  // SCORE SYSTEM
  // CORES
  auto cores_count = s_Registry->Get<GameStateComponent>(s_coresCount);
  for (const auto &core : s_cores) {
    const auto &collider = s_Registry->Get<ColliderComponent>(core);
    if (collider && collider->collided_with.has_value()) {
      Game::GatherCore();
      cores_count->value = g_Game.total_cores;
    }
  }
  auto coresCount_text = s_Registry->Get<TextComponent>(s_coresCount);
  coresCount_text->value = fmt::format("{} Cores", g_Game.total_cores);

  // SCORE
  auto score = s_Registry->Get<GameStateComponent>(s_score);
  for (const auto &meteor : s_meteors) {
    const auto &collider = s_Registry->Get<ColliderComponent>(meteor);
    if (collider && collider->collided_with.has_value()) {
      // Let's earn 1 point for every hit for now....
      Game::MineMeteor();
      score->value = g_Game.score;
    }
  }
  auto score_text = s_Registry->Get<TextComponent>(s_score);
  std::visit([score_text](auto &&value) { score_text->value = fmt::format("{}", value); },
             score->value);

  // Collision Resolution
  s_Registry->CollisionResolutionSystem();

  // Kill s_cores with zero health
  for (auto core_it = s_cores.begin(); core_it != s_cores.end();) {
    const auto core_health = s_Registry->Get<HealthComponent>(*core_it);
    if (core_health && core_health->value == 0) {
      s_Registry->DeleteEntity(*core_it);
      core_it = s_cores.erase(core_it);
    } else {
      ++core_it;
    }
  }

  // Meteors Updates
  for (auto it = s_meteors.begin(); it != s_meteors.end();) {
    // Kill s_meteors with zero health
    const auto meteor_health = s_Registry->Get<HealthComponent>(*it);
    if (meteor_health->value < 10.f) {
      // every core is considered to be before to a meteor
      const Entity core = *it - 1;

      s_Registry->DeleteEntity(*it);
      it = s_meteors.erase(it);

      // Enable core
      auto collider = s_Registry->Get<ColliderComponent>(core);
      if (collider == nullptr) {
        // fmt::println("Activating {} for {}", core, meteor);
        // activate core
        auto core_velocity = s_Registry->Get<VelocityComponent>(core);
        if (core_velocity) {
          core_velocity->value.x *= -1;
          if (core_velocity->value.y < 0) {
            core_velocity->value.y *= -1;
          }
        }
        auto core_pos = s_Registry->Get<PositionComponent>(core);
        if (core_pos) {
          core_pos->value.x += 10.f;
          core_pos->value.y += 10.f;
        }
        s_Registry->Add<ColliderComponent>(core, Game::METEOR_CORE_SIZE);

        // Update render so that it is displayed
        auto render = s_Registry->Get<RenderComponent>(core);
        render->dimensions.x = Game::METEOR_CORE_SIZE;
      }
      continue;
    }

    // Update collider + size based on health
    auto render = s_Registry->Get<RenderComponent>(*it);
    render->dimensions.x = meteor_health->value;
    auto collider = s_Registry->Get<ColliderComponent>(*it);
    collider->dimensions.x = meteor_health->value;

    ++it;
  }

  // UISystem-UPDATE ????
  // Sync state with components i.e Spaceship.health -> SpaceShipHealth.state
  const auto health = s_Registry->Get<HealthComponent>(s_spaceShip);
  auto state = s_Registry->Get<GameStateComponent>(s_spaceshipHealth);
  std::visit([health](auto &value) { Game::DmgSpaceship(value - health->value); }, state->value);
  state->value = g_Game.health;

  auto text = s_Registry->Get<TextComponent>(s_spaceshipLives);
  text->value = fmt::format("{} Lives", g_Game.lives);

  auto spaceship_lives = s_Registry->Get<GameStateComponent>(s_spaceshipLives);
  std::visit(
      [](auto &&value) {
        if (g_Game.lives != value) {
          // remove collider so that on next cycle we will reset spaceship
          s_Registry->Remove<ColliderComponent>(s_spaceShip);
        }
      },
      spaceship_lives->value);
  spaceship_lives->value = g_Game.lives;
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

  // DEBUG
  // s_Registry->Debug();

  // DrawEllipseLines(100.f, 100.f, 20.f, 10.f, BLACK);

  // // DEBUG MINING BEAM
  // auto beam_collider = s_Registry->Get<ColliderComponent>(s_miningBeam);
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
  //   const auto &health = s_Registry->Get<HealthComponent>(meteor);
  //   const auto &pos = s_Registry->Get<PositionComponent>(meteor);
  //   const auto &render = s_Registry->Get<s_Registry->RenderComponent>(meteor);
  //   DrawText(TextFormat("%i", meteor), pos->value.x,
  //            pos->value.y + render->m_dimensions.x, 20, GREEN);
  //   // DrawRectangle(pos->value.x + render->m_dimensions.x,
  //   //               pos->value.y + render->m_dimensions.x, health->value, 10,
  //   BLACK);
  // }

  // Render Cores info for DEBUG
  // for (const auto &core : s_cores) {
  //   const auto &pos = s_Registry->Get<PositionComponent>(core);
  //   const auto &render = s_Registry->Get<s_Registry->RenderComponent>(core);
  //   DrawText(TextFormat("%i", core), pos->value.x,
  //            pos->value.y + render->m_dimensions.x, 20, RED);
  //   // DrawRectangle(pos->value.x + render->m_dimensions.x,
  //   //               pos->value.y + render->m_dimensions.x, health->value, 10,
  //   BLACK);
  // }

  // DEBUG
  // const auto &collider = s_Registry->Get<ColliderComponent>(s_miningBeam);
  // if (collider) {
  //   DrawText(TextFormat("Col(%i) - w = %f, h = %f", collider->m_shape,
  //                       collider->m_dimensions.x, collider->m_dimensions.y),
  //            10, 50, 20, RED);
  // }
  //
  // DrawText(TextFormat("%i", colliders.dense.size()), 10, 70, 20, RED);
  // const auto &beam_pos = s_Registry->Get<PositionComponent>(s_miningBeam);
  // DrawText(TextFormat("Beam: %f, %f", beam_pos->value.x, beam_pos->value.y), 10,
  // 50,
  //          20, RED);
}

void UnloadGame() {
  s_meteors.clear();
  s_cores.clear();
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
