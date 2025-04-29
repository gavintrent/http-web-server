#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

// namespace http = boost::beast::http;

#include "echo_handler.h"

// http::response<http::string_body> EchoHandler::HandleRequest(boost::system::error_code& parser_error, 
//                                    const char *data, 
//                                    size_t bytes_transferred 
// ) {
//   //parse received data as HTTP request
//   http::request_parser<http::string_body> p;
//   p.put(boost::asio::buffer(data, bytes_transferred), parser_error);
//   p.put_eof(parser_error);
//   http::request<http::string_body> req = p.get();

//   //setup server response
//   http::response<http::string_body> res;
//   res.version(11);
//   res.set(http::field::content_type, "text/plain");

//   //accept only HTTP 1.1 GET requests and setup response accordingly
//   if (req.method() == http::verb::get && req.version() == 11) {
//     res.result(http::status::ok);
//     res.body() = std::string(data, bytes_transferred); //append client req to body of response
//   }
//   else {
//     res.result(http::status::bad_request);
//   }

//   //return HTTP response
//   res.prepare_payload();
//   return res;
// }
