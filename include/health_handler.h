#ifndef HEALTH_HANDLER_H
#define HEALTH_HANDLER_H

#include "request_handler.h"
#include "http_types.h"
#include "config_parser.h"
#include <memory>
#include <string>

// request handler that returns a 200 OK response to check if the server is running
class HealthHandler : public RequestHandler {
public:
    HealthHandler() = default;
    ~HealthHandler() override = default;

    void Init(const std::string& uri_prefix, const NginxConfig& config);

    std::unique_ptr<HttpResponse> handle_request(const HttpRequest& request) override;

    static const std::string kName;
    std::string get_kName() { return kName; };
private:
    std::string uri_prefix_;
};

#endif // HEALTH_HANDLER_H