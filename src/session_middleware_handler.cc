#include "session_middleware_handler.h"
#include "session_store.h"
#include <boost/log/trivial.hpp>
#include <regex>
#include <optional>
#include <string>

const std::regex SessionMiddlewareHandler::session_cookie_regex("session=([^;]+)");

SessionMiddlewareHandler::SessionMiddlewareHandler(std::unique_ptr<RequestHandler> next_handler)
    : next_handler_(std::move(next_handler)) {}

std::unique_ptr<HttpResponse> SessionMiddlewareHandler::handle_request(const HttpRequest& request) {
    HttpRequest modified_request = request; // mutable copy of the request
    modified_request.session_context.clear(); // clear previous session context
    auto session_token = extract_session_token(request); // extract and set session token

    if (session_token) {
        // get session data from store
        auto session_data = SessionStore::get_instance().get_session(*session_token);
        if (session_data) {
            modified_request.session_context.session_token = *session_token;
            modified_request.session_context.user_id = session_data->user_id;
            modified_request.session_context.data = session_data->data;
        }
    }

    // process the request with the next handler
    auto response = next_handler_->handle_request(modified_request);

    // if this is a login response and it was successful, create a new session
    if (request.path == "/login" && response->status_code == 200) {
        std::string user_id = response->body;
        std::string new_token = SessionStore::get_instance().create_session(user_id);
        
        // add session cookie to response with domain
        response->headers["Set-Cookie"] = "session=" + new_token + "; HttpOnly; Path=/;";
    }
    // if this is a logout response, invalidate the session
    else if (request.path == "/logout" && session_token) {
        SessionStore::get_instance().invalidate_session(*session_token);
        response->headers["Set-Cookie"] = "session=; HttpOnly; Path=/; Max-Age=0";
    }

    return response;
}

std::optional<std::string> SessionMiddlewareHandler::extract_session_token(const HttpRequest& request) {
    // get from cookie header
    auto cookie_it = request.headers.find("Cookie");
    if (cookie_it != request.headers.end()) {
        std::smatch matches;
        if (std::regex_search(cookie_it->second, matches, session_cookie_regex)) {
            return matches[1].str();
        }
    }

    // then get from authorization header
    auto auth_it = request.headers.find("Authorization");
    if (auth_it != request.headers.end()) {
        // check if it's a Bearer token
        if (auth_it->second.substr(0, 7) == "Bearer ") {
            return auth_it->second.substr(7);
        }
    }

    return std::nullopt;
} 
