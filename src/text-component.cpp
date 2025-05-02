#include "text-component.h"
#include "component.h"

TextComponent::TextComponent(const std::string &text) : m_text(text) {
  m_type = ComponentType::TEXT;
}

const std::string &TextComponent::GetText() const { return m_text; }

void TextComponent::Update(float delta) {}

void TextComponent::Render(const Vector2& pos) const {
  DrawText(m_text.c_str(), pos.x, pos.y, 20, BLACK);
}
