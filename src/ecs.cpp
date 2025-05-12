#include "ecs.hpp"
#include "fmt/core.h"
#include "raylib.h"
#include <functional>
#include <string>

namespace ECS {

std::unordered_map<std::type_index, std::bitset<MAX_COMPONENTS>> s_typeToBitSetMap;

std::vector<Entity> entities;
// SparseSet::SparseSet<PositionComponent> positions;
// SparseSet::SparseSet<VelocityComponent> velocities;
// SparseSet::SparseSet<TextComponent> texts;

Entity CreateEntity() {
  size_t index = entities.size();
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

  // ForEach<PositionComponent, TextComponent>(fn);

  // DEBUG DATA
  // const auto &forcesComponents = forces.dense;
  // const auto &velocityComponents = velocities.dense;
  //
  // DrawText(std::to_string(forcesComponents.size()).c_str(), 10, 100, 20,
  // BLUE); DrawText(std::to_string(velocityComponents.size()).c_str(), 10, 140,
  // 20,
  //          BLUE);
  // DrawText(std::to_string(positions.dense.size()).c_str(), 10, 180, 20,
  //          BLUE);
}

void PositionSystem() {
  for (auto &pos : positions.dense) {
    // DrawText(std::to_string(pos.m_entity).c_str(), 200, 100, 20, BLUE);

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

void UISystem() {
  for (auto &widget : widgets.dense) {
    auto widgetPos = positions.Get(widget.m_entity);
    auto selectedWidgetPos = positions.Get(widget.m_selectedEntity);

    if (widgetPos && selectedWidgetPos) {
      widgetPos->m_value.x = selectedWidgetPos->m_value.x;
      widgetPos->m_value.y = selectedWidgetPos->m_value.y;
    }
    // ECS::Add<ECS::PositionComponent>(s_SelectedBtn, H_CenterText("New Game") - padding,
    // 100.f - padding); ECS::Add<ECS::RenderComponent>(s_SelectedBtn, textWidth + 2.f *
    // padding, 20.f + 2 * padding);  // Rectangle
  }
}

void CollisionDetectionSystem() {
  const auto &positionComponents = positions.dense;
  const auto &colliderComponents = colliders.dense;

  for (size_t indexA = 0; indexA < colliderComponents.size(); indexA++) {
    for (size_t indexB = indexA + 1; indexB < colliders.dense.size(); indexB++) {
      // if (colliderA.m_entity == colliderB.m_entity) {
      //   continue;
      // }

      const auto posA = positions.Get(colliderComponents[indexA].m_entity);
      const auto posB = positions.Get(colliderComponents[indexB].m_entity);

      if (Shape::RECTANGLE == colliderComponents[indexA].m_shape) {
        // fmt::println("Spaceship ({},{}) -> Meteor {} ({},{})", posA->m_value.x,
        //              posA->m_value.y, colliderComponents[indexB].m_entity,
        //              posB->m_value.x, posB->m_value.y);

        if (Shape::CIRCLE == colliderComponents[indexB].m_shape) {
          const auto &dimA = colliderComponents[indexA].m_dimensions;

          bool collides = CheckCollisionCircleRec(
              posB->m_value, colliderComponents[indexB].m_dimensions.x,
              {posA->m_value.x, posA->m_value.y, dimA.x, dimA.y});

          if (collides)
            fmt::println("{} collides with {}", colliderComponents[indexA].m_entity,
                         colliderComponents[indexB].m_entity);
        }
      }
    }
  }
}

void ResetSystem() { forces.Reset(); }

} // namespace ECS
