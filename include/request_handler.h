#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "http_types.h"

class RequestHandler {
public:
  virtual ~RequestHandler() = default;
  virtual HttpResponse handleRequest(const HttpRequest& req) = 0;
};

#endif
