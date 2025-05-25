#ifndef SERVER_H
#define SERVER_H

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>

using boost::asio::ip::tcp;

#include "session.h"

class server
{
public:
    server(boost::asio::io_service& io_service, short port);

    void start_accept();
    void handle_accept(session* new_session, const boost::system::error_code& error);
    void join_pool();

private:
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
  boost::asio::thread_pool thread_pool_{4};
  friend class ServerTest;
};
#endif
