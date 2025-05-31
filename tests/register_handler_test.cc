#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include "register_handler.h"
#include "http_types.h"
#include <boost/log/trivial.hpp>

class RegisterHandlerTestFixture : public ::testing::Test {
protected:
    std::string test_db_path = "../data/test_users.json"; // test db
    std::unique_ptr<RegisterHandler> handler;

    void SetUp() override {
        std::ofstream(test_db_path) << "{}"; 
        handler = std::make_unique<RegisterHandler>("/register", true);
    }

    void TearDown() override {
        std::remove(test_db_path.c_str());
    }

    HttpRequest makePostRequest(const std::string& body) {
        HttpRequest req;
        req.method = "POST";
        req.path = "/register";
        req.body = body;
        return req;
    }
};

TEST_F(RegisterHandlerTestFixture, RejectsNonPostMethod) {
    HttpRequest req;
    req.method = "GET";
    req.path = "/register";

    auto res = handler->handle_request(req);
    EXPECT_EQ(res->status_code, 405);
    EXPECT_EQ(res->body, "Method Not Allowed");
}

TEST_F(RegisterHandlerTestFixture, RejectsMalformedJSON) {
    HttpRequest req = makePostRequest("{\"username\": ");
    auto res = handler->handle_request(req);
    EXPECT_EQ(res->status_code, 400);
    EXPECT_EQ(res->body, "Invalid request format");
}

TEST_F(RegisterHandlerTestFixture, RegistersNewUserSuccessfully) {
    HttpRequest req = makePostRequest("{\"username\":\"alice\",\"password\":\"secret\"}");
    auto res = handler->handle_request(req);

    BOOST_LOG_TRIVIAL(debug) << res->body;

    EXPECT_EQ(res->status_code, 200);
    EXPECT_EQ(res->body, "Registration successful");

    std::ifstream in(test_db_path);
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_NE(contents.find("alice"), std::string::npos);
}

TEST_F(RegisterHandlerTestFixture, RejectsDuplicateUser) {
    HttpRequest first = makePostRequest("{\"username\":\"bob\",\"password\":\"pass123\"}");
    handler->handle_request(first);

    HttpRequest second = makePostRequest("{\"username\":\"bob\",\"password\":\"pass123\"}");
    auto res = handler->handle_request(second);

    EXPECT_EQ(res->status_code, 400);
    EXPECT_EQ(res->body, "Username already exists");
}