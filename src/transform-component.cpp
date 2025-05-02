#include "transform-component.h"
#include "raylib.h"
#include <cmath>

TransformComponent::TransformComponent(Vector2 position)
    : m_position(position), m_velocity(Vector2{0}) {}

void TransformComponent::SetPosition(Vector2 position) {
  m_position = position;
}
const Vector2 &TransformComponent::GetPosition() const { return m_position; }

void TransformComponent::Update(float delta) {
  m_position.x += m_velocity.x;
  m_position.y += m_velocity.y;

  // float to double conversion ?
  float sinT = sin(m_rotation);
  float cosT = cos(m_rotation);
  // Unecessary COPY here ???
  SetPosition(Vector2{m_position.x * cosT - sinT * m_position.y,
                      sinT * m_position.x + cosT * m_position.y});
}

void TransformComponent::SetRotation(float rotation) { m_rotation = rotation; }

float TransformComponent::GetRotation() const { return m_rotation; }
