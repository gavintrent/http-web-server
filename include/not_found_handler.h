#pragma once

#include "request_handler.h"
#include "http_types.h"

class NotFoundHandler : public RequestHandler {
public:
  std::unique_ptr<HttpResponse> handle_request(const HttpRequest& req);
};