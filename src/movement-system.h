#ifndef MOVEMENT_SYSTEM_H
#define MOVEMENT_SYSTEM_H

#include "collision-component.h"
#include "transform-component.h"
#include <memory>
#include <vector>

struct MovementArchetype {
  std::vector<TransformComponent> m_transform;
  static std::vector<std::unique_ptr<CollisionComponent>> collisions;
};

void MovementSystem(float delta, std::vector<TransformComponent> &transforms);

#endif
