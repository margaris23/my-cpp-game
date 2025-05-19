#ifndef ECS_H
#define ECS_H

#include "fmt/core.h"
#include "raylib.h"
#include "sparse-set.hpp"
#include <cstdint>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace ECS {

using Entity = size_t;

// To be used later: to check whether an entity has a component faster
// constexpr uint8_t MAX_COMPONENTS = 16;
// using ComponentMask = std::bitset<MAX_COMPONENTS>;
// extern std::unordered_map<std::type_index, std::bitset<MAX_COMPONENTS>> s_typeToBitSetMap;

// template <typename T>
// using ComponentGroups = std::unordered_map<ComponentMask, SparseSet<T>>;

enum class Shape {
  RECTANGLE,
  CIRCLE,

  // Only for RenderComponents
  RECTANGLE_SOLID,
  ELLIPSE,
};
enum class UIElement {
  TEXT,
  BAR,
};
enum class Layer : uint8_t { SUB, GROUND, SKY };

// static ComponentGroups groups; // Mask -> SparseSet<Components>

struct PositionComponent {
  Vector2 value;
  Entity entity;
  explicit PositionComponent(float x, float y) : value(Vector2{x, y}) {}
  ~PositionComponent() = default;
  PositionComponent(const PositionComponent &other) = delete;
  PositionComponent(PositionComponent &&other) noexcept = default;
  PositionComponent &operator=(PositionComponent &&rhs) noexcept = default;
};

struct VelocityComponent {
  Vector2 value;
  Entity entity;
  explicit VelocityComponent(float x, float y) : value(Vector2{x, y}) {}
  ~VelocityComponent() = default;
  VelocityComponent(const VelocityComponent &other) = delete;
  VelocityComponent(VelocityComponent &&other) noexcept = default;
  VelocityComponent &operator=(VelocityComponent &&rhs) noexcept = default;
};

struct ColliderComponent {
  Vector2 dimensions; // width/height or radius based on shape
  Entity entity;
  Shape shape;
  std::optional<Entity> collided_with;
  explicit ColliderComponent(float width, float height)
      : dimensions({width, height}), shape(Shape::RECTANGLE) {}
  explicit ColliderComponent(float radius) : dimensions({radius, radius}), shape(Shape::CIRCLE) {}
  ~ColliderComponent() = default;
  ColliderComponent(const ColliderComponent &other) = delete;
  ColliderComponent(ColliderComponent &&other) noexcept = default;
  ColliderComponent &operator=(ColliderComponent &&rhs) noexcept = default;
};

// TODO: merge with UIComponent
struct TextComponent {
  std::string value;
  Color color;
  Entity entity;
  explicit TextComponent(std::string_view text, Color color = BLACK) : value(text), color(color) {}
  ~TextComponent() = default;
  TextComponent(const TextComponent &other) = delete;
  TextComponent(TextComponent &&other) noexcept = default;
  TextComponent &operator=(TextComponent &&rhs) noexcept = default;
};

struct ForceComponent {
  Vector2 value;
  Entity entity;
  explicit ForceComponent(float x, float y) : value(Vector2{x, y}) {}
  ~ForceComponent() = default;
  ForceComponent(const ForceComponent &other) = delete;
  ForceComponent(ForceComponent &&other) noexcept = default;
  ForceComponent &operator=(ForceComponent &&rhs) noexcept = default;
};

struct UIComponent {
  UIElement type;
  Entity entity;
  std::optional<Entity> selectedEntity;
  explicit UIComponent(UIElement type) : type(type) {}
  explicit UIComponent(UIElement type, Entity selectedEntity)
      : type(type), selectedEntity(selectedEntity) {}
  ~UIComponent() = default;
  UIComponent(const UIComponent &other) = delete;
  UIComponent(UIComponent &&other) noexcept = default;
  UIComponent &operator=(UIComponent &&rhs) noexcept = default;
};

// RenderComponent is for primitive drawables
struct RenderComponent {
  Color color;
  Vector2 dimensions; // width/height or radius based on shape
  Entity entity;
  Shape shape;
  Layer priority; // for layering
  RenderComponent(Layer priority, Shape shape, Color color, float width, float height)
      : priority(priority), color(color), dimensions({width, height}), shape(shape) {}
  RenderComponent(Layer priority, Shape shape, Color color, float radius)
      : priority(priority), color(color), dimensions({radius, radius}), shape(shape) {}
  ~RenderComponent() = default;
  RenderComponent(const RenderComponent &other) = delete;
  RenderComponent(RenderComponent &&other) noexcept = default;
  RenderComponent &operator=(RenderComponent &&rhs) noexcept = default;
  bool IsVisible() {
    return Shape::CIRCLE == shape && dimensions.x > 0.f || dimensions.x > 0.f && dimensions.y > 0.f;
  }
};

struct HealthComponent {
  float value;
  Entity entity;
  explicit HealthComponent(float health) : value(health) {}
  ~HealthComponent() = default;
  HealthComponent(const HealthComponent &other) = delete;
  HealthComponent(HealthComponent &&other) noexcept = default;
  HealthComponent &operator=(HealthComponent &&rhs) noexcept = default;
};

struct DmgComponent {
  float value;
  Entity entity;
  explicit DmgComponent(float health) : value(health) {}
  ~DmgComponent() = default;
  DmgComponent(const DmgComponent &other) = delete;
  DmgComponent(DmgComponent &&other) noexcept = default;
  DmgComponent &operator=(DmgComponent &&rhs) noexcept = default;
};

struct WeaponComponent {
  Entity shooter;
  float max_length;
  Entity entity;
  explicit WeaponComponent(Entity shooter, float max_length)
      : shooter(shooter), max_length(max_length) {}
  ~WeaponComponent() = default;
  WeaponComponent(const WeaponComponent &other) = delete;
  WeaponComponent(WeaponComponent &&other) noexcept = default;
  WeaponComponent &operator=(WeaponComponent &&rhs) noexcept = default;
};

using GameStateValue = float; // std::variant<float, int>;

struct GameStateComponent {
  GameStateValue value;
  Entity entity;
  explicit GameStateComponent(GameStateValue value) : value(value) {}
  ~GameStateComponent() = default;
  GameStateComponent(const GameStateComponent &other) = delete;
  GameStateComponent(GameStateComponent &&other) noexcept = default;
  GameStateComponent &operator=(GameStateComponent &&rhs) noexcept = default;
};

// Used for entities isolation i.e per scene
class Registry {
public:
  Registry() = default;
  ~Registry();

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
      return m_positions.Add(entity, std::move(component));
    } else if constexpr (std::is_same_v<T, VelocityComponent>) {
      return m_velocities.Add(entity, std::move(component));
    } else if constexpr (std::is_same_v<T, ColliderComponent>) {
      return m_colliders.Add(entity, std::move(component));
    } else if constexpr (std::is_same_v<T, TextComponent>) {
      return m_texts.Add(entity, std::move(component));
    } else if constexpr (std::is_same_v<T, RenderComponent>) {
      m_renders_sorted = false;
      return m_renders.Add(entity, std::move(component));
    } else if constexpr (std::is_same_v<T, ForceComponent>) {
      return m_forces.Add(entity, std::move(component));
    } else if constexpr (std::is_same_v<T, UIComponent>) {
      return m_widgets.Add(entity, std::move(component));
    } else if constexpr (std::is_same_v<T, HealthComponent>) {
      return m_healths.Add(entity, std::move(component));
    } else if constexpr (std::is_same_v<T, DmgComponent>) {
      return m_dmgs.Add(entity, std::move(component));
    } else if constexpr (std::is_same_v<T, GameStateComponent>) {
      return m_stateValues.Add(entity, std::move(component));
    } else if constexpr (std::is_same_v<T, WeaponComponent>) {
      return m_weapons.Add(entity, std::move(component));
    }

    return false;
  }

  template <typename T> void Remove(Entity entity) {
    if constexpr (std::is_same_v<T, PositionComponent>) {
      m_positions.Remove(entity);
    } else if constexpr (std::is_same_v<T, VelocityComponent>) {
      m_velocities.Remove(entity);
    } else if constexpr (std::is_same_v<T, ColliderComponent>) {
      m_colliders.Remove(entity);
    } else if constexpr (std::is_same_v<T, TextComponent>) {
      m_texts.Remove(entity);
    } else if constexpr (std::is_same_v<T, RenderComponent>) {
      m_renders.Remove(entity);
    } else if constexpr (std::is_same_v<T, ForceComponent>) {
      m_forces.Remove(entity);
    } else if constexpr (std::is_same_v<T, UIComponent>) {
      m_widgets.Remove(entity);
    } else if constexpr (std::is_same_v<T, HealthComponent>) {
      m_healths.Remove(entity);
    } else if constexpr (std::is_same_v<T, DmgComponent>) {
      m_dmgs.Remove(entity);
    } else if constexpr (std::is_same_v<T, GameStateComponent>) {
      m_stateValues.Remove(entity);
    } else if constexpr (std::is_same_v<T, WeaponComponent>) {
      m_weapons.Remove(entity);
    }
  }

  template <typename T> T *Get(Entity entity) {
    if constexpr (std::is_same_v<T, PositionComponent>) {
      return m_positions.Get(entity);
    } else if constexpr (std::is_same_v<T, VelocityComponent>) {
      return m_velocities.Get(entity);
    } else if constexpr (std::is_same_v<T, ColliderComponent>) {
      return m_colliders.Get(entity);
    } else if constexpr (std::is_same_v<T, TextComponent>) {
      return m_texts.Get(entity);
    } else if constexpr (std::is_same_v<T, RenderComponent>) {
      return m_renders.Get(entity);
    } else if constexpr (std::is_same_v<T, ForceComponent>) {
      return m_forces.Get(entity);
    } else if constexpr (std::is_same_v<T, UIComponent>) {
      return m_widgets.Get(entity);
    } else if constexpr (std::is_same_v<T, HealthComponent>) {
      return m_healths.Get(entity);
    } else if constexpr (std::is_same_v<T, DmgComponent>) {
      return m_dmgs.Get(entity);
    } else if constexpr (std::is_same_v<T, GameStateComponent>) {
      return m_stateValues.Get(entity);
    } else if constexpr (std::is_same_v<T, WeaponComponent>) {
      return m_weapons.Get(entity);
    }

    return nullptr;
  }

private:
  size_t m_entityCounter = 0;
  std::vector<Entity> m_entities;

  SparseSet<PositionComponent> m_positions;
  SparseSet<VelocityComponent> m_velocities;
  SparseSet<ColliderComponent> m_colliders;
  SparseSet<TextComponent> m_texts;
  SparseSet<ForceComponent> m_forces;
  SparseSet<RenderComponent> m_renders;
  SparseSet<UIComponent> m_widgets;
  SparseSet<HealthComponent> m_healths;
  SparseSet<DmgComponent> m_dmgs;
  SparseSet<GameStateComponent> m_stateValues;
  SparseSet<WeaponComponent> m_weapons;

  void CleanupEntity(Entity entity);

  bool m_renders_sorted;
};

// template <typename... C> void RegisterComponentGroup() {
//   std::bitset<MAX_COMPONENTS> bitset;
//   /// Get bitset from list of ClassNames
//   ((bitset |= s_typeToBitSetMap[std::type_index(typeid(C))]), ...);
//
//   // TODO: do not know yet...
// }
//
// template <typename... C, typename Func> void ForEach(Func system) {
//   std::bitset<MAX_COMPONENTS> targetBitSet;
//   /// Get bitset from list of ClassNames
//   ((targetBitSet |= s_typeToBitSetMap[std::type_index(typeid(C))]), ...);
//
//   // for (const auto &[mask, sparseset] : groups) {
//   //   if ((mask & targetBitSet) == targetBitSet) {
//   //     // DO STUFF...
//   //   }
//   // }
// }

} // namespace ECS

#endif
