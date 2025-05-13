#include "ecs.hpp"
#include "fmt/core.h"
#include "raylib.h"
#include <functional>
#include <optional>
#include <string>

namespace ECS {

std::unordered_map<std::type_index, std::bitset<MAX_COMPONENTS>> s_typeToBitSetMap;

std::vector<Entity> entities;
// SparseSet::SparseSet<PositionComponent> positions;
// SparseSet::SparseSet<VelocityComponent> velocities;
// SparseSet::SparseSet<TextComponent> texts;

namespace {
size_t entityCounter = 0;
}

Entity CreateEntity() {
  size_t index = entityCounter++;
  entities.push_back(index);
  return index;
}

void DeleteEntity(Entity entity) {
  fmt::print("\tRemoving Entity {} ... ", entity);
  entities.erase(std::remove(entities.begin(), entities.end(), entity));
  fmt::println(" #entities left: {}", entities.size());

  Remove<PositionComponent>(entity);
  Remove<VelocityComponent>(entity);
  Remove<ColliderComponent>(entity);
  Remove<TextComponent>(entity);
  Remove<RenderComponent>(entity);
  Remove<ForceComponent>(entity);
  Remove<UIComponent>(entity);
  Remove<HealthComponent>(entity);
  Remove<DmgComponent>(entity);
  Remove<GameStateComponent>(entity);
  // add more ...
}

void Init() {
  // s_typeToBitSetMap[std::type_index(typeid(TransformComponent))] = 1 << 0;
  // s_typeToBitSetMap[std::type_index(typeid(RenderComponent))] = 1 << 1;
  // s_typeToBitSetMap[std::type_index(typeid(CollisionComponent))] = 1 << 2;
}

void RenderSystem() {
  const auto &textComponents = texts.dense;
  const auto &renderComponents = renders.dense;

  for (const auto &pos : positions.dense) {
    // TEXTS
    const auto text = texts.Get(pos.m_entity);
    if (text) {
      DrawText(text->m_value.c_str(), pos.m_value.x, pos.m_value.y, 20, BLACK);
    }
    // SHAPES
    const auto render = renders.Get(pos.m_entity);
    if (render) {
      if (Shape::RECTANGLE == render->m_shape) {
        DrawRectangleLines(pos.m_value.x, pos.m_value.y, render->m_dimensions.x,
                           render->m_dimensions.y, BLACK);
      } else if (Shape::CIRCLE == render->m_shape) {
        DrawCircle(pos.m_value.x, pos.m_value.y, render->m_dimensions.x, BLACK);
      }
      // TODO: add more...
    }
  }
}

void PositionSystem() {
  for (auto &pos : positions.dense) {
    const auto force = forces.Get(pos.m_entity);
    auto velocity = velocities.Get(pos.m_entity);

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

void CollisionDetectionSystem() {
  const auto &positionComponents = positions.dense;
  auto &colliderComps = colliders.dense;

  for (size_t indexA = 0; indexA < colliderComps.size(); indexA++) {
    for (size_t indexB = indexA + 1; indexB < colliders.dense.size(); indexB++) {
      auto posA = positions.Get(colliderComps[indexA].m_entity);
      auto posB = positions.Get(colliderComps[indexB].m_entity);

      // Optimizations:
      // Rectangle is expected to be our Spaceship,
      // Circles are expected to be our Meteors
      // Circles do not expect to collide each other so,
      // Rectangle is expected to collide with only 1 Circle
      if (Shape::RECTANGLE == colliderComps[indexA].m_shape) {
        if (Shape::CIRCLE == colliderComps[indexB].m_shape) {
          const auto &dimA = colliderComps[indexA].m_dimensions;

          bool collides =
              CheckCollisionCircleRec(posB->m_value, colliderComps[indexB].m_dimensions.x,
                                      {posA->m_value.x, posA->m_value.y, dimA.x, dimA.y});

          if (collides) {
            // fmt::println("{} collides with {}", colliderComps[indexA].m_entity,
            //              colliderComps[indexB].m_entity);
            colliderComps[indexA].m_collided_with = colliderComps[indexB].m_entity;
            colliderComps[indexB].m_collided_with = colliderComps[indexA].m_entity;

            return; // see optimizations comment a few lines above
          }
        }
      }
    }
  }
}

void CollisionResolutionSystem() {
  for (auto &collider : colliders.dense) {
    if (collider.m_collided_with.has_value()) {
      auto health = healths.Get(collider.m_entity);
      auto dmg = dmgs.Get(collider.m_collided_with.value());
      if (health && dmg) {
        health->m_value -= dmg->m_value;
        collider.m_collided_with.reset();

        // Need to add Bounce physics
        // Need to know: pos and dir ???
        // auto pos = positions.Get(collider.m_entity);
        // Add<ForceComponent>(collider.m_entity, -10.f, -10.f);
      }
    }
  }
}

void UISystem() {
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
      // padding, 100.f - padding); ECS::Add<ECS::RenderComponent>(s_SelectedBtn,
      // textWidth + 2.f * padding, 20.f + 2 * padding);  // Rectangle
    } else if (UIElement::BAR == widget.m_type) {
      const auto state = stateValues.Get(widget.m_entity);
      if (state) {
        DrawRectangle(widgetPos->m_value.x, widgetPos->m_value.y, state->m_value * 10, 20,
                      BLACK);
      }
    }
  }
}

void ResetSystem() { forces.Reset(); }

} // namespace ECS
