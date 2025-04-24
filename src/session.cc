#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <boost/log/trivial.hpp>

using boost::asio::ip::tcp;

namespace http = boost::beast::http;

#include "session.h"
#include "echo_handler.h"

session::session(boost::asio::io_service& io_service)
  : socket_(io_service)
{
}

tcp::socket& session::socket()
{
  return socket_;
}

void session::start()
{
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error,
    size_t bytes_transferred)
{
  if (!error)
  {
    boost::system::error_code parser_error;

    try { // log read request
      auto client_ip = socket_.remote_endpoint().address().to_string();
      auto client_port = socket_.remote_endpoint().port();
      BOOST_LOG_TRIVIAL(info) << "Received request from " << client_ip << ":" << client_port;
    } catch (std::exception& e) {
      BOOST_LOG_TRIVIAL(warning) << "Could not retrieve client address: " << e.what();
    }

    response = echoHandler.HandleRequest(parser_error, data_, bytes_transferred);

    if (parser_error) { //fail if message could not be parsed
      BOOST_LOG_TRIVIAL(error) << "Failed to parse HTTP request: " << parser_error.message();
      delete this;
      return;
    }

    BOOST_LOG_TRIVIAL(debug) << "Parsed request, sending response...";

    http::async_write(socket_,
        response,
        boost::bind(&session::handle_write, this,
          boost::asio::placeholders::error));
  }
  else
  {
    BOOST_LOG_TRIVIAL(warning) << "Read error: " << error.message();
    delete this;
  }
}

void session::handle_write(const boost::system::error_code& error)
{
  if (!error)
  {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
    BOOST_LOG_TRIVIAL(info) << "Successfully sent response to client.";
  }
  else
  {
    BOOST_LOG_TRIVIAL(warning) << "Write error: " << error.message();
    delete this;
  }
}
