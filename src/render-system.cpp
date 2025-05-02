#include "render-system.h"
#include "raylib.h"

void RenderSystem(const RenderArchetype &archetype) {
  for (size_t i = 0; i < archetype.m_render.size(); ++i) {
    const Vector2 &pos = archetype.m_transform[i].GetPosition();
    archetype.m_render[i]->Render(pos);
  }
}
