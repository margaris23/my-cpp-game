#ifndef TEXT_COMPONENT_H
#define TEXT_COMPONENT_H

#include "render-component.h"
#include <string>

class TextComponent : public RenderComponent {
public:
  TextComponent(const std::string &text);
  const std::string &GetText() const;

  void Update(float delta) override;
  void Render(const Vector2& pos) const override;

private:
  std::string m_text;
};

#endif
