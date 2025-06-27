#include "events.hpp"

void EventBus::Add(std::string event, EventHandler handler) {
  events[event] = handler;
}

void EventBus::Trigger(std::string event) {
  auto& fn = events[event];
  if (fn) {
    fn();
  }
}
