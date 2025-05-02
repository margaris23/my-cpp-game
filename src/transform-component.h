#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include "component.h"
#include "raylib.h"

class TransformComponent : Component {
 public:
  TransformComponent(Vector2 position);
  ~TransformComponent() = default;

  void SetPosition(Vector2 pos);
  const Vector2 &GetPosition() const;
  void Update(float delta) override;
  void SetRotation(float rotation);
  float GetRotation() const;

 private:
  Vector2 m_position;
  float m_rotation;
  Vector2 m_scale;

  Vector2 m_velocity;
  Vector2 m_angularVelocity;
  Vector2 m_scaleVelocity;
};

#endif
