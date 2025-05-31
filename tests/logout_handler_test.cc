#include <gtest/gtest.h>
#include "logout_handler.h"

class LogoutHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        handler = std::make_unique<LogoutHandler>("/logout");
    }

    std::unique_ptr<LogoutHandler> handler;
};

TEST_F(LogoutHandlerTest, NoSessionCookie) {
    HttpRequest request;
    auto response = handler->handle_request(request);
    
    EXPECT_EQ(response->status_code, 200);
    EXPECT_EQ(response->headers["Content-Type"], "text/plain");
    EXPECT_EQ(response->headers["Set-Cookie"], "session=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT; HttpOnly");
    EXPECT_EQ(response->body, "Logged out successfully");
}

TEST_F(LogoutHandlerTest, WithSessionCookie) {
    HttpRequest request;
    request.headers["Cookie"] = "session=test_session_token";
    
    auto response = handler->handle_request(request);
    
    EXPECT_EQ(response->status_code, 200);
    EXPECT_EQ(response->headers["Content-Type"], "text/plain");
    EXPECT_EQ(response->headers["Set-Cookie"], "session=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT; HttpOnly");
    EXPECT_EQ(response->body, "Logged out successfully");
}

TEST_F(LogoutHandlerTest, InvalidatesSession) {
    HttpRequest request;
    request.headers["Cookie"] = "session=test_session_token";
    
    auto response = handler->handle_request(request);
    
    EXPECT_EQ(response->status_code, 200);
    EXPECT_EQ(response->headers["Content-Type"], "text/plain");
    EXPECT_EQ(response->headers["Set-Cookie"], "session=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT; HttpOnly");
    EXPECT_EQ(response->body, "Logged out successfully");
}
