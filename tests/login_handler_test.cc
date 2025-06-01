#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include "login_handler.h"
#include "session_middleware_handler.h"
#include "http_types.h"
#include "request_parser.h"

class LoginHandlerTestFixture : public ::testing::Test {
    protected:
        std::string test_db_path = "../data/users_test.json";
        std::unique_ptr<LoginHandler> handler;

        void SetUp() override {
            std::hash<std::string> hasher;
            std::string alice_hash = std::to_string(hasher("secret"));
            std::string bob_hash   = std::to_string(hasher("hunter2"));

            std::ofstream out(test_db_path);
            out << "{\n";
            out << "  \"alice\": \"" << alice_hash << "\",\n";
            out << "  \"bob\": \"" << bob_hash << "\"\n";
            out << "}\n";
            out.close();

            handler = std::make_unique<LoginHandler>("/login", true); // true = testing mode
        }

        void TearDown() override {
            std::remove(test_db_path.c_str());
        }

        HttpRequest makePostRequest(const std::string& body) {
            HttpRequest req;
            req.method = "POST";
            req.path = "/login";
            req.body = body;
            return req;
        }
};

TEST_F(LoginHandlerTestFixture, RejectsNonPostMethod) {
    HttpRequest req;
    req.method = "GET";
    req.path = "/login";

    auto res = handler->handle_request(req);
    EXPECT_EQ(res->status_code, 405);
    EXPECT_EQ(res->body, "Method Not Allowed");
}

TEST_F(LoginHandlerTestFixture, RejectsMalformedJSON) {
    HttpRequest req = makePostRequest("{\"username\": ");
    auto res = handler->handle_request(req);
    EXPECT_EQ(res->status_code, 400);
    EXPECT_EQ(res->body, "Invalid request format");
}

TEST_F(LoginHandlerTestFixture, RejectsMissingFields) {
    HttpRequest req = makePostRequest("{\"username\": \"alice\"}");
    auto res = handler->handle_request(req);
    EXPECT_EQ(res->status_code, 400);
    EXPECT_EQ(res->body, "Invalid request format");
}

TEST_F(LoginHandlerTestFixture, RejectsWrongPassword) {
    HttpRequest req = makePostRequest("{\"username\": \"alice\", \"password\": \"wrong\"}");
    auto res = handler->handle_request(req);
    EXPECT_EQ(res->status_code, 401);
    EXPECT_EQ(res->body, "Invalid credentials");
}

TEST_F(LoginHandlerTestFixture, RejectsNonexistentUser) {
    HttpRequest req = makePostRequest("{\"username\": \"notreal\", \"password\": \"anything\"}");
    auto res = handler->handle_request(req);
    EXPECT_EQ(res->status_code, 401);
    EXPECT_EQ(res->body, "Invalid credentials");
}

TEST_F(LoginHandlerTestFixture, ReturnsUsernameOnSuccess) {
    // Send a POST with valid credentials for “alice”
    HttpRequest req = makePostRequest("{\"username\": \"alice\", \"password\": \"secret\"}");
    auto res = handler->handle_request(req);

    EXPECT_EQ(res->status_code, 200);
    EXPECT_EQ(res->headers["Content-Type"], "text/plain");
    EXPECT_EQ(res->body, "alice");
}

TEST_F(LoginHandlerTestFixture, MiddlewareSetsSessionCookieOnSuccess) {

    std::ifstream infile("../data/test_users.json");
    if (!infile.is_open()) {
        // Create the file with {"alice": hashed("secret")}
        nlohmann::json u;
        u["alice"] = std::to_string(std::hash<std::string>()("secret"));
        std::ofstream out("../data/test_users.json");
        out << u.dump(2);
        out.close();
    }

    auto realLogin = std::make_unique<LoginHandler>("/login", /*testing=*/true);
    SessionMiddlewareHandler middleware(std::move(realLogin));

    HttpRequest req = makePostRequest("{\"username\": \"alice\", \"password\": \"secret\"}");
    req.path = "/login";

    // Act:
    auto res = middleware.handle_request(req);

    // Assert on the LoginHandler part:
    EXPECT_EQ(res->status_code, 200);
    EXPECT_EQ(res->headers["Content-Type"], "text/plain");
    EXPECT_EQ(res->body, "alice");

    // Now assert that the middleware added a Set-Cookie header:
    auto it = res->headers.find("Set-Cookie");
    ASSERT_NE(it, res->headers.end());

    std::string cookie = it->second;
    // It should start with “session=” and contain “; HttpOnly; Path=/”
    EXPECT_FALSE(cookie.find("session=") == std::string::npos);
    EXPECT_FALSE(cookie.find("; HttpOnly; Path=/") == std::string::npos);
}