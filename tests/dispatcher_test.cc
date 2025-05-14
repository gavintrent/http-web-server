#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <functional>
#include "dispatcher.h"
#include "request_handler.h"

class MockCallback {
public:
    MOCK_METHOD(std::unique_ptr<RequestHandler>, call, (), ()); 
};

TEST(DispatcherTest, SingleMatch) { 
    MockCallback mock;
    EXPECT_CALL(mock, call()).Times(1);

    Dispatcher::registerRoute("/", [&mock]() {return mock.call();});
    Dispatcher::match("/");
}

TEST(DispatcherTest, LongestMatch) { 
    MockCallback mock;
    EXPECT_CALL(mock, call()).Times(1);

    Dispatcher::registerRoute("/", []() {return nullptr;});
    Dispatcher::registerRoute("/foo", []() {return nullptr;});
    Dispatcher::registerRoute("/foo/bar", [&mock]() {return mock.call();});
    Dispatcher::registerRoute("/foo/bar/baz", []() {return nullptr;});
    Dispatcher::match("/foo/bar");
}
