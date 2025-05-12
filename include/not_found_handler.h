#pragma once

#include "request_handler.h"
#include "http_types.h"

class NotFoundHandler : public RequestHandler {
public:
  HttpResponse handleRequest(const HttpRequest& req);
};