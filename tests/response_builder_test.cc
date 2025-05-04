#include <gtest/gtest.h>
#include "response_builder.h"
#include "http_types.h"

// test echo on GET
TEST(ResponseBuilderTest, EchoOnGET) {
  HttpRequest req;
  req.method = "GET";
  req.raw = "hello world";

  HttpResponse res = ResponseBuilder::echo(req);
  EXPECT_EQ(res.status_code, 200);
  EXPECT_EQ(res.body, "hello world");
  EXPECT_EQ(res.headers["Content-Type"], "text/plain");
  EXPECT_EQ(res.headers["Content-Length"], std::to_string(req.raw.size()));
}

// test empty body
TEST(ResponseBuilderTest, EmptyBodyOnGET) {
  HttpRequest req;
  req.method = "GET";
  req.raw = "";

  HttpResponse res = ResponseBuilder::echo(req);
  EXPECT_EQ(res.status_code, 200);
  EXPECT_EQ(res.body, "");
  EXPECT_EQ(res.headers["Content-Type"], "text/plain");
  EXPECT_EQ(res.headers["Content-Length"], "0");
}


// test content length header
TEST(ResponseBuilderTest, ContentLengthHeader) {
  HttpRequest req;
  req.method = "GET";
  req.raw   = "test body";

  HttpResponse res = ResponseBuilder::echo(req);
  EXPECT_EQ(res.headers["Content-Length"], std::to_string(req.raw.size()));
}

// test empty body
TEST(ResponseBuilderTest, EmptyBody) {
  HttpRequest req;
  req.method = "GET";
  req.body   = "";

  HttpResponse res = ResponseBuilder::echo(req);
  EXPECT_EQ(res.body, "");
  EXPECT_EQ(res.headers["Content-Length"], "0");
}

