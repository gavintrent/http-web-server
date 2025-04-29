#ifndef SESSION_H
#define SESSION_H

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

using boost::asio::ip::tcp;

namespace http = boost::beast::http;

#include "echo_handler.h"
#include "request_parser.h" 
#include "request_handler.h"

class session
{
public:
  session(boost::asio::io_service& io_service, const std::vector<std::pair<std::string,std::shared_ptr<RequestHandler>>>& routes);
  tcp::socket& socket();
  void start();
  
private:
  void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
  void handle_write(const boost::system::error_code& error);

  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
  http::response<http::string_body> response;
  RequestParser parser_;
  RequestHandler* handler_;
  std::vector<std::pair<std::string,std::shared_ptr<RequestHandler>>> routes_;
};
#endif
