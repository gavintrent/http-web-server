#include "handler_registry.h"

std::vector<std::string>& HandlerRegistry::registry() { //gives most up to date registry to all handlers
  static std::vector<std::string> inst;
  return inst;
}

void HandlerRegistry::registerHandler(const std::string& name) {
  registry().push_back(name);
}

std::vector<std::string> HandlerRegistry::getRegisteredNames() {
  return registry();
}