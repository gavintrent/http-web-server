#include "logout_handler.h"
#include "handler_registry.h"
#include <boost/log/trivial.hpp>

const std::string LogoutHandler::kName = "LogoutHandler";

LogoutHandler::LogoutHandler(const std::string& path) : path_(path) {}

std::unique_ptr<HttpResponse> LogoutHandler::handle_request(const HttpRequest& request) {
    auto response = std::make_unique<HttpResponse>();
    response->status_code = 200;
    response->headers["Content-Type"] = "text/plain";
    response->headers["Set-Cookie"] = "session=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT; HttpOnly";
    response->body = "Logged out successfully";
    return response;
}

static const bool logoutRegistered =
    HandlerRegistry::instance()
        .registerHandler(
            LogoutHandler::kName,
            [](auto const& args) {
                return std::make_unique<LogoutHandler>(args.at(0));
            }
        );