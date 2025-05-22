#include "ecs.hpp"
#include "raylib.h"
#include <algorithm>
#include <functional>
#include <optional>
#include <string>
#include <variant>

namespace ECS {

// std::unordered_map<std::type_index, std::bitset<MAX_COMPONENTS>> s_typeToBitSetMap;

Entity Registry::CreateEntity() {
  size_t index = m_entityCounter++;
  m_entities.push_back(index);
  return index;
}

Registry::~Registry() {
  for (auto it = m_entities.begin(); it != m_entities.end();) {
    CleanupEntity(*it);
    it = m_entities.erase(it);
  }
}

void Registry::CleanupEntity(Entity entity) {
  Remove<PositionComponent>(entity);
  Remove<VelocityComponent>(entity);
  Remove<ColliderComponent>(entity);
  Remove<TextComponent>(entity);
  Remove<RenderComponent>(entity);
  m_renders_sorted = false;
  Remove<SpriteComponent>(entity);
  Remove<ForceComponent>(entity);
  Remove<UIComponent>(entity);
  Remove<HealthComponent>(entity);
  Remove<DmgComponent>(entity);
  Remove<GameStateComponent>(entity);
  Remove<WeaponComponent>(entity);
  // add more ...
}

void Registry::DeleteEntity(Entity entity) {
  const auto &it = std::remove(m_entities.begin(), m_entities.end(), entity);
  m_entities.pop_back();
  CleanupEntity(entity);
}

void Registry::Init() {
  // s_typeToBitSetMap[std::type_index(typeid(TransformComponent))] = 1 << 0;
  // s_typeToBitSetMap[std::type_index(typeid(RenderComponent))] = 1 << 1;
  // s_typeToBitSetMap[std::type_index(typeid(CollisionComponent))] = 1 << 2;
}
void Registry::PositionSystem() {
  float screen_width = GetScreenWidth();
  float screen_height = GetScreenHeight();

  for (auto &pos : m_positions.dense) {
    const auto force = m_forces.Get(pos.entity);
    auto velocity = m_velocities.Get(pos.entity);
    auto weapon = m_weapons.Get(pos.entity);

    if (force) {
      // A = F / M, M == 1
      if (velocity) {
        velocity->value.x += force->value.x;
        velocity->value.y += force->value.y;
      } else {
        pos.value.x += force->value.x;
        pos.value.y += force->value.y;
      }
    } else if (velocity) {
      pos.value.x += velocity->value.x;
      pos.value.y += velocity->value.y;
    } else if (weapon) {
      const auto shooterPos = m_positions.Get(weapon->shooter);
      const auto shooterSprite = m_sprites.Get(weapon->shooter);
      // offsets are due to weapon size - TODO: address this
      pos.value.x = shooterPos->value.x + shooterSprite->texture.width / 2.f - 5.f;
      pos.value.y = shooterPos->value.y + shooterSprite->texture.height / 2.f - 8.f;
    }

    // ideally, size should be included
    if (pos.value.x < -30.f) {
      pos.value.x = screen_width;
    } else if (pos.value.x > screen_width + 30.f) {
      pos.value.x = 0;
    }

    if (pos.value.y < -30.f) {
      pos.value.y = screen_height;
    } else if (pos.value.y > screen_height + 30.f) {
      pos.value.y = 0;
    }
  }
}

bool HandleCollision(ColliderComponent &colA, ColliderComponent &colB,
                     const PositionComponent *posA, const PositionComponent *posB) {
  const auto &dimA = colA.dimensions;

  bool collides = CheckCollisionCircleRec(posB->value, colB.dimensions.x,
                                          {posA->value.x, posA->value.y, dimA.x, dimA.y});

  if (collides) {
    colA.collided_with = colB.entity;
    colB.collided_with = colA.entity;
    return true;
  }

  return false;
}

void Registry::CollisionDetectionSystem() {
  const auto &positionComponents = m_positions.dense;
  auto &colliderComps = m_colliders.dense;

  for (size_t indexA = 0; indexA < colliderComps.size() - 1; indexA++) {
    for (size_t indexB = indexA + 1; indexB < m_colliders.dense.size(); indexB++) {
      auto posA = m_positions.Get(colliderComps[indexA].entity);
      auto posB = m_positions.Get(colliderComps[indexB].entity);

      // TODO: (Edge Case) Need to address this later
      if (!posB || !posA) {
        continue;
      }

      // Optimizations:
      // RECTANGLE expected to be our Spaceship or its MiningBeam (weapon),
      // Circles are expected to be our Meteors
      // Circles do not expect to collide each other so,
      // Rectangle is expected to collide with only 1 Circle
      if (Shape::RECTANGLE == colliderComps[indexA].shape &&
          Shape::CIRCLE == colliderComps[indexB].shape) {
        if (HandleCollision(colliderComps[indexA], colliderComps[indexB], posA, posB)) {
          break;
        }
      } else if (Shape::RECTANGLE == colliderComps[indexB].shape &&
                 Shape::CIRCLE == colliderComps[indexA].shape) {
        if (HandleCollision(colliderComps[indexB], colliderComps[indexA], posB, posA)) {
          break;
        }
      }
    }
  }
}

void Registry::CollisionResolutionSystem() {
  for (auto &collider : m_colliders.dense) {
    if (collider.collided_with.has_value()) {
      auto health = m_healths.Get(collider.entity);
      auto dmg = m_dmgs.Get(collider.collided_with.value());
      if (health && dmg) {
        health->value -= dmg->value;

        // constraint
        if (health->value < 0) {
          health->value = 0;
        }

        // Need to add Bounce physics
        // Need to know: pos and dir ???
        // auto pos = positions.Get(collider.entity);
        // Add<ForceComponent>(collider.entity, -10.f, -10.f);
      }
      collider.collided_with.reset();
    }
  }
}

void Registry::UISystem() {
  for (auto &widget : m_widgets.dense) {
    auto widgetPos = m_positions.Get(widget.entity);

    // Handle selected widget logic ... Like a focus
    if (widget.selectedEntity.has_value()) {
      auto selectedWidgetPos = m_positions.Get(widget.selectedEntity.value());

      if (widgetPos && selectedWidgetPos) {
        widgetPos->value.x = selectedWidgetPos->value.x;
        widgetPos->value.y = selectedWidgetPos->value.y;
      }
      // ECS::Add<ECS::PositionComponent>(s_SelectedBtn, H_CenterText("New Game") -
      // padding, 100.f - padding); ECS::Add<ECS::eenderComponent>(s_SelectedBtn,
      // textWidth + 2.f * padding, 20.f + 2 * padding);  // Rectangle
    } else if (UIElement::BAR == widget.type) {
      const auto state = m_stateValues.Get(widget.entity);
      if (state) {
        std::visit(
            [widgetPos](auto &&val) {
              DrawRectangle(widgetPos->value.x, widgetPos->value.y, val * 10, 20, BLACK);
            },
            state->value);
      }
    }
    // else if (UIElement::TEXT == widget.m_type) {
    //   const auto text = texts.Get(widget.entity);
    //   if (text) {
    //     DrawText(text->value.c_str(), widgetPos->value.x, widgetPos->value.y, 20,
    //     BLACK);
    //   }
    // }
  }
}

struct {
  bool operator()(const RenderComponent &lhs, const RenderComponent &rhs) {
    return lhs.priority < rhs.priority;
  }
} compareLayer;

void Registry::RenderSystem() {
  if (!m_renders_sorted) {
    // Sort by Layer
    std::sort(m_renders.dense.begin(), m_renders.dense.end(), compareLayer);
    // Update sparse indexing
    for (int i = 0; i < m_renders.dense.size(); i++) {
      m_renders.sparse[m_renders.dense[i].entity] = i;
    }
    m_renders_sorted = true;
    // TODO: sort sprites
  }

  // SHAPES
  for (auto &render : m_renders.dense) {
    const auto pos = m_positions.Get(render.entity);
    if (render.IsVisible()) {
      if (Shape::RECTANGLE == render.shape) {
        DrawRectangleLines(pos->value.x, pos->value.y, render.dimensions.x, render.dimensions.y,
                           render.color);
        // DrawRectangle(pos->value.x + 1.f, pos->value.y + 1.f,
        //               render.dimensions.x - 2.f, render.dimensions.y - 2.f,
        //               RAYWHITE);
      } else if (Shape::ELLIPSE == render.shape) {
        DrawEllipseLines(pos->value.x, pos->value.y, render.dimensions.x, render.dimensions.y,
                         render.color);
      } else if (Shape::CIRCLE == render.shape) {
        DrawCircle(pos->value.x, pos->value.y, render.dimensions.x, render.color);
        // DrawCircle(pos->value.x, pos->value.y, 10.f, render.color);
      } else if (Shape::RECTANGLE_SOLID == render.shape) {
        DrawRectangle(pos->value.x, pos->value.y, render.dimensions.x, render.dimensions.y,
                      render.color);
      }
      // TODO: add more...
    }
  }

  for (auto &sprite : m_sprites.dense) {
    const auto pos = m_positions.Get(sprite.entity);
    // Anchor point is center of texture
    // DrawTexture(sprite.texture, pos->value.x - sprite.texture.width / 2.f,
    //             pos->value.y - sprite.texture.height / 2.f, WHITE);

    DrawTexture(sprite.texture, pos->value.x, pos->value.y, WHITE);
    // DrawTextureEx(sprite.texture, {pos->value.x, pos->value.y}, 0, 1.f, WHITE);

    // Rectangle source{0, 0, (float)sprite.texture.width, (float)sprite.texture.height};
    // Rectangle dest{pos->value.x, pos->value.y, (float)sprite.texture.width,
    //                (float)sprite.texture.height};
    // DrawTexturePro(sprite.texture, source, dest,
    //                {sprite.texture.width / 2.f, sprite.texture.height / 2.f}, 0, WHITE);
  }

  // DEBUG
  // for (auto &collider : m_colliders.dense) {
  //   const auto pos = m_positions.Get(collider.entity);
  //   if (Shape::CIRCLE == collider.shape) {
  //     DrawCircleLines(pos->value.x, pos->value.y, collider.dimensions.x, GREEN);
  //   } else if (Shape::RECTANGLE == collider.shape) {
  //     DrawRectangleLines(pos->value.x, pos->value.y, collider.dimensions.x,
  //     collider.dimensions.y,
  //                        GREEN);
  //   }
  // }

  // TEXTS
  for (const auto &text : m_texts.dense) {
    const auto pos = m_positions.Get(text.entity);
    DrawText(text.value.c_str(), pos->value.x, pos->value.y, 20, text.color);
  }
}

void Registry::ResetSystem() { m_forces.Reset(); }

} // namespace ECS
