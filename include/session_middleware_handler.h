#ifndef SESSION_MIDDLEWARE_HANDLER_H
#define SESSION_MIDDLEWARE_HANDLER_H

#include "request_handler.h"
#include "session_context.h"
#include <regex>
#include <optional>
#include <string>

class SessionMiddlewareHandler : public RequestHandler {
public:
  explicit SessionMiddlewareHandler(std::unique_ptr<RequestHandler> next);
  std::unique_ptr<HttpResponse> handle_request(const HttpRequest& request) override;
  std::string get_kName() override { return "SessionMiddlewareHandler"; }

private:
  std::unique_ptr<RequestHandler> next_handler_;
  static const std::regex session_cookie_regex;
  std::optional<std::string> extract_session_token(const HttpRequest& request);
};

#endif // SESSION_MIDDLEWARE_HANDLER_H
