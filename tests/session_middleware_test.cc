#include "session_middleware_handler.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class MockRequestHandler : public RequestHandler {
public:
    MOCK_METHOD(std::unique_ptr<HttpResponse>, handle_request, (const HttpRequest&), (override));
    std::string get_kName() override { return "MockRequestHandler"; }
};

class SessionMiddlewareHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a mock next handler
        auto tmp = std::make_unique<MockRequestHandler>();
        raw_handler = tmp.get();
        middleware = std::make_unique<SessionMiddlewareHandler>(std::move(tmp));
    }

    void TearDown() override {
        middleware.reset();
    }

    MockRequestHandler* raw_handler = nullptr;
    std::unique_ptr<SessionMiddlewareHandler> middleware;
};

TEST_F(SessionMiddlewareHandlerTest, NoSessionToken) {
    HttpRequest request;
    request.path = "/test";
    
    auto expected_response = std::make_unique<HttpResponse>();
    expected_response->status_code = 200;
    
    EXPECT_CALL(*raw_handler, handle_request(testing::_))
        .WillOnce(testing::Return(std::move(expected_response)));
    
    auto response = middleware->handle_request(request);
    EXPECT_EQ(response->status_code, 200);
    EXPECT_FALSE(request.session_context.session_token.has_value());
    EXPECT_FALSE(request.session_context.user_id.has_value());
}

TEST_F(SessionMiddlewareHandlerTest, ValidSessionToken) {
    HttpRequest request;
    request.path = "/test";
    request.headers["Cookie"] = "session=valid_token";
    
    auto expected_response = std::make_unique<HttpResponse>();
    expected_response->status_code = 200;
    
    EXPECT_CALL(*raw_handler, handle_request(testing::_))
        .WillOnce(testing::Return(std::move(expected_response)));
    
    auto response = middleware->handle_request(request);
    EXPECT_EQ(response->status_code, 200);
}

TEST_F(SessionMiddlewareHandlerTest, LoginSuccess) {
    HttpRequest request;
    request.path = "/login";
    
    auto expected_response = std::make_unique<HttpResponse>();
    expected_response->status_code = 200;
    expected_response->body = "user123";
    
    EXPECT_CALL(*raw_handler, handle_request(testing::_))
        .WillOnce(testing::Return(std::move(expected_response)));
    
    auto response = middleware->handle_request(request);
    EXPECT_EQ(response->status_code, 200);
    EXPECT_TRUE(response->headers.find("Set-Cookie") != response->headers.end());
    EXPECT_TRUE(response->headers["Set-Cookie"].find("session=") != std::string::npos);
    EXPECT_TRUE(response->headers["Set-Cookie"].find("HttpOnly") != std::string::npos);
    EXPECT_TRUE(response->headers["Set-Cookie"].find("Path=/") != std::string::npos);
}

TEST_F(SessionMiddlewareHandlerTest, LogoutSuccess) {
    HttpRequest request;
    request.path = "/logout";
    request.headers["Cookie"] = "session=valid_token";
    
    auto expected_response = std::make_unique<HttpResponse>();
    expected_response->status_code = 200;
    
    EXPECT_CALL(*raw_handler, handle_request(testing::_))
        .WillOnce(testing::Return(std::move(expected_response)));
    
    auto response = middleware->handle_request(request);
    EXPECT_EQ(response->status_code, 200);
    EXPECT_TRUE(response->headers.find("Set-Cookie") != response->headers.end());
    EXPECT_TRUE(response->headers["Set-Cookie"].find("session=;") != std::string::npos);
    EXPECT_TRUE(response->headers["Set-Cookie"].find("Max-Age=0") != std::string::npos);
    EXPECT_TRUE(response->headers["Set-Cookie"].find("HttpOnly") != std::string::npos);
    EXPECT_TRUE(response->headers["Set-Cookie"].find("Path=/") != std::string::npos);
}

TEST_F(SessionMiddlewareHandlerTest, BearerToken) {
    HttpRequest request;
    request.path = "/test";
    request.headers["Authorization"] = "Bearer valid_token";
    
    auto expected_response = std::make_unique<HttpResponse>();
    expected_response->status_code = 200;
    
    EXPECT_CALL(*raw_handler, handle_request(testing::_))
        .WillOnce(testing::Return(std::move(expected_response)));
    
    auto response = middleware->handle_request(request);
    EXPECT_EQ(response->status_code, 200);
}

TEST_F(SessionMiddlewareHandlerTest, InvalidBearerToken) {
    HttpRequest request;
    request.path = "/test";
    request.headers["Authorization"] = "Invalid valid_token";
    
    auto expected_response = std::make_unique<HttpResponse>();
    expected_response->status_code = 200;
    
    EXPECT_CALL(*raw_handler, handle_request(testing::_))
        .WillOnce(testing::Return(std::move(expected_response)));
    
    auto response = middleware->handle_request(request);
    EXPECT_EQ(response->status_code, 200);
    EXPECT_FALSE(request.session_context.session_token.has_value());
}

TEST_F(SessionMiddlewareHandlerTest, ExpiredSession) {
    HttpRequest request;
    request.path = "/test";
    request.headers["Cookie"] = "session=expired_token";
    
    auto expected_response = std::make_unique<HttpResponse>();
    expected_response->status_code = 200;
    
    EXPECT_CALL(*raw_handler, handle_request(testing::_))
        .WillOnce(testing::Return(std::move(expected_response)));
    
    auto response = middleware->handle_request(request);
    EXPECT_EQ(response->status_code, 200);
    EXPECT_FALSE(request.session_context.session_token.has_value());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
