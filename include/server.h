#ifndef SERVER_H
#define SERVER_H

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

#include "session.h"

class server
{
public:
    server(boost::asio::io_service& io_service,
         short port,
         const std::vector<std::pair<std::string,std::shared_ptr<RequestHandler>>>& routes);

private:
  void start_accept();
  void handle_accept(session* new_session, const boost::system::error_code& error);
  
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
  std::vector<std::pair<std::string,std::shared_ptr<RequestHandler>>> routes_;
};
#endif
