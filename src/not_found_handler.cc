#include "not_found_handler.h"
#include "handler_registry.h"

const std::string NotFoundHandler::kName = "NotFoundHandler";

NotFoundHandler::NotFoundHandler(const std::string& path) : path_(path) {}

RequestHandler* NotFoundHandler::Create(const std::string& path, const std::map<std::string, std::string>& args) {
  if (!args.empty()) return nullptr;
  return new NotFoundHandler(path);
}

std::unique_ptr<HttpResponse> NotFoundHandler::handle_request(const HttpRequest& req){
    auto res = std::make_unique<HttpResponse>();
    res->status_code = 404;
    return res;
}


static const bool notFoundRegistered =
  HandlerRegistry::instance()
    .registerHandler(
      NotFoundHandler::kName,          
      [](auto const& args) {      
        return std::make_unique<NotFoundHandler>(
          args.at(0)
        );
      }
    );