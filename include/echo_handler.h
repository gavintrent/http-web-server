#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H
#include <boost/system/error_code.hpp>
#include <string>
#include <map>
#include "request_handler.h"

/// Serves `/echo` URLs by echoing back the request body.
class EchoHandler : public RequestHandler {
public:
  explicit EchoHandler(const std::string& path);

  std::unique_ptr<HttpResponse> handle_request(const HttpRequest& req) override;
  //factory method:
  static RequestHandler* Create(const std::string& path, const std::map<std::string, std::string>& args);
  static const std::string kName;

protected:
  std::string path_;
  // new hook
  virtual std::unique_ptr<HttpResponse> doEcho(const HttpRequest& req);
};

#endif
