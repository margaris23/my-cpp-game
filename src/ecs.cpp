#include "ecs.h"
#include "raylib.h"
#include "sparse-set.h"
#include <algorithm>
#include <string>

namespace ECS {

std::unordered_map<std::type_index, std::bitset<MAX_COMPONENTS>>
    s_typeToBitSetMap;

std::vector<Entity> entities;
// SparseSet::SparseSet<PositionComponent> positions;
// SparseSet::SparseSet<VelocityComponent> velocities;
// SparseSet::SparseSet<TextComponent> texts;

Entity CreateEntity() {
  size_t index = entities.size();
  entities.push_back(index);
  return index;
}

void Init() {
  // s_typeToBitSetMap[std::type_index(typeid(TransformComponent))] = 1 << 0;
  // s_typeToBitSetMap[std::type_index(typeid(RenderComponent))] = 1 << 1;
  // s_typeToBitSetMap[std::type_index(typeid(CollisionComponent))] = 1 << 2;
}

void RenderSystem() {
  // we expect texts to have position - WRONG???
  const auto &textComponents = texts.GetDense();
  for (const auto &pos : positions.GetDense()) {
    const auto text = texts.Get(pos.entity);
    if (text) {
      DrawText(text->m_value.c_str(), pos.m_value.x, pos.m_value.y, 20, BLACK);
    }
  }

  // ForEach<PositionComponent, TextComponent>(fn);

  // DEBUG DATA
  // const auto &forcesComponents = forces.GetDense();
  // const auto &velocityComponents = velocities.GetDense();
  //
  // DrawText(std::to_string(forcesComponents.size()).c_str(), 10, 100, 20, BLUE);
  // DrawText(std::to_string(velocityComponents.size()).c_str(), 10, 140, 20,
  //          BLUE);
  // DrawText(std::to_string(positions.GetDense().size()).c_str(), 10, 180, 20,
  //          BLUE);
}

void PositionSystem() {
  for (auto &pos : positions.GetDense()) {
    // DrawText(std::to_string(pos.entity).c_str(), 200, 100, 20, BLUE);

    const auto force = forces.Get(pos.entity);
    auto velocity = velocities.Get(pos.entity);

    if (force) {
      if (velocity) {
        velocity->m_value.x += force->m_value.x;
        velocity->m_value.y += force->m_value.y;
      } else {
        pos.m_value.x += force->m_value.x;
        pos.m_value.y += force->m_value.y;
      }
    } else if (velocity) {
      pos.m_value.x += velocity->m_value.x;
      pos.m_value.y += velocity->m_value.y;
    }
  }
}

void ResetSystem() { forces.Reset(); }

} // namespace ECS
