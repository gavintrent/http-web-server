#include "health_handler.h"
#include "handler_registry.h"
#include <boost/log/trivial.hpp>

const std::string HealthHandler::kName = "HealthHandler";

void HealthHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
  uri_prefix_ = uri_prefix;
}

std::unique_ptr<HttpResponse>
HealthHandler::handle_request(const HttpRequest& request) {
    auto response = std::make_unique<HttpResponse>();
    
    // check for malformed requests first
    if (request.method.empty()) {
        response->status_code = 400;
        response->headers["Content-Type"] = "text/plain";
        response->body = "Bad Request";
    }
    //  check for valid health check requests
    else if (request.method == "GET" && request.path == uri_prefix_) {
        response->status_code = 200;
        response->headers["Content-Type"] = "text/plain";  
        response->body = "OK";
    }
    // all else is 404
    else {
        response->status_code = 404;
        response->headers["Content-Type"] = "text/plain";
        response->body = "Not Found";
    }

    return response;
}

// LCOV_EXCL_START
static const bool healthRegistered =
  HandlerRegistry::instance()
    .registerHandler(
      HealthHandler::kName,
      [](const std::vector<std::string>& args) {
        auto h = std::make_unique<HealthHandler>();
        // args[0] is the "/health" prefix from the config
        h->Init(args[0], NginxConfig{});
        return h;
      }
    );
// LCOV_EXCL_STOP
