#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

#include <fstream>
#include <string>
#include <sstream>
#include "echo_handler.h"
#include "handler_registry.h"
#include <thread>
#include <boost/log/trivial.hpp>

const std::string EchoHandler::kName = "EchoHandler";

EchoHandler::EchoHandler(const std::string& path) : path_(path) {}

std::unique_ptr<HttpResponse> EchoHandler::handle_request(const HttpRequest& req) {
  BOOST_LOG_TRIVIAL(info) << "Handling /echo in thread " << std::this_thread::get_id();
  auto res = std::make_unique<HttpResponse>();
  if (req.method == "GET") {
    return doEcho(req);
  } else {
    res->status_code = 400;
  }
  res->headers["Content-Length"] = std::to_string(res->body.size());
  return res;
}

 std::unique_ptr<HttpResponse> EchoHandler::doEcho(const HttpRequest& req) {
   auto res = std::make_unique<HttpResponse>();
   res->status_code = 200;
   res->body        = req.raw;
   res->headers["Content-Type"] = "text/plain";
   res->headers["Content-Length"] = std::to_string(res->body.size());
   return res;
 }


static const bool echoRegistered =
  HandlerRegistry::instance()
    .registerHandler(
      EchoHandler::kName,    
      [](auto const& args) {        
        return std::make_unique<EchoHandler>(
          args.at(0)
        );
      }
    );