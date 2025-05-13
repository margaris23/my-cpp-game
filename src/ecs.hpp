#ifndef ECS_H
#define ECS_H

#include "fmt/core.h"
#include "raylib.h"
#include "sparse-set.hpp"
#include <bitset>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace ECS {

using Entity = size_t;

// To be used later: to check whether an entity has a component faster
constexpr uint8_t MAX_COMPONENTS = 16;
using ComponentMask = std::bitset<MAX_COMPONENTS>;
extern std::unordered_map<std::type_index, std::bitset<MAX_COMPONENTS>> s_typeToBitSetMap;

template <typename T>
using ComponentGroups = std::unordered_map<ComponentMask, SparseSet<T>>;

enum class Shape {
  RECTANGLE,
  CIRCLE,
};
enum class UIElement {
  TEXT,
  BAR,
};

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

struct ColliderComponent {
  Vector2 m_dimensions; // width/height or radius based on shape
  Entity m_entity;
  Shape m_shape;
  std::optional<Entity> m_collided_with;
  explicit ColliderComponent(float width, float height)
      : m_dimensions({width, height}), m_shape(Shape::RECTANGLE) {}
  explicit ColliderComponent(float radius)
      : m_dimensions({radius, radius}), m_shape(Shape::CIRCLE) {}
  ~ColliderComponent() = default;
  ColliderComponent(const ColliderComponent &other) = delete;
  ColliderComponent(ColliderComponent &&other) noexcept = default;
  ColliderComponent &operator=(ColliderComponent &&rhs) noexcept = default;
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
  UIElement m_type;
  Entity m_entity;
  std::optional<Entity> m_selectedEntity;
  explicit UIComponent(UIElement type) : m_type(type) {}
  explicit UIComponent(UIElement type, Entity selectedEntity)
      : m_type(type), m_selectedEntity(selectedEntity) {}
  ~UIComponent() = default;
  UIComponent(const UIComponent &other) = delete;
  UIComponent(UIComponent &&other) noexcept = default;
  UIComponent &operator=(UIComponent &&rhs) noexcept = default;
};

// Looks the same as Collider, however it will possibly be enhanced
// RenderComponent is for primitive drawables
struct RenderComponent {
  Vector2 m_dimensions; // width/height or radius based on shape
  Entity m_entity;
  Shape m_shape;
  explicit RenderComponent(float width, float height)
      : m_dimensions({width, height}), m_shape(Shape::RECTANGLE) {}
  explicit RenderComponent(float radius)
      : m_dimensions({radius, radius}), m_shape(Shape::CIRCLE) {}
  ~RenderComponent() = default;
  RenderComponent(const RenderComponent &other) = delete;
  RenderComponent(RenderComponent &&other) noexcept = default;
  RenderComponent &operator=(RenderComponent &&rhs) noexcept = default;
};

struct HealthComponent {
  float m_value;
  Entity m_entity;
  explicit HealthComponent(float health) : m_value(health) {}
  ~HealthComponent() = default;
  HealthComponent(const HealthComponent &other) = delete;
  HealthComponent(HealthComponent &&other) noexcept = default;
  HealthComponent &operator=(HealthComponent &&rhs) noexcept = default;
};

struct DmgComponent {
  float m_value;
  Entity m_entity;
  explicit DmgComponent(float health) : m_value(health) {}
  ~DmgComponent() = default;
  DmgComponent(const DmgComponent &other) = delete;
  DmgComponent(DmgComponent &&other) noexcept = default;
  DmgComponent &operator=(DmgComponent &&rhs) noexcept = default;
};

using GameStateValue = float; // std::variant<float, int>;

struct GameStateComponent {
  GameStateValue m_value;
  Entity m_entity;
  explicit GameStateComponent(GameStateValue value) : m_value(value) {}
  ~GameStateComponent() = default;
  GameStateComponent(const GameStateComponent &other) = delete;
  GameStateComponent(GameStateComponent &&other) noexcept = default;
  GameStateComponent &operator=(GameStateComponent &&rhs) noexcept = default;
};

extern std::vector<Entity> entities;

inline SparseSet<PositionComponent> positions;
inline SparseSet<VelocityComponent> velocities;
inline SparseSet<ColliderComponent> colliders;
inline SparseSet<TextComponent> texts;
inline SparseSet<ForceComponent> forces;
inline SparseSet<RenderComponent> renders;
inline SparseSet<UIComponent> widgets;
inline SparseSet<HealthComponent> healths;
inline SparseSet<DmgComponent> dmgs;
inline SparseSet<GameStateComponent> stateValues;

Entity CreateEntity();

void Init();
void DeleteEntity(Entity entity);
void RenderSystem();
void ResetSystem();
void PositionSystem();
void UISystem();
void CollisionDetectionSystem();
void CollisionResolutionSystem();

// TEMPLATES
template <typename T, typename... Args> bool Add(Entity entity, Args &&...args) {
  T component{std::forward<Args>(args)...};

  if constexpr (std::is_same_v<T, PositionComponent>) {
    return positions.Add(entity, std::move(component));
  } else if constexpr (std::is_same_v<T, VelocityComponent>) {
    return velocities.Add(entity, std::move(component));
  } else if constexpr (std::is_same_v<T, ColliderComponent>) {
    return colliders.Add(entity, std::move(component));
  } else if constexpr (std::is_same_v<T, TextComponent>) {
    return texts.Add(entity, std::move(component));
  } else if constexpr (std::is_same_v<T, RenderComponent>) {
    return renders.Add(entity, std::move(component));
  } else if constexpr (std::is_same_v<T, ForceComponent>) {
    return forces.Add(entity, std::move(component));
  } else if constexpr (std::is_same_v<T, UIComponent>) {
    return widgets.Add(entity, std::move(component));
  } else if constexpr (std::is_same_v<T, HealthComponent>) {
    return healths.Add(entity, std::move(component));
  } else if constexpr (std::is_same_v<T, DmgComponent>) {
    return dmgs.Add(entity, std::move(component));
  } else if constexpr (std::is_same_v<T, GameStateComponent>) {
    return stateValues.Add(entity, std::move(component));
  }

  return false;
}

template <typename T> void Remove(Entity entity) {
  if constexpr (std::is_same_v<T, PositionComponent>) {
    positions.Remove(entity);
  } else if constexpr (std::is_same_v<T, VelocityComponent>) {
    velocities.Remove(entity);
  } else if constexpr (std::is_same_v<T, ColliderComponent>) {
    colliders.Remove(entity);
  } else if constexpr (std::is_same_v<T, TextComponent>) {
    texts.Remove(entity);
  } else if constexpr (std::is_same_v<T, RenderComponent>) {
    renders.Remove(entity);
  } else if constexpr (std::is_same_v<T, ForceComponent>) {
    forces.Remove(entity);
  } else if constexpr (std::is_same_v<T, UIComponent>) {
    widgets.Remove(entity);
  } else if constexpr (std::is_same_v<T, HealthComponent>) {
    healths.Remove(entity);
  } else if constexpr (std::is_same_v<T, DmgComponent>) {
    dmgs.Remove(entity);
  } else if constexpr (std::is_same_v<T, GameStateComponent>) {
    stateValues.Remove(entity);
  }
}

template <typename T> T *Get(Entity entity) {
  if constexpr (std::is_same_v<T, PositionComponent>) {
    return positions.Get(entity);
  } else if constexpr (std::is_same_v<T, VelocityComponent>) {
    return velocities.Get(entity);
  } else if constexpr (std::is_same_v<T, ColliderComponent>) {
    return colliders.Get(entity);
  } else if constexpr (std::is_same_v<T, TextComponent>) {
    return texts.Get(entity);
  } else if constexpr (std::is_same_v<T, RenderComponent>) {
    return renders.Get(entity);
  } else if constexpr (std::is_same_v<T, ForceComponent>) {
    return forces.Get(entity);
  } else if constexpr (std::is_same_v<T, UIComponent>) {
    return widgets.Get(entity);
  } else if constexpr (std::is_same_v<T, HealthComponent>) {
    return healths.Get(entity);
  } else if constexpr (std::is_same_v<T, DmgComponent>) {
    return dmgs.Get(entity);
  } else if constexpr (std::is_same_v<T, GameStateComponent>) {
    return stateValues.Get(entity);
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
