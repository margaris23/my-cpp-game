#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include "render-component.h"
#include "transform-component.h"
#include <memory>
#include <vector>

struct RenderArchetype {
  std::vector<std::unique_ptr<RenderComponent>> m_render;
  std::vector<TransformComponent> m_transform;
};

void RenderSystem(const RenderArchetype &archetype);

#endif
