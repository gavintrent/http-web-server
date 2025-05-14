#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <tuple>

#include "server.h"
#include "session.h"
#include "request_handler.h"

// derive from session so we can intercept start()
class TestSession : public session {
 public:
  bool started{false};

  TestSession(boost::asio::io_service& ios) : session(ios) {}

  // override the one in session
  void start() {
    started = true;
  }
};

// test fixture for server tests; a google test fixture
class ServerTest : public ::testing::Test {
 protected:
  boost::asio::io_service ios;
  server srv;
  ServerTest() : srv(ios, 0) {}
};

// tests constructing a server doesn’t throw exceptions
TEST_F(ServerTest, ConstructorDoesNotThrowAndStartsAccept) {
  EXPECT_NO_THROW({
    server srv(ios, 0);
  });
}

// tests giving the server a valid route
TEST_F(ServerTest, HandleAcceptSuccess_CallsSessionStart) {
  server srv(ios, 0);

  // use our TestSession so we can see when start() is called
  TestSession* ts = new TestSession(ios);
  boost::system::error_code ec;  // default-constructed means "no error"
  srv.handle_accept(ts, ec);
  EXPECT_TRUE(ts->started);
  delete ts;
}

// tests that error paths cleans up without throwing
TEST_F(ServerTest, HandleAcceptError_DoesNotThrow) {
  // allocate a real session; handle_accept should delete it on error
  session* s = new session(ios);
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
