#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

#include <fstream>
#include <string>
#include <sstream>

#include "echo_handler.h"

HttpResponse EchoHandler::handleRequest(const HttpRequest& req) {
  HttpResponse res;
  if (req.method == "GET") {
    res.status_code = 200;
    res.body = req.raw;
    res.headers["Content-Type"] = "text/plain";
  } else {
    res.status_code = 400;
  }
  res.headers["Content-Length"] = std::to_string(res.body.size());
  return res;
}