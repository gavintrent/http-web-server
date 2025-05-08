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
using HandlerPtr = std::shared_ptr<RequestHandler>;

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
    int port;
    std::vector<std::tuple<std::string, std::string, HandlerPtr>> routes;
    if (!parseConfig(argv[1], port, routes)) {
      return 1;
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
