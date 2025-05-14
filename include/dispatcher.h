#pragma once

#include "request_handler.h"
#include <functional>
#include <unordered_map>

class Dispatcher {
public:
  using HandlerFactory = std::function<std::unique_ptr<RequestHandler>()>;
  
  static void registerRoute(std::string path, HandlerFactory factory);
  static std::unique_ptr<RequestHandler> match(std::string path);

private:
  static std::unordered_map<std::string, HandlerFactory> routes;  
};