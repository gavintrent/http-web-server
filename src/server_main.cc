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

    server s(io_service, port);
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
