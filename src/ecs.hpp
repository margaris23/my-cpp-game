#ifndef ECS_H
#define ECS_H

#include "fmt/core.h"
#include "raylib.h"
#include "sparse-set.hpp"
#include <bitset>
#include <cstdint>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ECS {

using Entity = size_t;

constexpr uint8_t MAX_COMPONENTS = 16;
using ComponentMask = std::bitset<MAX_COMPONENTS>;
extern std::unordered_map<std::type_index, std::bitset<MAX_COMPONENTS>> s_typeToBitSetMap;

template <typename T>
using ComponentGroups = std::unordered_map<ComponentMask, SparseSet::SparseSet<T>>;

// static ComponentGroups groups; // Mask -> SparseSet<Components>

struct PositionComponent {
  Vector2 m_value;
  Entity m_entity;
  explicit PositionComponent(float x, float y) : m_value(Vector2{x, y}) {}
  ~PositionComponent() = default;
  PositionComponent(const PositionComponent &other) = delete;
  PositionComponent(PositionComponent &&other) noexcept = default;
  PositionComponent &operator=(PositionComponent &&rhs) noexcept = default;
};

struct VelocityComponent {
  Vector2 m_value;
  Entity m_entity;
  explicit VelocityComponent(float x, float y) : m_value(Vector2{x, y}) {}
  ~VelocityComponent() = default;
  VelocityComponent(const VelocityComponent &other) = delete;
  VelocityComponent(VelocityComponent &&other) noexcept = default;
  VelocityComponent &operator=(VelocityComponent &&rhs) noexcept = default;
};

struct TextComponent {
  std::string m_value;
  Entity m_entity;
  explicit TextComponent(std::string text) : m_value(text) {}
  ~TextComponent() = default;
  TextComponent(const TextComponent &other) = delete;
  TextComponent(TextComponent &&other) noexcept = default;
  TextComponent &operator=(TextComponent &&rhs) noexcept = default;
};

struct ForceComponent {
  Vector2 m_value;
  Entity m_entity;
  explicit ForceComponent(float x, float y) : m_value(Vector2{x, y}) {}
  ~ForceComponent() = default;
  ForceComponent(const ForceComponent &other) = delete;
  ForceComponent(ForceComponent &&other) noexcept = default;
  ForceComponent &operator=(ForceComponent &&rhs) noexcept = default;
};

struct UIComponent {
  Entity m_selectedEntity;
  Entity m_entity;
  explicit UIComponent(Entity selectedEntity) : m_selectedEntity(selectedEntity) {}
  ~UIComponent() = default;
  UIComponent(const UIComponent &other) = delete;
  UIComponent(UIComponent &&other) noexcept = default;
  UIComponent &operator=(UIComponent &&rhs) noexcept = default;
};

enum class RenderType { REC, CIRCLE };
struct RenderComponent {
  Vector2 m_dimensions; // width/height or radius
  Entity m_entity;
  RenderType m_type;
  explicit RenderComponent(float width, float height)
      : m_dimensions({width, height}), m_type(RenderType::REC) {}
  explicit RenderComponent(float radius)
      : m_dimensions({radius, radius}), m_type(RenderType::CIRCLE) {}
  ~RenderComponent() = default;
  RenderComponent(const RenderComponent &other) = delete;
  RenderComponent(RenderComponent &&other) noexcept = default;
  RenderComponent &operator=(RenderComponent &&rhs) noexcept = default;
};

extern std::vector<Entity> entities;

inline SparseSet::SparseSet<PositionComponent> positions;
inline SparseSet::SparseSet<VelocityComponent> velocities;
inline SparseSet::SparseSet<TextComponent> texts;
inline SparseSet::SparseSet<ForceComponent> forces;
inline SparseSet::SparseSet<RenderComponent> renders;
inline SparseSet::SparseSet<UIComponent> widgets;

Entity CreateEntity();

void Init();
void DeleteEntity(Entity entity);
void RenderSystem();
void ResetSystem();
void PositionSystem();
void UISystem();

// TEMPLATES
template <typename T, typename... Args> bool Add(Entity entity, Args &&...args) {
  T component{std::forward<Args>(args)...};

  if constexpr (std::is_same_v<T, PositionComponent>) {
    return positions.Add(entity, std::move(component));
  } else if constexpr (std::is_same_v<T, VelocityComponent>) {
    return velocities.Add(entity, std::move(component));
  } else if constexpr (std::is_same_v<T, TextComponent>) {
    return texts.Add(entity, std::move(component));
  } else if constexpr (std::is_same_v<T, RenderComponent>) {
    return renders.Add(entity, std::move(component));
  } else if constexpr (std::is_same_v<T, ForceComponent>) {
    return forces.Add(entity, std::move(component));
  } else if constexpr (std::is_same_v<T, UIComponent>) {
    return widgets.Add(entity, std::move(component));
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
  } else if constexpr (std::is_same_v<T, RenderComponent>) {
    renders.Remove(entity);
  } else if constexpr (std::is_same_v<T, ForceComponent>) {
    forces.Remove(entity);
  } else if constexpr (std::is_same_v<T, UIComponent>) {
    widgets.Remove(entity);
  }
}

template <typename T> T *Get(Entity entity) {
  if constexpr (std::is_same_v<T, PositionComponent>) {
    return positions.Get(entity);
  } else if constexpr (std::is_same_v<T, VelocityComponent>) {
    return velocities.Get(entity);
  } else if constexpr (std::is_same_v<T, TextComponent>) {
    return texts.Get(entity);
  } else if constexpr (std::is_same_v<T, RenderComponent>) {
    return renders.Get(entity);
  } else if constexpr (std::is_same_v<T, ForceComponent>) {
    return forces.Get(entity);
  } else if constexpr (std::is_same_v<T, UIComponent>) {
    return widgets.Get(entity);
  }
  return nullptr;
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
