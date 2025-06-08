#include <boost/lexical_cast.hpp>

#include <gtest/gtest.h>
#include "static_handler.h"
#include "request_parser.h"
#include <boost/system/error_code.hpp>
#include "http_types.h" 

class StaticHandlerTestFixture : public ::testing::Test {
protected:
    StaticHandler handler;

    StaticHandlerTestFixture()
        : handler("/static", "/static_files") {}

    HttpRequest makeRequest(const std::string& method, const std::string& path) {
        HttpRequest req;
        req.method = method;
        req.path = path;
        return req;
    }
};

TEST_F(StaticHandlerTestFixture, GetHTML) {
    HttpRequest req = makeRequest("GET", "/static/test.html");
    std::unique_ptr<HttpResponse> response = handler.handle_request(req);
    
    EXPECT_TRUE(response->status_code == 200);
    EXPECT_TRUE(response->headers["Content-Type"] == "text/html");
}

// test getting jpeg
TEST_F(StaticHandlerTestFixture, GetJPEG) {
    HttpRequest req = makeRequest("GET", "/static/test.jpg");
    std::unique_ptr<HttpResponse> response = handler.handle_request(req);
    
    EXPECT_TRUE(response->status_code == 200);
    EXPECT_TRUE(response->headers["Content-Type"] == "image/jpeg");
}

// test getting txt
TEST_F(StaticHandlerTestFixture, GetTXT) {
    HttpRequest req = makeRequest("GET", "/static/test.txt");
    std::unique_ptr<HttpResponse> response = handler.handle_request(req);
    
    EXPECT_TRUE(response->status_code == 200);
    EXPECT_TRUE(response->headers["Content-Type"] == "text/plain");
}

// test getting zip
TEST_F(StaticHandlerTestFixture, GetZIP) {
    HttpRequest req = makeRequest("GET", "/static/test.zip");
    std::unique_ptr<HttpResponse> response = handler.handle_request(req);
    
    EXPECT_TRUE(response->status_code == 200);
    EXPECT_TRUE(response->headers["Content-Type"] == "application/zip");
}

// test non-GET request
TEST_F(StaticHandlerTestFixture, NonGETRequest) {
    HttpRequest req = makeRequest("POST", "/static/test.html");
    std::unique_ptr<HttpResponse> response = handler.handle_request(req);
    
    EXPECT_TRUE(response->status_code == 400);
}

// test bad target path
TEST_F(StaticHandlerTestFixture, BadPath) {
    HttpRequest req = makeRequest("GET", "/static/bad_path");
    std::unique_ptr<HttpResponse> response = handler.handle_request(req);
    
    EXPECT_TRUE(response->status_code == 404);
}
