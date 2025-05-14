#include "dispatcher.h"

std::unordered_map<std::string, Dispatcher::HandlerFactory> Dispatcher::routes;

void Dispatcher::registerRoute(std::string path, HandlerFactory factory) {
    routes[path] = std::move(factory);
}

std::unique_ptr<RequestHandler> Dispatcher::match(std::string path) {
    std::string longestMatch = "";
    int matchLength = 0;
    
    for (const auto& [key, value] : routes) {
        if (path.rfind(key, 0) == std::string::npos) { continue; }
        if (key.length() > matchLength) {
            longestMatch = key;
            matchLength = key.length();
        }
    }

    return routes[longestMatch]();
}