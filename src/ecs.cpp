#include "ecs.hpp"
#include "fmt/core.h"
#include "game.hpp"
#include "raylib.h"
#include "raymath.h"
#include "reasings.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <optional>
#include <random>
#include <string>
#include <variant>

namespace ECS {

std::atomic<size_t> ThreadSafeIdGenerator::counter(0);

// std::unordered_map<std::type_index, std::bitset<MAX_COMPONENTS>> s_typeToBitSetMap;

Entity Registry::CreateEntity() {
  size_t index = ThreadSafeIdGenerator::getNextId();
  m_entities.push_back(index);
  return index;
}

Registry::~Registry() {
  if (m_entities.size() > 0) {
    for (auto it = m_entities.begin(); it != m_entities.end();) {
      CleanupEntity(*it);
      it = m_entities.erase(it);
    }
  }
}

void Registry::CleanupEntity(Entity entity) {
  Remove<PositionComponent>(entity);
  Remove<VelocityComponent>(entity);
  Remove<ColliderComponent>(entity);
  Remove<TextComponent>(entity);
  Remove<RenderComponent>(entity);
  m_renders_sorted = false; // will trigger sorting in RenderSystem
  Remove<SpriteComponent>(entity);
  Remove<ForceComponent>(entity);
  Remove<UIComponent>(entity);
  Remove<HealthComponent>(entity);
  Remove<DmgComponent>(entity);
  Remove<GameStateComponent>(entity);
  Remove<WeaponComponent>(entity);
  Remove<InputComponent>(entity);
  Remove<EmitterComponent>(entity);
  Remove<ParticleComponent>(entity);
}

void Registry::DeleteEntity(Entity entity) {
  CleanupEntity(entity);
  m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), entity), m_entities.end());
}

void Registry::Init() {
  // TODO: Experimentation - remove
  // s_typeToBitSetMap[std::type_index(typeid(TransformComponent))] = 1 << 0;
  // s_typeToBitSetMap[std::type_index(typeid(RenderComponent))] = 1 << 1;
  // s_typeToBitSetMap[std::type_index(typeid(CollisionComponent))] = 1 << 2;
  gen = std::mt19937(rd());
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

      // generate particles
      if (Shape::CIRCLE == collider.shape) {
        // Randomize
        auto meteor_vel = Get<VelocityComponent>(collider.entity);
        auto pos = Get<PositionComponent>(collider.entity);

        auto dir = Vector2Subtract(meteor_vel->value, pos->value);
        dir.x = dir.x < 0 ? -1.f : 1.f;
        dir.y = dir.y < 0 ? -1.f : 1.f;

        std::uniform_real_distribution<float> rnd_vel_y(5.f,10.f);
        std::uniform_real_distribution<float> rnd_vel_x(5.f,10.f);
        // Generate particle -- NO Emitter for now ...
        Entity particle = CreateEntity();
        Add<RenderComponent>(particle, Layer::GROUND, Shape::ELLIPSE, BLACK, 5.f, 5.f);
        Add<HealthComponent>(particle, 10.f);
        Add<PositionComponent>(particle, pos->value.x, pos->value.y);
        Add<VelocityComponent>(particle, meteor_vel->value.y + dir.y * rnd_vel_y(gen),
                               meteor_vel->value.x + dir.x * rnd_vel_x(gen));
        Add<ParticleComponent>(particle, pos->entity);
      }

      collider.collided_with.reset();
    }
  }
}

void Registry::UISystem() {
  for (auto &widget : m_widgets.dense) {
    auto widgetPos = m_positions.Get(widget.entity);

    // // Handle selected widget logic ... Like a focus
    // if (widget.selectedEntity.has_value()) {
    //   auto selectedWidgetPos = m_positions.Get(widget.selectedEntity.value());
    //
    //   if (widgetPos && selectedWidgetPos) {
    //     widgetPos->value.x = selectedWidgetPos->value.x;
    //     widgetPos->value.y = selectedWidgetPos->value.y;
    //   }
    //   // ECS::Add<ECS::PositionComponent>(s_SelectedBtn, H_CenterText("New Game") -
    //   // padding, 100.f - padding); ECS::Add<ECS::eenderComponent>(s_SelectedBtn,
    //   // textWidth + 2.f * padding, 20.f + 2 * padding);  // Rectangle
    // } else

    if (UIElement::BAR == widget.type) {
      const auto state = m_stateValues.Get(widget.entity);
      if (state) {
        std::visit(
            [&widgetPos, &widget](auto &&val) {
              DrawRectangle(widgetPos->value.x, widgetPos->value.y, val * 10, 20, widget.color);
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
      } else if (Shape::METEOR == render.shape) {
        Vector2 center{pos->value.x, pos->value.y};

        // ==== METEORS ====
        const auto &values = render.noise_values;
        const float two_pi_count = 2.f * PI / values.size();

        // TODO: make more performant
        for (int i = 0; i < values.size(); i++) {
          const int next = (i + 1) % values.size();
          const float radius[2]{render.dimensions.x + values[i],
                                render.dimensions.x + values[next]};
          const float angle[2]{i * two_pi_count, next * two_pi_count};
          const Vector2 coeff[2]{
              {center.x + cosf(angle[0]) * radius[0], center.y + sinf(angle[0]) * radius[0]},
              {center.x + cosf(angle[1]) * radius[1], center.y + sinf(angle[1]) * radius[1]}};

          // DrawLineV(coeff[0], coeff[1], render.color); // TRANSPARENT
          DrawTriangle(center, coeff[1], coeff[0], render.color); // SOLID
        }

      } else if (Shape::LINE == render.shape) {
        DrawLine(pos->value.x, pos->value.y, pos->value.x + render.dimensions.x,
                 pos->value.y + render.dimensions.y, render.color);
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

  // SPRITES
  for (auto &sprite : m_sprites.dense) {
    const auto pos = m_positions.Get(sprite.entity);
    // Anchor point is center of texture
    // DrawTexture(sprite.texture, pos->value.x - sprite.texture.width / 2.f,
    //             pos->value.y - sprite.texture.height / 2.f, WHITE);

    // DrawTexture(sprite.texture, pos->value.x, pos->value.y, WHITE);
    DrawTextureEx(sprite.texture, {pos->value.x, pos->value.y}, 0, sprite.scale, WHITE);

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

// Only 1 input component supported for now
void Registry::InputSystem() {
  const auto &input = m_inputs.dense[0];
  const Entity spaceship = input.entity;

  // TODO: implement a force accumulator
  auto force = Get<ForceComponent>(spaceship);
  if (force) {
    // Simulate drag by applying force inc/dec per dt and limiting
    if (IsKeyDown(KEY_RIGHT)) {
      force->value.x += input.push_force_step;
    } else if (IsKeyDown(KEY_LEFT)) {
      force->value.x -= input.push_force_step;
    } else if (force->value.x > input.push_force_step_half) { // correction
      force->value.x -= input.push_force_step;
    } else if (force->value.x < -input.push_force_step_half) { // correction
      force->value.x += input.push_force_step;
    } else {
      force->value.x = 0.f;
    }

    if (IsKeyDown(KEY_UP)) {
      force->value.y -= input.push_force_step;
    } else if (IsKeyDown(KEY_DOWN)) {
      force->value.y += input.push_force_step;
    } else if (force->value.y > input.push_force_step_half) { // correction
      force->value.y -= input.push_force_step;
    } else if (force->value.y < -input.push_force_step_half) { // correction
      force->value.y += input.push_force_step;
    } else {
      force->value.y = 0.f;
    }

    // Limit and then SetMag
    if (Vector2Length(force->value) > input.max_push_force) {
      force->value = Vector2Normalize(force->value);
      force->value.x *= input.max_push_force;
      force->value.y *= input.max_push_force;
    }
  }

  // WEAPON FIRING
  // Only 1 weapon supported for now
  auto &weapon = m_weapons.dense[0];
  const Entity miningBeam = weapon.entity;

  if (IsKeyDown(KEY_SPACE)) {
    if (!weapon.isFiring) {
      weapon.isFiring = true;
      weapon.firingDuration = 0;
      Add<ColliderComponent>(miningBeam, Game::WEAPON_SIZE, 10.f);

      // Enable Emitter
      // auto emitter = Get<EmitterComponent>(miningBeam);
      // if (emitter && !emitter->active) {
      //   emitter->active = true;
      //   emitter->m_timer = 0.f;
      //   fmt::println("Emitter: STATUS {}", emitter->active);
      // }

      Add<RenderComponent>(miningBeam, Layer::GROUND, Shape::RECTANGLE, BROWN, Game::WEAPON_SIZE,
                           10.f);
    } else {
      auto beam_collider = Get<ColliderComponent>(miningBeam);

      if (beam_collider) {
        if (!beam_collider->collided_with.has_value() && weapon.firingDuration < 24.f) {
          ++weapon.firingDuration;
        }

        // Ease(now, start, max_change, duration)
        float tween =
            EaseLinearIn(weapon.firingDuration, 10.f, Game::WEAPON_MAX_DISTANCE - 10.f, 24.f);
        beam_collider->dimensions.y = tween;

        // Update Weapon's RenderComponent
        auto beam_render = Get<RenderComponent>(miningBeam);
        if (beam_render) {
          beam_render->dimensions.y = tween;
        }
      }
    }
  } else if (weapon.isFiring) {
    weapon.isFiring = false;
    weapon.firingDuration = 0;
    // auto emitter = Get<EmitterComponent>(miningBeam);
    // if (emitter) {
    //   emitter->active = false;
    //   emitter->m_timer = 0.f;
    //   fmt::println("Emitter: STATUS {}", emitter->active);
    // }
    Remove<RenderComponent>(miningBeam);
    Remove<ColliderComponent>(miningBeam);
  }
}

void Registry::ParticleSystem() {
  for (auto &emitter : m_emitters.dense) {
    if (!emitter.active) {
      continue;
    }

    ++emitter.m_timer;
    if (emitter.m_timer >= emitter.rate) {
      fmt::println("Emitter FIRE: {}", emitter.m_timer);
      emitter.m_timer = 0;

      // Generate Particles (emitter is the origin)
      auto emitter_pos = Get<PositionComponent>(emitter.entity);

      // Randomizers
      std::uniform_int_distribution<int> number(3, 5);
      std::uniform_real_distribution<float> rnd_offset(-5.f, 5.f);
      std::uniform_real_distribution<float> rnd_velocity(emitter.particle_velocity.y - 5.f,
                                                         emitter.particle_velocity.y);

      // multiple particles -> SEGFAULT
      // for (int i = 0; i < number(gen); i++) {
      // Entity particle = CreateEntity();
      // fmt::println("Generated: {}", particle);
      // Add<RenderComponent>(particle, Layer::GROUND, emitter.particle_shape, MAROON, 0.f, 20.f);
      // Add<HealthComponent>(particle, emitter.particle_lifetime);
      // Add<PositionComponent>(particle, emitter_pos->value.x, emitter_pos->value.y);
      // Add<VelocityComponent>(particle, rnd_velocity(gen), emitter.particle_velocity.y);
      // // connect particle with emitter, add position offset
      // Add<ParticleComponent>(particle, emitter.entity, rnd_offset(gen));
      // }
    }
  }

  // TODO: check if this should be done at a later step i.e a separate System
  for (auto &particle : m_particles.dense) {
    if (!particle.active) {
      DeleteEntity(particle.entity);
      continue;
    }

    // update health
    auto health = Get<HealthComponent>(particle.entity);
    --health->value;

    if (health->value <= 0.f) {
      particle.active = false;
    }

    // else {
    //   // update pos
    //   // follow emitter's x
    //   auto emitter_pos = Get<PositionComponent>(particle.emitter);
    //   auto pos = Get<PositionComponent>(particle.entity);
    //   pos->value.x = emitter_pos->value.x + particle.offset;
    //   // do not allow particle to appear above spaceshifp
    //   if (emitter_pos->value.y > pos->value.y) {
    //     pos->value.y = emitter_pos->value.y;
    //   }
    // }
  }
}

void Registry::ResetSystem() { m_forces.Reset(); }

void Registry::Debug() {
  int positions = m_positions.dense.size();
  int renders = m_renders.dense.size();
  int particles = m_particles.dense.size();
  int entities = m_entities.size();

  DrawText(TextFormat("e:%i", entities), 10, 60, 20, BLACK);
  DrawText(TextFormat("p:%i", positions), 10, 80, 20, BLACK);
  DrawText(TextFormat("r:%i", renders), 10, 100, 20, BLACK);
  DrawText(TextFormat("cnt:%i", ThreadSafeIdGenerator::getCurrentId()), 10, 120, 20, BLACK);
  DrawText(TextFormat("pts:%i", particles), 10, 140, 20, BLACK);
}
} // namespace ECS
