#pragma once

#include "request_handler.h"
#include "http_types.h"

class NotFoundHandler : public RequestHandler {
public:
  explicit NotFoundHandler(const std::string& path);
  std::unique_ptr<HttpResponse> handle_request(const HttpRequest& req);
  static RequestHandler* Create(const std::string& path, const std::map<std::string, std::string>& args);
  static const std::string kName;
protected:
std::string path_;
};