#ifndef COLLISION_COMPONENT_H
#define COLLISION_COMPONENT_H

#include "raylib.h"
#include <memory>
#include <variant>

struct Circle {
  Vector2 center;
  float radius;
};

using ShapeVariant = std::variant<Rectangle, Circle>;

class CollisionComponent {
public:
  CollisionComponent() = delete;
  CollisionComponent(ShapeVariant shape);
  virtual ~CollisionComponent();

  const ShapeVariant &GetShape() const;
  virtual bool CheckCollision(const CollisionComponent &other) = 0;

protected:
  ShapeVariant m_shape;
  std::unique_ptr<const CollisionComponent> m_collidedWith;
};

#endif
