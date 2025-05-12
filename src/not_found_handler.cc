#include "not_found_handler.h"

HttpResponse NotFoundHandler::handleRequest(const HttpRequest& req){
    HttpResponse res;
    res.status_code = 404;
    res.headers["Content-Type"]   = "text/plain";
    res.headers["Content-Length"] = "0";
    return res;
}
