#include "login_handler.h"
#include "handler_registry.h"
#include "session_middleware_handler.h"
#include <fstream>
#include <sstream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/log/trivial.hpp>

const std::string LoginHandler::kName = "LoginHandler";

LoginHandler::LoginHandler(const std::string& path, bool testing)
    : path_(path) {
    user_db_file_ = testing ? "../data/users_test.json" : "../data/users.json";
    BOOST_LOG_TRIVIAL(info) << "LoginHandler using user DB: " << user_db_file_;
}

std::string LoginHandler::hash_password(const std::string& password) {
    std::hash<std::string> hasher;
    return std::to_string(hasher(password)); 
}

std::unique_ptr<HttpResponse> LoginHandler::handle_request(const HttpRequest& request) {
    auto res = std::make_unique<HttpResponse>();
    res->headers["Content-Type"] = "text/plain";
    
    if (request.method != "POST") {
        res->status_code = 405;
        res->body = "Method Not Allowed";
        return res;
    }

    std::string username, password;
    try {
        auto json = nlohmann::json::parse(request.body);
        username = json.at("username").get<std::string>();
        password = json.at("password").get<std::string>();
    } catch (...) {
        res->status_code = 400;
        res->body = "Invalid request format";
        return res;
    }

    if (!credentials_valid(username, password)) {
        res->status_code = 401;
        res->body = "Invalid credentials";
        return res;
    }

    // On success: simply echo back the user ID (username) in the body.
    // The SessionMiddlewareHandler will see path=="/login" && status_code==200,
    // call create_session(username), and set Set-Cookie.

    res->status_code = 200;
    res->body = username;
    return res;
}

bool LoginHandler::credentials_valid(const std::string& username, const std::string& password) {
    BOOST_LOG_TRIVIAL(debug) << "Checking credentials in file: " << user_db_file_;

    std::ifstream in(user_db_file_);
    if (!in.is_open()) {
        BOOST_LOG_TRIVIAL(error) << "Failed to open user DB file: " << user_db_file_;
        return false;
    }

    try {
        nlohmann::json users = nlohmann::json::parse(in);
        return users.contains(username) && users[username] == hash_password(password);
    } catch (...) {
        return false;
    }
}

static const bool loginRegistered =
    HandlerRegistry::instance().registerHandler(
        LoginHandler::kName,
        [](const auto& args) {
            bool testing = (args.size() > 1 && args[1] == "true");
            
            auto realLogin = std::make_unique<LoginHandler>(args.at(0), testing);
            
            // Wrapped in SessionMiddleware
            return std::make_unique<SessionMiddlewareHandler>(std::move(realLogin));
        }
    );