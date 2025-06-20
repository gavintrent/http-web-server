#include <boost/lexical_cast.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "echo_handler.h"
#include "request_parser.h"
#include <boost/system/error_code.hpp>
#include "http_types.h" 
#include <memory>

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

class DoEchoTest : public ::testing::Test {
protected:
  struct TestEcho : public EchoHandler {
    TestEcho() : EchoHandler("/test") {}
    using EchoHandler::doEcho;
  } handler;

  HttpRequest makeReq(const std::string& raw = "") {
    HttpRequest req;
    req.raw = raw;
    return req;
  }
};

struct MockEchoHandler : public EchoHandler {
  MockEchoHandler() : EchoHandler("/mock") {}
  MOCK_METHOD(std::unique_ptr<HttpResponse>, doEcho, (const HttpRequest&), (override));
};

TEST(EchoHandlerTest, HandleGetAndReturnResponseMock) {
    MockEchoHandler mock;
    HttpRequest req = makeRequest("GET", "/test", "hello");

    auto fakeResp = std::make_unique<HttpResponse>();
    fakeResp->status_code = 200;
    fakeResp->headers = {
        {"Content-Type", "text/plain"},
        {"Content-Length", std::to_string(req.raw.size())}
    };
    fakeResp->body = req.raw;

    EXPECT_CALL(mock, doEcho(Ref(req)))
         .WillOnce(Return(testing::ByMove(std::move(fakeResp))));

    std::unique_ptr<HttpResponse> out = mock.handle_request(req);
    EXPECT_EQ(out->status_code, 200);
    EXPECT_EQ(out->headers, (std::map<std::string,std::string>{{"Content-Type", "text/plain"},{"Content-Length", std::to_string(req.raw.size())}}));
    EXPECT_EQ(out->body,        req.raw);
    EXPECT_EQ(out->body.size(), req.raw.size());

}

TEST(EchoHandlerTest, HandleNonGETAndReturns400Mock) {
    MockEchoHandler mock;
    HttpRequest req = makeRequest("POST", "/");
    EXPECT_CALL(mock, doEcho(_)).Times(0);

    std::unique_ptr<HttpResponse> out = mock.handle_request(req);
    EXPECT_EQ(out->status_code, 400);
}

// test special characters in request
TEST(EchoHandlerTest, HandleGetSpecialCharacterAndReturnResponseMock) {
    MockEchoHandler mock;
    HttpRequest req = makeRequest("GET", "/search", "?q=hello%20world&lang=en");

    auto fakeResp = std::make_unique<HttpResponse>();
    fakeResp->status_code = 200;
    fakeResp->headers = {
        {"Content-Type", "text/plain"},
        {"Content-Length", std::to_string(req.raw.size())}
    };
    fakeResp->body = req.raw;

    EXPECT_CALL(mock, doEcho(Ref(req)))
        .WillOnce(Return(testing::ByMove(std::move(fakeResp))));

    std::unique_ptr<HttpResponse> out = mock.handle_request(req);
    EXPECT_EQ(out->status_code, 200);
    EXPECT_EQ(out->headers, (std::map<std::string,std::string>{{"Content-Type", "text/plain"},{"Content-Length", std::to_string(req.raw.size())}}));
    EXPECT_EQ(out->body,        req.raw);
    EXPECT_EQ(out->body.size(), req.raw.size());

}

TEST(EchoHandlerTest, HandleEmptyAndReturnResponseMock) {
    MockEchoHandler mock;
    HttpRequest req = makeRequest("GET", "/test", "");

    auto fakeResp = std::make_unique<HttpResponse>();
    fakeResp->status_code = 200;
    fakeResp->headers = {
        {"Content-Type", "text/plain"},
        {"Content-Length", std::to_string(req.raw.size())}
    };
    fakeResp->body = req.raw;

    EXPECT_CALL(mock, doEcho(Ref(req)))
        .WillOnce(Return(testing::ByMove(std::move(fakeResp))));

    std::unique_ptr<HttpResponse> out = mock.handle_request(req);
    EXPECT_EQ(out->status_code, 200);
    EXPECT_EQ(out->headers, (std::map<std::string,std::string>{{"Content-Type", "text/plain"},{"Content-Length", std::to_string(req.raw.size())}}));
    EXPECT_EQ(out->body,        req.raw);
    EXPECT_EQ(out->body.size(), req.raw.size());

}

TEST_F(DoEchoTest, BasicEcho) {
  const std::string raw = "GET /foo/bar HTTP/1.1\r\nHost: localhost\r\n\r\nhello";
  std::unique_ptr<HttpResponse> resp = handler.doEcho(makeReq(raw));

  EXPECT_EQ(resp->status_code, 200);
  EXPECT_EQ(resp->body,        raw);
  EXPECT_EQ(resp->headers["Content-Type"],   "text/plain");
  EXPECT_EQ(resp->headers["Content-Length"], std::to_string(raw.size()));
}


TEST_F(DoEchoTest, EmptyRaw) {
  std::unique_ptr<HttpResponse> resp = handler.doEcho(makeReq(""));

  EXPECT_EQ(resp->status_code, 200);
  EXPECT_EQ(resp->body,        "");
  EXPECT_EQ(resp->headers["Content-Type"],   "text/plain");
  EXPECT_EQ(resp->headers["Content-Length"], "0");
}