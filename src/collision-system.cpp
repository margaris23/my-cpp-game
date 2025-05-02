#include "collision-system.h"
#include "collision-component.h"
#include "raylib.h"
#include <memory>
#include <vector>

// Unfortunately collision Components refer to entities which have TransformComponents
// and based on those, collision is verified
// TODO: need to GetEntity->GetPosition first!!!
void CollisionSystem(
    std::vector<std::unique_ptr<CollisionComponent>> &components) {

  // O(N^2) for now...
  for (auto &component1 : components) {
    for (auto &component2 : components) {
      if (component1->CheckCollision(*component2)) {
        TraceLog(LOG_INFO, "COLLIDED!");
      }
    }
  }
}
