#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <boost/log/trivial.hpp>
#include "dispatcher.h"

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

void session::handle_read(const boost::system::error_code& ec,
    size_t bytes_transferred)
{
  if (ec) {
    BOOST_LOG_TRIVIAL(warning) << "Read error: " << ec.message();
    delete this;
    return;
  }

  boost::system::error_code parse_ec;
  auto req = parser_.parse(data_, bytes_transferred, parse_ec);

  try {
    auto client_ip = socket_.remote_endpoint().address().to_string();
    auto client_port = socket_.remote_endpoint().port();
    BOOST_LOG_TRIVIAL(info) << "Received request from " << client_ip << ":" << client_port;
    req.client_ip = client_ip;
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(warning) << "Could not retrieve client address: " << e.what();
    req.client_ip = "unknown";
  }

  std::unique_ptr<HttpResponse> app_res;
  std::string handler_name = "None";
  if (parse_ec) {
    BOOST_LOG_TRIVIAL(error) << "Failed to parse HTTP request: " << parse_ec.message();
    app_res = std::make_unique<HttpResponse>();
    app_res->status_code = 400;
    app_res->headers["Content-Type"] = "text/plain";
    app_res->body = "Bad Request";
  } else {
    BOOST_LOG_TRIVIAL(debug) << "Parsed request, routing...";
    auto handler = Dispatcher::match(req.path);
    handler_name = handler->get_kName();
    app_res = handler->handle_request(req);
  }

  // Build Beast response
  response.result((http::status)app_res->status_code);
  response.body() = std::move(app_res->body);
  response.set(http::field::content_type, app_res->headers["Content-Type"]);
  response.prepare_payload();

  BOOST_LOG_TRIVIAL(debug) << "Sending response with status code: " << app_res->status_code;

  BOOST_LOG_TRIVIAL(info)
      << "[ResponseMetrics]"
      << " code="   << app_res->status_code
      << " path="   << req.path
      << " client=" << req.client_ip
      << " handler="<< handler_name;

  http::async_write(socket_, response,
    boost::bind(&session::handle_write, this,
                boost::asio::placeholders::error));
}

void session::handle_write(const boost::system::error_code& ec)
{
  if (!ec) {
    BOOST_LOG_TRIVIAL(info) << "Successfully sent response to client.";
    socket_.shutdown(tcp::socket::shutdown_both);
  } else {
    BOOST_LOG_TRIVIAL(warning) << "Write error: " << ec.message();
  }

  delete this;
}
