#include "not_found_handler.h"
#include "handler_registry.h"

const std::string NotFoundHandler::kName = "NotFoundHandler";

NotFoundHandler::NotFoundHandler(const std::string& path) : path_(path) {}

std::unique_ptr<HttpResponse> NotFoundHandler::handle_request(const HttpRequest& req){
    auto res = std::make_unique<HttpResponse>();
    res->status_code = 404;
    return res;
}

// LCOV_EXCL_START
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
// LCOV_EXCL_STOP
