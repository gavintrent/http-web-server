#include <boost/lexical_cast.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "echo_handler.h"
#include "request_parser.h"
#include <boost/system/error_code.hpp>
#include "http_types.h" 

using ::testing::_;
using ::testing::Ref;
using ::testing::Return;

static HttpRequest makeRequest(const std::string& method = "GET",
                               const std::string& path   = "/",
                               const std::string& body   = "",
                               const std::vector<std::pair<std::string,std::string>>& hdrs = {{"Host","localhost"}})
{
    HttpRequest req;
    req.method  = method;
    req.path    = path;
    req.headers.clear();
    for (auto &h : hdrs) req.headers[h.first] = h.second;
    req.body    = body;

    std::ostringstream ss;
    ss << method << " " << path << " HTTP/1.1\r\n";
    for (auto &h : hdrs) ss << h.first << ": " << h.second << "\r\n";
    ss << "\r\n" << body;
    req.raw = ss.str();

    return req;
}

struct MockEchoHandler : public EchoHandler {
  MOCK_METHOD(HttpResponse, doEcho, (const HttpRequest&), (override));
};

TEST(EchoHandlerTest, HandleGetAndReturnResponseMock) {
    MockEchoHandler mock;
    HttpRequest req = makeRequest("GET", "/test", "hello");

    HttpResponse fakeResp;
    fakeResp.status_code = 200;
    fakeResp.headers = {
        {"Content-Type", "text/plain"},
        {"Content-Length", std::to_string(req.raw.size())}
    };
    fakeResp.body = req.raw;

    EXPECT_CALL(mock, doEcho(Ref(req)))
        .WillOnce(Return(fakeResp));

    HttpResponse out = mock.handleRequest(req);
    EXPECT_EQ(out.status_code, fakeResp.status_code);
    EXPECT_EQ(out.headers,     fakeResp.headers);
    EXPECT_EQ(out.body,        fakeResp.body);
}

TEST(EchoHandlerTest, HandleNonGET_DoesNotCallEchoImplAndReturns400) {
    MockEchoHandler mock;
    HttpRequest req = makeRequest("POST", "/");
    EXPECT_CALL(mock, doEcho(_)).Times(0);

    HttpResponse out = mock.handleRequest(req);
    EXPECT_EQ(out.status_code, 400);
}

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