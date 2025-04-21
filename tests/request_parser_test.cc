#include <gtest/gtest.h>
#include "request_parser.h"
#include "http_types.h"
#include <boost/system/error_code.hpp>
#include <cstring>

// test the RequestParser class
TEST(RequestParserTest, ParsesValidGET) {
  const char raw[] =
    "GET /foo/bar HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "X-Custom: 123\r\n"
    "\r\n"
    "body-data";
  boost::system::error_code ec;
  RequestParser parser;
  HttpRequest req = parser.parse(raw, std::strlen(raw), ec);

  ASSERT_FALSE(ec) << "Parser returned error: " << ec.message();
  EXPECT_EQ(req.method, "GET");
  EXPECT_EQ(req.path, "/foo/bar");
  EXPECT_EQ(req.headers["Host"], "example.com");
  EXPECT_EQ(req.headers["X-Custom"], "123");
  EXPECT_EQ(req.body, "body-data");
}

// test the RequestParser class with a malformed request
TEST(RequestParserTest, ReportsErrorOnMalformed) {
  const char raw[] = "BADREQUEST /nope\r\n\r\n";
  boost::system::error_code ec;
  RequestParser parser;
  HttpRequest req = parser.parse(raw, std::strlen(raw), ec);

  EXPECT_TRUE(ec) << "Expected a parse error on malformed input";
}

// test version mismatch
TEST(RequestParserTest, HandlesVersionMismatch) {
  const char raw[] =
    "GET /foo/bar HTTP/2.0\r\n"
    "Host: example.com\r\n"
    "\r\n";
  boost::system::error_code ec;
  RequestParser parser;
  HttpRequest req = parser.parse(raw, std::strlen(raw), ec);

  ASSERT_FALSE(ec) << "Parser returned error: " << ec.message();
  EXPECT_EQ(req.method, "GET");
  EXPECT_EQ(req.path, "/foo/bar");
  EXPECT_EQ(req.headers["Host"], "example.com");
  EXPECT_EQ(req.body, "");
}

// test missing headers
TEST(RequestParserTest, HandlesMissingHeaders) {
  const char raw[] =
    "GET /foo/bar HTTP/1.1\r\n"
    "\r\n";
  boost::system::error_code ec;
  RequestParser parser;
  HttpRequest req = parser.parse(raw, std::strlen(raw), ec);

  ASSERT_FALSE(ec) << "Parser returned error: " << ec.message();
  EXPECT_EQ(req.method, "GET");
  EXPECT_EQ(req.path, "/foo/bar");
  EXPECT_TRUE(req.headers.empty());
  EXPECT_EQ(req.body, "");
}

// test empty request
TEST(RequestParserTest, HandlesEmptyRequest) {
  const char raw[] = "";
  boost::system::error_code ec;
  RequestParser parser;
  HttpRequest req = parser.parse(raw, std::strlen(raw), ec);

  EXPECT_TRUE(ec) << "Expected a parse error on empty input";
}

