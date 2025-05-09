#include "movement-system.hpp"

void MovementSystem(float delta, std::vector<TransformComponent> &transforms) {
  for (auto &transform : transforms) {
    transform.Update(delta);
  }
}
