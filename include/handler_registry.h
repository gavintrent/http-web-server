#pragma once
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>
#include "request_handler.h"

class HandlerRegistry {
public:
  using HandlerPtr = std::unique_ptr<RequestHandler>;
  // Now factory takes *any* number of string args in a vector
  using Factory    = std::function<HandlerPtr(const std::vector<std::string>&)>;

  // singleton
  static HandlerRegistry& instance();

  // register a factory under `name`
  bool registerHandler(const std::string& name, Factory factory);

  // pass along the `args` vector to the chosen factory
  HandlerPtr createHandler(const std::string& name,
                           const std::vector<std::string>& args = {}) const;

private:
  HandlerRegistry();
  std::unordered_map<std::string, Factory> factories;
};
