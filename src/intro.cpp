#include "collision-component.h"
#include "ecs.h"
#include "raylib.h"
#include "scenes.h"
#include <cstdint>
#include <memory>
#include <vector>

#include <cassert>

static int s_counter;

static ECS::Entity title;

// TODO: remove archetypes and use sparseset
// static struct RenderArchetype s_renderArch;
static std::vector<std::unique_ptr<CollisionComponent>> collisions;

const int MAX_RENDER_COMPONENTS = 5;
// constexpr uint16_t MAX_ENTITIES = 5;

//-------------------- NEW ECS EXPERIEMENTATION--------------------

void LoadIntro() {
  float screen_cw = GetScreenWidth() / 2.f;
  float screen_ch = GetScreenHeight() / 2.f;

  // minor perf improve
  // if (s_renderArch.m_render.capacity() <= MAX_RENDER_COMPONENTS) {
  //   s_renderArch.m_render.reserve(MAX_RENDER_COMPONENTS);
  // }
  if (collisions.capacity() <= MAX_RENDER_COMPONENTS) {
    collisions.reserve(MAX_RENDER_COMPONENTS);
  }

  // FOR RENDER
  // s_renderArch.m_render.push_back(std::make_unique<TextComponent>("Minoids"));
  // TransformComponent title_transform{Vector2{screen_cw, screen_ch}};
  // // title_transform.SetRotation(0.05);
  // // Is title_transform copied??? -- Let's use move
  // s_renderArch.m_transform.emplace_back(std::move(title_transform));
  // ++s_counter;

  // FOR COLLISIONS
  // collisions.emplace_back(
  //     std::make_unique<RectangleCollisionComponent>(Rectangle{10, 10, 50,
  //     50}));
  // collisions.emplace_back(
  //     std::make_unique<RectangleCollisionComponent>(Rectangle{60, 60, 40,
  //     40}));

  // TESTING SPARSESET
  // SparseSet::SparseSet collisionSet;
  // collisionSet.Add(4);
  // collisionSet.Add(6);
  // collisionSet.Add(3);
  // collisionSet.Remove(4);
  // assert(collisionSet.Get(6) == 6);
  // assert(collisionSet.Get(4) == SparseSet::EMPTY);

  // positions.reserve(MAX_ENTITIES);
  // velocities.reserve(MAX_ENTITIES);
  //
  // positions.emplace_back(Vector2{30.f, 30.f});
  // velocities.emplace_back(Vector2{0.01f, 0.01f});

  title = ECS::CreateEntity();
  ECS::Add<ECS::TextComponent>(title, "Minoids");
  ECS::Add<ECS::PositionComponent>(title, 30.f, 30.f);
}

void UpdateIntro(float delta) {
  // Order of Systems
  // 1. Input System
  // 2. AI System
  // 3. Physics
  // ...
  // N-1. Render
  // N. Audio

  // auto start = std::chrono::high_resolution_clock::now();

  // Input System
  if (IsKeyDown(KEY_RIGHT)) {
    ECS::Add<ECS::ForceComponent>(title, 5.f, 0.f);
  } else if (IsKeyDown(KEY_LEFT)) {
    ECS::Add<ECS::ForceComponent>(title, -5.f, 0.f);
  } else if (IsKeyDown(KEY_UP)) {
    ECS::Add<ECS::ForceComponent>(title, 0.f, -5.f);
  } else if (IsKeyDown(KEY_DOWN)) {
    ECS::Add<ECS::ForceComponent>(title, 0.f, 5.f);
  } else {
    ECS::Remove<ECS::ForceComponent>(title);
  }

  // transform on 2 vectors
  // std::transform(positions.begin(), positions.end(), velocities.begin(),
  //                positions.begin(), moveSystem);

  // std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(
  //                  std::chrono::high_resolution_clock::now() - start)
  //                  .count()
  //           << "\n";

  // ECS::ForEach<TransformComponent, CollisionComponent>(moveSystem);

  // TODO: maybe I need to move colliders too
  // MovementSystem(delta, s_renderArch.m_transform);
  // CollisionSystem(collisions);

  // FIXME: strange that I use m_render
  // for (auto &component : s_renderArch.m_render) {
  //   component->Update(delta);
  // }

  ECS::PositionSystem();
}

void DrawIntro() {
  // RenderSystem(s_renderArch);
  ECS::RenderSystem();
  // ECS::ResetSystem();
}

void UnloadIntro() {
  // s_renderArch.m_render.clear();
  // s_renderArch.m_transform.clear();
}
