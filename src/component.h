#ifndef COMPONENT_H
#define COMPONENT_H

enum class ComponentType { GENERIC, TEXT, TRANSFORM, COLLISION };

class Component {
public:
  Component();
  virtual ~Component();

  virtual void Update(float delta) = 0;

protected:
  ComponentType m_type;
};
#endif
