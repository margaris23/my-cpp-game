#include "collision-component.h"
#include "collision-system.h"
#include "movement-system.h"
#include "raylib.h"
#include "rectangle-collision-component.h"
#include "render-system.h"
#include "scenes.h"
#include "sparse-set.h"
#include "text-component.h"
#include "transform-component.h"
#include <bitset>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include <cassert>

static int s_counter;

constexpr uint8_t MAX_COMPONENTS = 16;
using ComponentMask = std::bitset<MAX_COMPONENTS>;

// Mask -> SparseSet<Components>
std::unordered_map<ComponentMask, SparseSet::SparseSet> groups;

static struct RenderArchetype s_renderArch;
static std::vector<std::unique_ptr<CollisionComponent>> collisions;

const int MAX_RENDER_COMPONENTS = 5;

void LoadIntro() {
  float screen_cw = GetScreenWidth() / 2.f;
  float screen_ch = GetScreenHeight() / 2.f;

  // minor perf improve
  if (s_renderArch.m_render.capacity() <= MAX_RENDER_COMPONENTS) {
    s_renderArch.m_render.reserve(MAX_RENDER_COMPONENTS);
  }
  if (collisions.capacity() <= MAX_RENDER_COMPONENTS) {
    collisions.reserve(MAX_RENDER_COMPONENTS);
  }

  // FOR RENDER
  s_renderArch.m_render.push_back(std::make_unique<TextComponent>("Minoids"));
  TransformComponent title_transform{Vector2{screen_cw, screen_ch}};
  // title_transform.SetRotation(0.05);
  // Is title_transform copied??? -- Let's use move
  s_renderArch.m_transform.emplace_back(std::move(title_transform));
  ++s_counter;

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
}

void UpdateIntro(float delta) {
  // TODO: maybe I need to move colliders too
  MovementSystem(delta, s_renderArch.m_transform);
  CollisionSystem(collisions);
  // FIXME: strange that I use m_render
  // for (auto &component : s_renderArch.m_render) {
  //   component->Update(delta);
  // }
}

void DrawIntro() { RenderSystem(s_renderArch); }

void UnloadIntro() {
  s_renderArch.m_render.clear();
  s_renderArch.m_transform.clear();
}
