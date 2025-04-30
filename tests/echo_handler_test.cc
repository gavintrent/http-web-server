#include <boost/lexical_cast.hpp>

#include <gtest/gtest.h>
#include "echo_handler.h"
#include "request_parser.h"
#include <boost/system/error_code.hpp>
#include "http_types.h" 

// test simple echo handler
TEST(EchoHandlerTest, ReturnsRequestAsBody) {
    std::string raw_request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    EchoHandler handler;
    RequestParser parser;
    boost::system::error_code ec;
    HttpRequest req = parser.parse(raw_request.c_str(), raw_request.size(), ec);
    HttpResponse response = handler.handleRequest(req);
    
    EXPECT_TRUE(response.status_code == 200);
    EXPECT_TRUE(response.headers["Content-Type"] == "text/plain");
    EXPECT_EQ(response.body, raw_request);
}

// test response contains correct content-length header
TEST(EchoHandlerTest, CorrectContentLength) {
    std::string raw_request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    EchoHandler handler;
    RequestParser parser;
    boost::system::error_code ec;
    HttpRequest req = parser.parse(raw_request.c_str(), raw_request.size(), ec);
    HttpResponse response = handler.handleRequest(req);
    // Extract the Content-Length value from the response
    EXPECT_TRUE(response.headers["Content-Length"] == std::to_string(raw_request.size()));
}

// // test special characters in request
TEST(EchoHandlerTest, HandlesSpecialCharacters) {
    std::string raw_request = "GET /search?q=hello%20world&lang=en HTTP/1.1\r\nHost: localhost\r\n\r\n";
    EchoHandler handler;
    RequestParser parser;
    boost::system::error_code ec;
    HttpRequest req = parser.parse(raw_request.c_str(), raw_request.size(), ec);
    HttpResponse response = handler.handleRequest(req);
    
    EXPECT_TRUE(response.status_code == 200);
    EXPECT_EQ(raw_request, response.body);
}

// test non-GET request
TEST(EchoHandlerTest, NonGETRequest) {
    std::string raw_request = "POST / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    EchoHandler handler;
    RequestParser parser;
    boost::system::error_code ec;
    HttpRequest req = parser.parse(raw_request.c_str(), raw_request.size(), ec);
    HttpResponse response = handler.handleRequest(req);
    
    EXPECT_TRUE(response.status_code == 400);
}
