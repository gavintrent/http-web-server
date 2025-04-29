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

session::session(boost::asio::io_service& io_service, const std::vector<std::pair<std::string,std::shared_ptr<RequestHandler>>>& routes)
  : socket_(io_service),
  routes_(routes)
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
  if (ec) return;

  boost::system::error_code parse_ec;
  auto req = parser_.parse(data_, bytes_transferred, parse_ec);

  HttpResponse app_res;
  if (parse_ec) {
    app_res.status_code = 400;
    app_res.body        = "Bad Request";
  } else {
        bool handled = false;
    for (auto& [prefix, handler] : routes_) {
      if (req.path.rfind(prefix, 0) == 0) {
        app_res = handler->handleRequest(req);
        handled = true;
        break;
      }
    }
    if (!handled) {
      app_res.status_code = 404;
      app_res.body        = "Not Found";
    }
  }

  // Build Beast response
  response.result((http::status)app_res.status_code);
  response.body() = std::move(app_res.body);
  response.prepare_payload();

  http::async_write(socket_, response,
    boost::bind(&session::handle_write, this,
                boost::asio::placeholders::error));
}

void session::handle_write(const boost::system::error_code& ec)
{
  if (!ec) {
    // For simplicity, close after one request/response
    socket_.shutdown(tcp::socket::shutdown_both);
  }
}
