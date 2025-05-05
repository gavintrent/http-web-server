#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H
#include <boost/system/error_code.hpp>
#include <string>
#include <map>

#include "request_handler.h"

/// Serves `/echo` URLs by echoing back the request body.
class EchoHandler : public RequestHandler {
public:
  HttpResponse handleRequest(const HttpRequest& req);
};

#endif
