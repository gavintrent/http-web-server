#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "http_types.h"
#include <string>
#include <map>
#include <functional>

class RequestHandler {
public:
  virtual ~RequestHandler() = default;
  virtual HttpResponse handleRequest(const HttpRequest& req) = 0;
};

using RequestHandlerFactory = std::function<RequestHandler*(
    const std::string& path,
    const std::map<std::string, std::string>& args)>;

#endif
