// sleep_handler.cc
#include "sleep_handler.h"
#include "handler_registry.h"
#include <thread>
#include <chrono>
#include <thread>
#include <boost/log/trivial.hpp>

const std::string SleepHandler::kName = "SleepHandler";

std::unique_ptr<HttpResponse> SleepHandler::handle_request(const HttpRequest& req) {
    BOOST_LOG_TRIVIAL(info) << "Handling /sleep in thread " << std::this_thread::get_id();
  std::this_thread::sleep_for(std::chrono::seconds(3));  // Delay
  auto res = std::make_unique<HttpResponse>();
  res->status_code = 200;
  res->body = "Slept";
  res->headers["Content-Type"] = "text/plain";
  res->headers["Content-Length"] = std::to_string(res->body.size());
  return res;
}

static const bool sleepRegistered =
  HandlerRegistry::instance()
    .registerHandler(
      SleepHandler::kName,
      [](auto const& args) {
        return std::make_unique<SleepHandler>();
      });
