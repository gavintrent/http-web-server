#ifndef LOGOUT_HANDLER_H
#define LOGOUT_HANDLER_H

#include "request_handler.h"
#include "http_types.h"
#include "config_parser.h"
#include <memory>
#include <string>

class LogoutHandler : public RequestHandler {
public:
    explicit LogoutHandler(const std::string& path);
    ~LogoutHandler() override = default;

    std::unique_ptr<HttpResponse> handle_request(const HttpRequest& request) override;

    static const std::string kName;
    std::string get_kName() override { return kName; }

private:
    std::string path_;
};

#endif // LOGOUT_HANDLER_H
