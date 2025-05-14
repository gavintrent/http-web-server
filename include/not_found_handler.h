#pragma once

#include "request_handler.h"
#include "http_types.h"

class NotFoundHandler : public RequestHandler {
public:
  explicit NotFoundHandler(const std::string& path);
  std::unique_ptr<HttpResponse> handle_request(const HttpRequest& req);
  static const std::string kName;
protected:
std::string path_;
};