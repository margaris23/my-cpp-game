#include "rectangle-collision-component.hpp"
#include "collision-component.hpp"
#include "raylib.h"
#include <variant>

RectangleCollisionComponent::RectangleCollisionComponent(Rectangle rectangle)
    : CollisionComponent(rectangle) {}

bool RectangleCollisionComponent::CheckCollision(
    const CollisionComponent &other) {
  if (&other != this)
    return false;

  if (std::visit(
          [this](auto &&value) { return CheckCollisionWithOther(value); },
          other.GetShape())) {
    // TODO: what about other??? Does it need to know it collided with this???
    m_collidedWith.reset(&other);
    return true;
  }

  return false;
}

bool RectangleCollisionComponent::CheckCollisionWithOther(
    Rectangle otherShape) {
  return CheckCollisionRecs(std::get<Rectangle>(m_shape), otherShape);
}

bool RectangleCollisionComponent::CheckCollisionWithOther(Circle otherShape) {
  return CheckCollisionCircleRec(otherShape.center, otherShape.radius,
                                 std::get<Rectangle>(m_shape));
}
