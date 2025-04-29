#ifndef STATIC_HANDLER_H
#define STATIC_HANDLER_H

#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

#include "http_types.h"
#include "request_handler.h"

class StaticHandler : public RequestHandler {
public:
  HttpResponse handleRequest(const HttpRequest& req);
};

#endif
