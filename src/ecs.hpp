#ifndef ECS_H
#define ECS_H

#include "FastNoiseLite.h"
#include "fmt/core.h"
#include "fmt/format.h"
#include "raylib.h"
#include "sparse-set.hpp"
#include <atomic>
#include <cstdint>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace ECS {

using Entity = size_t;

// To be used later: to check whether an entity has a component faster
// constexpr uint8_t MAX_COMPONENTS = 16;
// using ComponentMask = std::bitset<MAX_COMPONENTS>;
// extern std::unordered_map<std::type_index, std::bitset<MAX_COMPONENTS>> s_typeToBitSetMap;

// template <typename T>
// using ComponentGroups = std::unordered_map<ComponentMask, SparseSet<T>>;

namespace {
static constexpr float NOISE_SCALE = 0.1f;
static inline FastNoiseLite s_noise;
static inline std::random_device s_rd;
} // namespace

class ThreadSafeIdGenerator {
public:
  // Get next unique ID
  static size_t getNextId() { return counter.fetch_add(1, std::memory_order_relaxed); }

  // Get current ID without incrementing
  static size_t getCurrentId() { return counter.load(std::memory_order_relaxed); }

  // Reset the counter (use with caution)
  static void reset(size_t value = 0) { counter.store(value, std::memory_order_relaxed); }

private:
  static std::atomic<size_t> counter;
};

enum class Shape {
  RECTANGLE,
  CIRCLE,
  LINE,

  // Only for RenderComponents
  RECTANGLE_SOLID,
  ELLIPSE,
  METEOR,
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
  Color color;
  // std::optional<Entity> selectedEntity;
  explicit UIComponent(UIElement type) : type(type) {}
  explicit UIComponent(UIElement type, Color color) : type(type), color(color) {}
  // explicit UIComponent(UIElement type, Entity selectedEntity)
  //     : type(type), selectedEntity(selectedEntity) {}
  ~UIComponent() = default;
  UIComponent(const UIComponent &other) = delete;
  UIComponent(UIComponent &&other) noexcept = default;
  UIComponent &operator=(UIComponent &&rhs) noexcept = default;
};

// RenderComponent is for primitive drawables
struct RenderComponent {
  Color color;
  Vector2 dimensions; // width/height or radius or radius/noise_amp based on shape
  Entity entity;
  Shape shape;
  Layer priority; // for layering
  std::vector<float> noise_values;

  // Contructors - TODO: constraint shapes
  RenderComponent(Layer priority, Shape shape /*Rectangle | Line */, Color color, float width,
                  float height)
      : priority(priority), color(color), dimensions({width, height}), shape(shape) {}

  RenderComponent(Layer priority, Shape shape /* Circle */, Color color, float radius)
      : priority(priority), color(color), dimensions({radius, radius}), shape(shape) {}

  RenderComponent(Layer priority, Shape shape /* Meteor */, Color color, float radius,
                  float noise_amplitude, int point_count)
      : priority(priority), color(color), dimensions({radius, noise_amplitude}), shape(shape) {

    s_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    s_noise.SetFrequency(NOISE_SCALE);
    s_noise.SetSeed(s_rd());

    noise_values.reserve(point_count);
    for (int i = 0; i < point_count; i++) {
      float noiseValue = s_noise.GetNoise<float>(i, 0.0f);
      noise_values.push_back(noiseValue * noise_amplitude);
    }
  }

  ~RenderComponent() = default;
  RenderComponent(const RenderComponent &other) = delete;
  RenderComponent(RenderComponent &&other) noexcept = default;
  RenderComponent &operator=(RenderComponent &&rhs) noexcept = default;

  bool IsVisible() {
    if (Shape::CIRCLE == shape || Shape::METEOR == shape) {
      return dimensions.x > 0.f;
    } else if (Shape::RECTANGLE == shape) {
      return dimensions.x > 0.f && dimensions.y > 0.f;
    }
    // LINE
    return dimensions.x > 0.f || dimensions.y > 0.f;
  }
};

struct SpriteComponent {
  Texture2D texture;
  Layer priority;
  float scale;
  Entity entity;
  bool m_owns_texture; // RAII for the texture

  explicit SpriteComponent(Layer priority, std::string filename)
      : priority(priority), scale(1.f) {
    texture = LoadTexture(filename.c_str());
    m_owns_texture = true;
  }
  explicit SpriteComponent(Layer priority, std::string filename, float scale)
      : priority(priority), scale(scale) {
    texture = LoadTexture(filename.c_str());
    m_owns_texture = true;
  }

  ~SpriteComponent() {
    if (m_owns_texture) {
      UnloadTexture(texture);
    }
  }

  SpriteComponent(const SpriteComponent &other) = delete;
  SpriteComponent &operator=(const SpriteComponent &other) = delete;

  // safe MOVE Semantics due to Texture
  SpriteComponent(SpriteComponent &&other) noexcept
      : texture(other.texture), m_owns_texture(true), priority(std::move(other.priority)),
        scale(std::move(other.scale)), entity(std::move(other.entity)) {
    other.m_owns_texture = false;
  }

  SpriteComponent &operator=(SpriteComponent &&rhs) noexcept {
    if (this != &rhs) {
      if (m_owns_texture) {
        UnloadTexture(texture);
      } else {
        rhs.m_owns_texture = false;
        m_owns_texture = true;
      }
      texture = rhs.texture;
      priority = std::move(rhs.priority);
      entity = std::move(rhs.entity);
      priority = std::move(rhs.priority);
    }

    return *this;
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
  float firingDuration;
  bool isFiring;
  explicit WeaponComponent(Entity shooter, float max_length)
      : shooter(shooter), max_length(max_length) {}
  ~WeaponComponent() = default;
  WeaponComponent(const WeaponComponent &other) = delete;
  WeaponComponent(WeaponComponent &&other) noexcept = default;
  WeaponComponent &operator=(WeaponComponent &&rhs) noexcept = default;
};

using GameStateValue = std::variant<float, int>;

struct GameStateComponent {
  GameStateValue value;
  Entity entity;
  explicit GameStateComponent(GameStateValue value) : value(value) {}
  ~GameStateComponent() = default;
  GameStateComponent(const GameStateComponent &other) = delete;
  GameStateComponent(GameStateComponent &&other) noexcept = default;
  GameStateComponent &operator=(GameStateComponent &&rhs) noexcept = default;
};

struct InputComponent {
  float push_force_step;
  float max_push_force;
  Entity entity;
  float push_force_step_half;
  explicit InputComponent(float push_force_step, float max_push_force)
      : push_force_step(push_force_step), max_push_force(max_push_force),
        push_force_step_half(push_force_step / 2.f) {}
  ~InputComponent() = default;
  InputComponent(const InputComponent &other) = delete;
  InputComponent(InputComponent &&other) noexcept = default;
  InputComponent &operator=(InputComponent &&rhs) noexcept = default;
};

// Particle Emitter
struct EmitterComponent {
  int rate;                // in frames
  float particle_lifetime; // in frames (used in Health i.e float)
  Shape particle_shape;
  Vector2 particle_velocity;
  Entity entity;
  bool active = false;
  int m_timer = 0; // in frames
  explicit EmitterComponent(int emission_rate, int particle_lifetime, Shape particle_shape,
                            Vector2 particle_velocity)
      : rate(emission_rate), particle_lifetime(particle_lifetime), particle_shape(particle_shape),
        particle_velocity(particle_velocity) {}
  ~EmitterComponent() = default;
  EmitterComponent(const EmitterComponent &other) = delete;
  EmitterComponent(EmitterComponent &&other) noexcept = default;
  EmitterComponent &operator=(EmitterComponent &&rhs) noexcept = default;
};

struct ParticleComponent {
  Entity emitter;
  Entity entity;
  bool active = true; // TODO: avoid
  explicit ParticleComponent(Entity emitter) : emitter(emitter) {}
  ~ParticleComponent() = default;
  ParticleComponent(const ParticleComponent &other) = delete;
  ParticleComponent(ParticleComponent &&other) noexcept = default;
  ParticleComponent &operator=(ParticleComponent &&rhs) noexcept = default;
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
  void InputSystem();
  void CollisionDetectionSystem();
  void CollisionResolutionSystem();
  void ParticleSystem();

  void Debug();

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
    } else if constexpr (std::is_same_v<T, SpriteComponent>) {
      return m_sprites.Add(entity, std::move(component));
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
    } else if constexpr (std::is_same_v<T, InputComponent>) {
      return m_inputs.Add(entity, std::move(component));
    } else if constexpr (std::is_same_v<T, EmitterComponent>) {
      return m_emitters.Add(entity, std::move(component));
    } else if constexpr (std::is_same_v<T, ParticleComponent>) {
      return m_particles.Add(entity, std::move(component));
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
    } else if constexpr (std::is_same_v<T, SpriteComponent>) {
      m_sprites.Remove(entity);
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
    } else if constexpr (std::is_same_v<T, InputComponent>) {
      m_inputs.Remove(entity);
    } else if constexpr (std::is_same_v<T, EmitterComponent>) {
      m_emitters.Remove(entity);
    } else if constexpr (std::is_same_v<T, ParticleComponent>) {
      m_particles.Remove(entity);
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
    } else if constexpr (std::is_same_v<T, SpriteComponent>) {
      return m_sprites.Get(entity);
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
    } else if constexpr (std::is_same_v<T, InputComponent>) {
      return m_inputs.Get(entity);
    } else if constexpr (std::is_same_v<T, EmitterComponent>) {
      return m_emitters.Get(entity);
    } else if constexpr (std::is_same_v<T, ParticleComponent>) {
      return m_particles.Get(entity);
    }

    return nullptr;
  }

private:
  // size_t m_entityCounter = 0;

  // TODO: use Tombstoned vector for O(1) delete and element reusability
  std::vector<Entity> m_entities;

  SparseSet<PositionComponent> m_positions;
  SparseSet<VelocityComponent> m_velocities;
  SparseSet<ColliderComponent> m_colliders;
  SparseSet<TextComponent> m_texts;
  SparseSet<ForceComponent> m_forces;
  SparseSet<RenderComponent> m_renders;
  SparseSet<SpriteComponent> m_sprites;
  SparseSet<UIComponent> m_widgets;
  SparseSet<HealthComponent> m_healths;
  SparseSet<DmgComponent> m_dmgs;
  SparseSet<GameStateComponent> m_stateValues;
  SparseSet<WeaponComponent> m_weapons;
  SparseSet<InputComponent> m_inputs;
  SparseSet<EmitterComponent> m_emitters;
  SparseSet<ParticleComponent> m_particles;

  void CleanupEntity(Entity entity);

  bool m_renders_sorted;

  // ENTROPY
  std::random_device rd;
  std::mt19937 gen;
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
