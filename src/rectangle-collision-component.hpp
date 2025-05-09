#ifndef RECTANGLE_COLLISION_COMPONENT_H
#define RECTANGLE_COLLISION_COMPONENT_H

#include "collision-component.hpp"
#include "raylib.h"

class RectangleCollisionComponent : public CollisionComponent {
public:
  RectangleCollisionComponent(Rectangle rectangle);
  ~RectangleCollisionComponent() = default;

  bool CheckCollision(const CollisionComponent &other) override;

private:
  bool CheckCollisionWithOther(Rectangle otherShape);
  bool CheckCollisionWithOther(Circle otherShape);
};

#endif
