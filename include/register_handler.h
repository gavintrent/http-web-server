#pragma once
#include "http_types.h"
#include "request_handler.h"
#include <string>

class RegisterHandler : public RequestHandler {
public:
    static const std::string kName;

    RegisterHandler(const std::string& path, bool testing = false); // optional testing flag to change db

    std::unique_ptr<HttpResponse> handle_request(const HttpRequest& request) override;

    std::string get_kName() override { return kName; }

private:
    std::string path_;
    std::string user_db_file_;

    std::string hash_password(const std::string& password);
    bool username_exists(const std::string& username);
    bool save_user(const std::string& username, const std::string& hashed_pw);
};
