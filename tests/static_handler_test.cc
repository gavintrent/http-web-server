#include <boost/lexical_cast.hpp>

#include <gtest/gtest.h>
#include "static_handler.h"
#include "request_parser.h"
#include <boost/system/error_code.hpp>
#include "http_types.h" 

// test getting html
TEST(StaticHandlerTest, GetHTML) {
    std::string raw_request = "GET /static_files/test.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    StaticHandler handler;
    RequestParser parser;
    boost::system::error_code ec;
    HttpRequest req = parser.parse(raw_request.c_str(), raw_request.size(), ec);
    HttpResponse response = handler.handleRequest(req);
    
    EXPECT_TRUE(response.status_code == 200);
    EXPECT_TRUE(response.headers["Content-Type"] == "text/html");
}

// test getting jpeg
TEST(StaticHandlerTest, GetJPEG) {
    std::string raw_request = "GET /static_files/test.jpg HTTP/1.1\r\nHost: localhost\r\n\r\n";
    StaticHandler handler;
    RequestParser parser;
    boost::system::error_code ec;
    HttpRequest req = parser.parse(raw_request.c_str(), raw_request.size(), ec);
    HttpResponse response = handler.handleRequest(req);
    
    EXPECT_TRUE(response.status_code == 200);
    EXPECT_TRUE(response.headers["Content-Type"] == "image/jpeg");
}

// test getting txt
TEST(StaticHandlerTest, GetTXT) {
    std::string raw_request = "GET /static_files/test.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";
    StaticHandler handler;
    RequestParser parser;
    boost::system::error_code ec;
    HttpRequest req = parser.parse(raw_request.c_str(), raw_request.size(), ec);
    HttpResponse response = handler.handleRequest(req);
    
    EXPECT_TRUE(response.status_code == 200);
    EXPECT_TRUE(response.headers["Content-Type"] == "text/plain");
}

// test getting zip
TEST(StaticHandlerTest, GetZIP) {
    std::string raw_request = "GET /static_files/test.zip HTTP/1.1\r\nHost: localhost\r\n\r\n";
    StaticHandler handler;
    RequestParser parser;
    boost::system::error_code ec;
    HttpRequest req = parser.parse(raw_request.c_str(), raw_request.size(), ec);
    HttpResponse response = handler.handleRequest(req);
    
    EXPECT_TRUE(response.status_code == 200);
    EXPECT_TRUE(response.headers["Content-Type"] == "application/zip");
}

// test non-GET request
TEST(StaticHandlerTest, NonGETRequest) {
    std::string raw_request = "POST / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    StaticHandler handler;
    RequestParser parser;
    boost::system::error_code ec;
    HttpRequest req = parser.parse(raw_request.c_str(), raw_request.size(), ec);
    HttpResponse response = handler.handleRequest(req);
    
    EXPECT_TRUE(response.status_code == 400);
}

// test bad target path
TEST(StaticHandlerTest, BadPath) {
    std::string raw_request = "GET /bad_path HTTP/1.1\r\nHost: localhost\r\n\r\n";
    StaticHandler handler;
    RequestParser parser;
    boost::system::error_code ec;
    HttpRequest req = parser.parse(raw_request.c_str(), raw_request.size(), ec);
    HttpResponse response = handler.handleRequest(req);
    
    EXPECT_TRUE(response.status_code == 404);
}
