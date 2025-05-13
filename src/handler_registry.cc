#include "handler_registry.h"

HandlerRegistry& HandlerRegistry::instance() {
  static HandlerRegistry inst;
  return inst;
}

HandlerRegistry::HandlerRegistry() = default;

bool HandlerRegistry::registerHandler(const std::string& name,
                                       Factory factory) {
  factories[name] = std::move(factory);
  return true;
}

HandlerRegistry::HandlerPtr
HandlerRegistry::createHandler(const std::string& name,
                               const std::vector<std::string>& args) const {
  auto it = factories.find(name);
  if (it == factories.end()) return nullptr;
  return it->second(args);
}

