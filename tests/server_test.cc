#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <tuple>

#include "server.h"
#include "session.h"
#include "request_handler.h"

using routes_t = std::vector<std::tuple<std::string, std::string, std::shared_ptr<RequestHandler>>>;

// no-op RequestHandler so we can build a routes vector
class DummyHandler : public RequestHandler {
 public:
  HttpResponse handleRequest(const HttpRequest&) override {
    return HttpResponse();
  }
};

// derive from session so we can intercept start()
class TestSession : public session {
 public:
  bool started{false};

  TestSession(boost::asio::io_service& ios, const routes_t& routes) : session(ios, routes) {}

  // override the one in session
  void start() {
    started = true;
  }
};

// test fixture for server tests; a google test fixture
class ServerTest : public ::testing::Test {
 protected:
  boost::asio::io_service ios;
  routes_t routes;
  server srv;
  ServerTest() : srv(ios, 0, routes) {}
};

// tests constructing a server doesn’t throw exceptions
TEST_F(ServerTest, ConstructorDoesNotThrowAndStartsAccept) {
  EXPECT_NO_THROW({
    server srv(ios, 0, routes);
  });
}

// tests giving the server a valid route
TEST_F(ServerTest, HandleAcceptSuccess_CallsSessionStart) {
  // prepare a route so signature matches
  routes = { {"/", "GET", std::make_shared<DummyHandler>()} };
  server srv(ios, 0, routes);

  // use our TestSession so we can see when start() is called
  TestSession* ts = new TestSession(ios, routes);
  boost::system::error_code ec;  // default-constructed means "no error"
  srv.handle_accept(ts, ec);
  EXPECT_TRUE(ts->started);
  delete ts;
}

// tests that error paths cleans up without throwing
TEST_F(ServerTest, HandleAcceptError_DoesNotThrow) {
  // allocate a real session; handle_accept should delete it on error
  session* s = new session(ios, routes);
  auto ec = boost::asio::error::make_error_code(boost::asio::error::operation_aborted);
  EXPECT_NO_THROW({
    srv.handle_accept(s, ec);
  });
  // note: do NOT delete s again; it should have been deleted inside handle_accept
}

// standard google test entry point
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
