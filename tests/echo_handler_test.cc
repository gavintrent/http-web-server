#include <gtest/gtest.h>
#include "echo_handler.h"

// test simple echo handler
TEST(EchoHandlerTest, ReturnsRequestAsBody) {
    std::string raw_request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    EchoHandler handler;
    std::string response = handler.HandleRequest(raw_request);

    EXPECT_TRUE(response.find("HTTP/1.1 200 OK") != std::string::npos);
    EXPECT_TRUE(response.find("Content-Type: text/plain") != std::string::npos);
    EXPECT_TRUE(response.find(raw_request) != std::string::npos);
}

// test response contains correct content-length header
TEST(EchoHandlerTest, CorrectContentLength) {
    std::string raw_request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    EchoHandler handler;
    std::string response = handler.HandleRequest(raw_request);
    
    // Extract the Content-Length value from the response
    size_t content_length_pos = response.find("Content-Length: ");
    size_t value_start = content_length_pos + 16; // "Content-Length: " is 16 chars
    size_t value_end = response.find("\r\n", value_start);
    std::string length_str = response.substr(value_start, value_end - value_start);
    int content_length = std::stoi(length_str);
    
    // The content length should be equal to the raw request length
    EXPECT_EQ(content_length, raw_request.size());
}

// test special characters in request
TEST(EchoHandlerTest, HandlesSpecialCharacters) {
    std::string raw_request = "GET /search?q=hello%20world&lang=en HTTP/1.1\r\nHost: localhost\r\n\r\n";
    EchoHandler handler;
    std::string response = handler.HandleRequest(raw_request);
    
    EXPECT_TRUE(response.find("HTTP/1.1 200 OK") != std::string::npos);
    EXPECT_TRUE(response.find(raw_request) != std::string::npos);
}