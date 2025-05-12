#pragma once

#include <string>
#include <vector>

class HandlerRegistry {
public:
  static void registerHandler(const std::string& name);
  static std::vector<std::string> getRegisteredNames(); //all registered names

private:
  static std::vector<std::string>& registry(); //shared between all handlers
};


#define REGISTER_HANDLER(Name)                            \
  namespace {                                            \
    const bool _##Name##_registered = []{                \
      HandlerRegistry::registerHandler(#Name);           \
      return true;                                       \
    }();                                                  \
  }