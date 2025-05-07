#ifndef ECS_H
#define ECS_H

#include "raylib.h"
#include "sparse-set.h"
#include <bitset>
#include <cstdint>
#include <functional>
#include <iostream>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ECS {

using Entity = size_t;

constexpr uint8_t MAX_COMPONENTS = 16;
using ComponentMask = std::bitset<MAX_COMPONENTS>;
extern std::unordered_map<std::type_index, std::bitset<MAX_COMPONENTS>>
    s_typeToBitSetMap;

template <typename T>
using ComponentGroups =
    std::unordered_map<ComponentMask, SparseSet::SparseSet<T>>;

// static ComponentGroups groups; // Mask -> SparseSet<Components>

struct PositionComponent {
  Vector2 m_value;
  Entity entity;
  PositionComponent(float x, float y) : m_value(Vector2{x, y}) {}
};

struct VelocityComponent {
  Vector2 m_value;
  Entity entity;
  VelocityComponent(float x, float y) : m_value(Vector2{x, y}) {}
};

struct TextComponent {
  std::string m_value;
  Entity entity;
  TextComponent(std::string text) : m_value(text) {}
};

struct ForceComponent {
  Vector2 m_value;
  Entity entity;
  ForceComponent(float x, float y) : m_value(Vector2{x, y}) {}
};

extern std::vector<Entity> entities;

inline SparseSet::SparseSet<PositionComponent> positions;
inline SparseSet::SparseSet<VelocityComponent> velocities;
inline SparseSet::SparseSet<TextComponent> texts;
inline SparseSet::SparseSet<ForceComponent> forces;

void Init();
Entity CreateEntity();
void DeleteEntity(Entity entity);
void RenderSystem();
void ResetSystem();
void PositionSystem();

// TEMPLATES
template <typename T, typename... Args>
bool Add(Entity entity, Args &&...args) {
  T component{std::forward<Args>(args)...};

  if constexpr (std::is_same_v<T, PositionComponent>) {
    return positions.Add(entity, component);
  } else if constexpr (std::is_same_v<T, VelocityComponent>) {
    return velocities.Add(entity, component);
  } else if constexpr (std::is_same_v<T, TextComponent>) {
    return texts.Add(entity, component);
  } else if constexpr (std::is_same_v<T, ForceComponent>) {
    return forces.Add(entity, component);
  }

  return false;
}

template <typename T> void Remove(Entity entity) {
  if constexpr (std::is_same_v<T, PositionComponent>) {
    positions.Remove(entity);
  } else if constexpr (std::is_same_v<T, VelocityComponent>) {
    velocities.Remove(entity);
  } else if constexpr (std::is_same_v<T, TextComponent>) {
    texts.Remove(entity);
  } else if constexpr (std::is_same_v<T, ForceComponent>) {
    forces.Remove(entity);
  }
}

template <typename... C> void RegisterComponentGroup() {
  std::bitset<MAX_COMPONENTS> bitset;
  /// Get bitset from list of ClassNames
  ((bitset |= s_typeToBitSetMap[std::type_index(typeid(C))]), ...);

  // TODO: do not know yet...
}

template <typename... C, typename Func> void ForEach(Func system) {
  std::bitset<MAX_COMPONENTS> targetBitSet;
  /// Get bitset from list of ClassNames
  ((targetBitSet |= s_typeToBitSetMap[std::type_index(typeid(C))]), ...);

  // for (const auto &[mask, sparseset] : groups) {
  //   if ((mask & targetBitSet) == targetBitSet) {
  //     // DO STUFF...
  //   }
  // }
}

} // namespace ECS

#endif
