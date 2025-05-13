#include "not_found_handler.h"
#include "handler_registry.h"

std::unique_ptr<HttpResponse> NotFoundHandler::handle_request(const HttpRequest& req){
    auto res = std::make_unique<HttpResponse>();
    res->status_code = 404;
    res->headers["Content-Type"]   = "text/plain";
    res->headers["Content-Length"] = "0";
    return res;
}


 REGISTER_HANDLER(NotFoundHandler)