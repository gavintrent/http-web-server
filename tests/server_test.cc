#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <tuple>
#include <future> 

#include "server.h"
#include "session.h"
#include "request_handler.h"

// derive from session so we can intercept start()
class TestSession : public session {
 public:
  std::promise<void> started_promise;
  std::shared_future<void> started_future;

  TestSession(boost::asio::io_service& ios)
      : session(ios), started_future(started_promise.get_future()) {}

  void start() override {
    started_promise.set_value();  // signal that start was called
  }
};

class ServerTest : public ::testing::Test {
 protected:
  boost::asio::io_service ios;
  server srv;
  ServerTest() : srv(ios, 0) {}
};

TEST_F(ServerTest, ConstructorDoesNotThrowAndStartsAccept) {
  EXPECT_NO_THROW({
    server srv(ios, 0);
  });
}

TEST_F(ServerTest, HandleAcceptSuccess_CallsSessionStart) {
  TestSession* ts = new TestSession(ios);
  boost::system::error_code ec;
  srv.handle_accept(ts, ec);

  auto status = ts->started_future.wait_for(std::chrono::milliseconds(100));
  EXPECT_EQ(status, std::future_status::ready) << "Session::start() was not called in time";

  delete ts;
}

TEST_F(ServerTest, HandleAcceptError_DoesNotThrow) {
  session* s = new session(ios);
  auto ec = boost::asio::error::make_error_code(boost::asio::error::operation_aborted);
  EXPECT_NO_THROW({
    srv.handle_accept(s, ec);
  });
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
