#include "register_handler.h"
#include "handler_registry.h"
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include <boost/log/trivial.hpp>

using json = nlohmann::json;

const std::string RegisterHandler::kName = "RegisterHandler";

RegisterHandler::RegisterHandler(const std::string& path, bool testing)
    : path_(path) {
    user_db_file_ = testing ? "../data/test_users.json" : "../data/users.json";
    BOOST_LOG_TRIVIAL(info) << "RegisterHandler using user DB: " << user_db_file_;
}

std::string RegisterHandler::hash_password(const std::string& password) {
    std::hash<std::string> hasher;
    return std::to_string(hasher(password));  // Simple hashing
}

bool RegisterHandler::username_exists(const std::string& username) {
    BOOST_LOG_TRIVIAL(debug) << "Checking username in file: " << user_db_file_;
    std::ifstream file(user_db_file_);
    if (!file.is_open()) {
        BOOST_LOG_TRIVIAL(error) << "Failed to open file: " << user_db_file_;
        return true;
    }

    json users;
    try {
        file >> users;
    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(warning) << "Failed to parse users.json: " << e.what();
        return true; 
    }
    return users.contains(username);
}

bool RegisterHandler::save_user(const std::string& username, const std::string& hashed_pw) {
    json users;
    {
        std::ifstream file(user_db_file_);
        if (file.is_open()) {
            try {
                file >> users;
            } catch (const std::exception& e) {
                BOOST_LOG_TRIVIAL(warning) << "Failed to parse users.json: " << e.what();
            }
        }
    }

    users[username] = hashed_pw;

    std::ofstream file(user_db_file_);
    if (!file.is_open()) return false;
    file << users.dump(2);
    return true;
}


std::unique_ptr<HttpResponse> RegisterHandler::handle_request(const HttpRequest& request) {
    auto response = std::make_unique<HttpResponse>();
    response->headers["Content-Type"] = "text/plain";

    if (request.method != "POST") {
        response->status_code = 405;
        response->body = "Method Not Allowed";
        return response;
    }

    try {
        auto data = json::parse(request.body);
        std::string username = data.at("username");
        std::string password = data.at("password");

        if (username_exists(username)) {
            response->status_code = 400;
            response->body = "Username already exists";
            return response;
        }

        std::string hashed_pw = hash_password(password);
        if (save_user(username, hashed_pw)) {
            response->status_code = 200;
            response->body = "Registration successful";
        } else {
            response->status_code = 500;
            response->body = "Internal server error";
        }

    } catch (const std::exception& e) {
        response->status_code = 400;
        response->body = "Invalid request format";
    }

    return response;
}

static const bool registerRegistered =
    HandlerRegistry::instance().registerHandler(
        RegisterHandler::kName,
        [](const auto& args) {
            return std::make_unique<RegisterHandler>(args.at(0));
        }
    );