#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <functional>
#include <string>
#include <unordered_map>

using EventHandler = std::function<void()>;

class EventBus {
public:
  void Add(std::string event, EventHandler);
  void Trigger(std::string event);

private:
  std::unordered_map<std::string, EventHandler> events;
};

#endif
