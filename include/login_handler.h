#pragma once

#include "http_types.h"
#include "request_handler.h"
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

class LoginHandler : public RequestHandler {
public:
    static const std::string kName;

    LoginHandler(const std::string& path, bool testing = false);

    std::unique_ptr<HttpResponse> handle_request(const HttpRequest& request) override;

    std::string get_kName() override { return kName; }

private:
    std::string path_;
    std::string user_db_file_;

    std::string hash_password(const std::string& password);
    bool credentials_valid(const std::string& username, const std::string& password);
};
