#include "ecs.hpp"
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
  entities.erase(std::remove(entities.begin(), entities.end(), entity));
  Remove<PositionComponent>(entity);
  Remove<VelocityComponent>(entity);
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
      if (RenderType::REC == render->m_type) {
        DrawRectangleLines(pos.m_value.x, pos.m_value.y, render->m_dimensions.x,
                           render->m_dimensions.y, BLACK);
      } else if (RenderType::CIRCLE == render->m_type) {
        DrawCircle(pos.m_value.x, pos.m_value.y, render->m_dimensions.x, BLACK);
      }
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
    // ECS::Add<ECS::PositionComponent>(s_SelectedBtn, H_CenterText("New Game") - padding, 100.f - padding);
    // ECS::Add<ECS::RenderComponent>(s_SelectedBtn, textWidth + 2.f * padding, 20.f + 2 * padding);  // Rectangle
  }
}

void ResetSystem() { forces.Reset(); }

} // namespace ECS
