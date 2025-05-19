#include "ecs.hpp"
#include "fmt/core.h"
#include "raylib.h"
#include <algorithm>
#include <functional>
#include <optional>
#include <string>

namespace ECS {

// std::unordered_map<std::type_index, std::bitset<MAX_COMPONENTS>> s_typeToBitSetMap;

namespace {
size_t entityCounter = 0;
}

Registry::~Registry() {
  for (auto it = entities.begin(); it != entities.end();) {
    CleanupEntity(*it);
    it = entities.erase(it);
  }
}

Entity Registry::CreateEntity() {
  size_t index = entityCounter++;
  entities.push_back(index);
  return index;
}

void Registry::CleanupEntity(Entity entity) {
  Remove<PositionComponent>(entity);
  Remove<VelocityComponent>(entity);
  Remove<ColliderComponent>(entity);
  Remove<TextComponent>(entity);
  Remove<RenderComponent>(entity);
  m_renders_sorted = false;
  Remove<ForceComponent>(entity);
  Remove<UIComponent>(entity);
  Remove<HealthComponent>(entity);
  Remove<DmgComponent>(entity);
  Remove<GameStateComponent>(entity);
  Remove<WeaponComponent>(entity);
  // add more ...
}

void Registry::DeleteEntity(Entity entity) {
  const auto &it = std::remove(entities.begin(), entities.end(), entity);
  entities.pop_back();
  CleanupEntity(entity);
}

void Registry::Init() {
  // s_typeToBitSetMap[std::type_index(typeid(TransformComponent))] = 1 << 0;
  // s_typeToBitSetMap[std::type_index(typeid(RenderComponent))] = 1 << 1;
  // s_typeToBitSetMap[std::type_index(typeid(CollisionComponent))] = 1 << 2;
}

struct {
  bool operator()(const RenderComponent &lhs, const RenderComponent &rhs) {
    return lhs.m_priority < rhs.m_priority;
  }
} compareLayer;

void Registry::RenderSystem() {
  if (!m_renders_sorted) {
    // Sort by Layer
    std::sort(renders.dense.begin(), renders.dense.end(), compareLayer);
    // Update sparse indexing
    for (int i = 0; i < renders.dense.size(); i++) {
      renders.sparse[renders.dense[i].m_entity] = i;
    }
    m_renders_sorted = true;
  }

  // SHAPES
  for (auto &render : renders.dense) {
    const auto pos = positions.Get(render.m_entity);
    if (render.IsVisible()) {
      if (Shape::RECTANGLE == render.m_shape) {
        DrawRectangleLines(pos->m_value.x, pos->m_value.y, render.m_dimensions.x,
                           render.m_dimensions.y, render.m_color);
        // DrawRectangle(pos->m_value.x + 1.f, pos->m_value.y + 1.f,
        //               render.m_dimensions.x - 2.f, render.m_dimensions.y - 2.f,
        //               RAYWHITE);
      } else if (Shape::ELLIPSE == render.m_shape) {
        DrawEllipseLines(pos->m_value.x, pos->m_value.y, render.m_dimensions.x,
                         render.m_dimensions.y, render.m_color);
      } else if (Shape::CIRCLE == render.m_shape) {
        DrawCircle(pos->m_value.x, pos->m_value.y, render.m_dimensions.x, render.m_color);
        // DrawCircle(pos->m_value.x, pos->m_value.y, 10.f, render.m_color);
      }
      // TODO: add more...
    }
  }

  // TEXTS
  for (const auto &text : texts.dense) {
    const auto pos = positions.Get(text.m_entity);
    DrawText(text.m_value.c_str(), pos->m_value.x, pos->m_value.y, 20, text.m_color);
  }
}

void Registry::PositionSystem() {
  float screen_width = GetScreenWidth();
  float screen_height = GetScreenHeight();

  for (auto &pos : positions.dense) {
    const auto force = forces.Get(pos.m_entity);
    auto velocity = velocities.Get(pos.m_entity);
    auto weapon = weapons.Get(pos.m_entity);

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
    } else if (weapon) {
      const auto shooterPos = positions.Get(weapon->m_shooter);
      // weapon position: custom offsets for now
      pos.m_value.x = shooterPos->m_value.x - 5.f;
      pos.m_value.y = shooterPos->m_value.y + 8.f;
    }

    // ideally, size should be included
    if (pos.m_value.x < -30.f) {
      pos.m_value.x = screen_width;
    } else if (pos.m_value.x > screen_width + 30.f) {
      pos.m_value.x = 0;
    }

    if (pos.m_value.y < -30.f) {
      pos.m_value.y = screen_height;
    } else if (pos.m_value.y > screen_height + 30.f) {
      pos.m_value.y = 0;
    }
  }
}

bool HandleCollision(ColliderComponent &colA, ColliderComponent &colB,
                     const PositionComponent *posA, const PositionComponent *posB) {
  const auto &dimA = colA.m_dimensions;

  bool collides =
      CheckCollisionCircleRec(posB->m_value, colB.m_dimensions.x,
                              {posA->m_value.x, posA->m_value.y, dimA.x, dimA.y});

  if (collides) {
    colA.m_collided_with = colB.m_entity;
    colB.m_collided_with = colA.m_entity;
    return true;
  }

  return false;
}

void Registry::CollisionDetectionSystem() {
  const auto &positionComponents = positions.dense;
  auto &colliderComps = colliders.dense;

  for (size_t indexA = 0; indexA < colliderComps.size() - 1; indexA++) {
    for (size_t indexB = indexA + 1; indexB < colliders.dense.size(); indexB++) {
      auto posA = positions.Get(colliderComps[indexA].m_entity);
      auto posB = positions.Get(colliderComps[indexB].m_entity);

      // TODO: Need to address this later
      if (!posB || !posA) {
        continue;
      }

      // Optimizations:
      // ELLIPSE is expected to be our Spaceship or its MiningBeam (weapon),
      // Circles are expected to be our Meteors
      // Circles do not expect to collide each other so,
      // Rectangle is expected to collide with only 1 Circle
      if (Shape::RECTANGLE == colliderComps[indexA].m_shape &&
          Shape::CIRCLE == colliderComps[indexB].m_shape) {
        if (HandleCollision(colliderComps[indexA], colliderComps[indexB], posA, posB)) {
          break;
        }
      } else if (Shape::RECTANGLE == colliderComps[indexB].m_shape &&
                 Shape::CIRCLE == colliderComps[indexA].m_shape) {
        if (HandleCollision(colliderComps[indexB], colliderComps[indexA], posB, posA)) {
          break;
        }
      }
    }
  }
}

void Registry::CollisionResolutionSystem() {
  for (auto &collider : colliders.dense) {
    if (collider.m_collided_with.has_value()) {
      auto health = healths.Get(collider.m_entity);
      auto dmg = dmgs.Get(collider.m_collided_with.value());
      if (health && dmg) {
        health->m_value -= dmg->m_value;

        // constraint
        if (health->m_value < 0) {
          health->m_value = 0;
        }

        // Need to add Bounce physics
        // Need to know: pos and dir ???
        // auto pos = positions.Get(collider.m_entity);
        // Add<ForceComponent>(collider.m_entity, -10.f, -10.f);
      }
      collider.m_collided_with.reset();
    }
  }
}

void Registry::UISystem() {
  for (auto &widget : widgets.dense) {
    auto widgetPos = positions.Get(widget.m_entity);

    // Handle selected widget logic ... Like a focus
    if (widget.m_selectedEntity.has_value()) {
      auto selectedWidgetPos = positions.Get(widget.m_selectedEntity.value());

      if (widgetPos && selectedWidgetPos) {
        widgetPos->m_value.x = selectedWidgetPos->m_value.x;
        widgetPos->m_value.y = selectedWidgetPos->m_value.y;
      }
      // ECS::Add<ECS::PositionComponent>(s_SelectedBtn, H_CenterText("New Game") -
      // padding, 100.f - padding); ECS::Add<ECS::eenderComponent>(s_SelectedBtn,
      // textWidth + 2.f * padding, 20.f + 2 * padding);  // Rectangle
    } else if (UIElement::BAR == widget.m_type) {
      const auto state = stateValues.Get(widget.m_entity);
      if (state) {
        DrawRectangle(widgetPos->m_value.x, widgetPos->m_value.y, state->m_value * 10, 20,
                      BLACK);
      }
    }
    // else if (UIElement::TEXT == widget.m_type) {
    //   const auto text = texts.Get(widget.m_entity);
    //   if (text) {
    //     DrawText(text->m_value.c_str(), widgetPos->m_value.x, widgetPos->m_value.y, 20,
    //     BLACK);
    //   }
    // }
  }
}

void Registry::ResetSystem() { forces.Reset(); }

} // namespace ECS
