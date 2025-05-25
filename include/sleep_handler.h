#pragma once
#include "request_handler.h"
#include <memory>

class SleepHandler : public RequestHandler {
 public:
  static const std::string kName;
  std::unique_ptr<HttpResponse> handle_request(const HttpRequest& req) override;
};