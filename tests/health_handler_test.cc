// tests/health_handler_test.cc

#include "health_handler.h"
#include "http_types.h"
#include "gtest/gtest.h"
#include <sstream>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>

class HealthHandlerTest : public ::testing::Test {
 protected:
  HealthHandler h_;
  NginxConfig empty_config_;
  std::stringstream log_stream_;

  virtual void SetUp() {
    h_.Init("/health", empty_config_);
    // Set up logging to capture output
    boost::log::add_console_log(log_stream_);
  }

  virtual void TearDown() {
    boost::log::core::get()->remove_all_sinks();
  }

  bool log_contains(const std::string& pattern) {
    return log_stream_.str().find(pattern) != std::string::npos;
  }
};

// test that the health handler returns a 200 OK response with the body "OK"
TEST_F(HealthHandlerTest, Returns200AndBodyOK) {
  HttpRequest req;
  req.method = "GET";
  req.path = "/health";
  req.headers["Content-Type"] = "text/plain";
  req.client_ip = "127.0.0.1";

  auto resp = h_.handle_request(req);
  EXPECT_EQ(resp->status_code, 200);
  EXPECT_EQ(resp->headers["Content-Type"], "text/plain");
  EXPECT_EQ(resp->body, "OK");
}

// test that the health handler returns a 404 Not Found response with the body "Not Found"
TEST_F(HealthHandlerTest, Returns404AndBodyNotFound) {
  HttpRequest req;
  req.method = "POST";
  req.path = "/health";
  req.headers["Content-Type"] = "text/plain";
  req.client_ip = "127.0.0.1";

  auto resp = h_.handle_request(req);
  EXPECT_EQ(resp->status_code, 404);
  EXPECT_EQ(resp->body, "Not Found");
}

// test malformed request - empty method
TEST_F(HealthHandlerTest, HandlesEmptyMethod) {
  HttpRequest req;
  req.method = "";
  req.path = "/health";
  req.client_ip = "127.0.0.1";

  auto resp = h_.handle_request(req);
  EXPECT_EQ(resp->status_code, 400);
  EXPECT_EQ(resp->body, "Bad Request");
}

// test malformed request - invalid path
TEST_F(HealthHandlerTest, HandlesInvalidPath) {
  HttpRequest req;
  req.method = "GET";
  req.path = "invalid_path";
  req.client_ip = "127.0.0.1";

  auto resp = h_.handle_request(req);
  EXPECT_EQ(resp->status_code, 404);
  EXPECT_EQ(resp->body, "Not Found");
}

// test malformed request - missing headers
TEST_F(HealthHandlerTest, HandlesMissingHeaders) {
  HttpRequest req;
  req.method = "GET";
  req.path = "/health";
  req.client_ip = "127.0.0.1";

  auto resp = h_.handle_request(req);
  EXPECT_EQ(resp->status_code, 200); // Health handler should still work without headers
  EXPECT_EQ(resp->body, "OK");
}
