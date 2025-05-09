#ifndef RENDER_COMPONENT_H
#define RENDER_COMPONENT_H

#include "component.h"
#include "raylib.h"

class RenderComponent : public Component {
public:
  virtual void Render(const Vector2& pos) const = 0;
};

#endif
