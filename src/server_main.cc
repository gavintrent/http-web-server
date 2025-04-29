//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <map>

#include "server.h"
#include "config_parser.h"
#include "logger.h"

int main(int argc, char* argv[])
{
  try
  {
    init_logging();
    BOOST_LOG_TRIVIAL(info) << "Starting server...";
    
    if (argc != 2)
    {
      BOOST_LOG_TRIVIAL(error) << "Usage: async_tcp_echo_server <config file>";
      return 1;
    }

    boost::asio::io_service io_service;
    auto echo_handler = std::make_shared<EchoHandler>();
    
    //parse argument as config file
    NginxConfigParser parser;
    NginxConfig config;
    if (!parser.Parse(argv[1], &config) ||
        config.statements_[0]->tokens_.size() != 2u || //assume first statement of config contains port
        config.statements_[0]->tokens_[0] != "listen"
    ) {
      BOOST_LOG_TRIVIAL(error) << "Error parsing config\n";
      return 1;
    }
    
    using namespace std; // For stoi.
    int port = stoi(config.statements_[0]->tokens_[1]);
    BOOST_LOG_TRIVIAL(info) << "Parsed port: " << port;

    // based on what is in the config, build a vector to know which handler to use
    using HandlerPtr = std::shared_ptr<RequestHandler>;
    std::vector<std::tuple<std::string,std::string,HandlerPtr>> routes;
    for (auto& stmt : config.statements_) {
      if (stmt->tokens_[0] == "location" &&
          stmt->tokens_.size() >= 2)
      {
        std::string prefix = stmt->tokens_[1];
        if (prefix.back()==';') prefix.pop_back();
        // only support echo for now, add static later on
        if (prefix == "/echo") {
          routes.emplace_back(
            "/echo",
            "/",
            std::make_shared<EchoHandler>()
          );
        }
        else {
          routes.emplace_back(
            prefix,
            stmt->tokens_[2],
            std::make_shared<StaticHandler>()
          );
        }
      }
    }

    server srv(io_service, port, routes);
    BOOST_LOG_TRIVIAL(info) << "Server listening on port " << port;

    io_service.run();
  }
  catch (std::exception& e)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Unhandled exception: " << e.what();
  }

  BOOST_LOG_TRIVIAL(info) << "Server shutting down.";
  return 0;
}
