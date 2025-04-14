#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

namespace http = boost::beast::http;

class EchoHandler {
  public:
    EchoHandler() {}
    http::response<http::string_body> HandleRequest(boost::system::error_code& parser_error, 
                                   const char *data, 
                                   size_t bytes_transferred 
    );
};
#endif
