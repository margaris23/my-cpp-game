#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include "collision-component.h"
#include <vector>

void CollisionSystem(
    std::vector<std::unique_ptr<CollisionComponent>> &components);
#endif
