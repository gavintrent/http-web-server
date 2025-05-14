#ifndef STATIC_HANDLER_H
#define STATIC_HANDLER_H

#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <string>
#include <map>
#include "http_types.h"
#include "request_handler.h"

class StaticHandler : public RequestHandler {
public:
  StaticHandler(const std::string& path, const std::string& root_dir);

  std::unique_ptr<HttpResponse> handle_request(const HttpRequest& req) override;

  static const std::string kName;

protected:
  std::string path_;
  std::string root_dir_;
};

#endif
