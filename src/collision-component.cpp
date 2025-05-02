#include "collision-component.h"

CollisionComponent::CollisionComponent(ShapeVariant shape) : m_shape(shape) {}
CollisionComponent::~CollisionComponent() {}

const ShapeVariant &CollisionComponent::GetShape() const { return m_shape; }
