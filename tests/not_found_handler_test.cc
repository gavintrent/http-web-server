#include <gtest/gtest.h>
#include <memory>

#include "not_found_handler.h"
#include "http_types.h"   

TEST(NotFoundHandlerTest, Returns404) {
    NotFoundHandler handler;
    HttpRequest req;

    auto res = handler.handle_request(req);
    ASSERT_TRUE(res);

    EXPECT_EQ(res->status_code, 404);
    EXPECT_TRUE(res->body.empty());
    EXPECT_EQ(res->headers["Content-Type"],   "text/plain");
    EXPECT_EQ(res->headers["Content-Length"], "0");
}

TEST(NotFoundHandlerTest, IgnoresContent) {
    NotFoundHandler handler;
    HttpRequest req;

    req.method  = "POST";
    req.path    = "/some/path";
    req.body    = "this should not matter";

    auto res = handler.handle_request(req);
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status_code, 404);
    EXPECT_TRUE(res->body.empty());
    EXPECT_EQ(res->headers["Content-Type"],   "text/plain");
    EXPECT_EQ(res->headers["Content-Length"], "0");
}

TEST(NotFoundHandlerTest, ReturnsDifferentInstances) {
    NotFoundHandler handler;
    HttpRequest req;

    auto r1 = handler.handle_request(req);
    auto r2 = handler.handle_request(req);
    ASSERT_TRUE(r1);
    ASSERT_TRUE(r2);
    EXPECT_NE(r1.get(), r2.get());
}