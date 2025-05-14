# Code Layout
* server_main.cc

  Contains main method. Initilizes the server by passing the given config file to the `config_parser` and creating a new `server` object.

* config_parser.cc

  Used for parsing config files. Passes parsed routes to the `dispatcher`.

* dispatcher.cc

  Manages dispatching HTTP requests to specific request handlers based on the longest matching URL prefix.

* handler_registry.cc

  Manages dynamically registering and creating handlers. Each handler registers itself by calling the `registerHandler` method. Registered handlers are created using the `createHandler` method.

* logger.cc

  Manages server logging.

* echo_handler.cc, static_handler.cc, not_found_handler.cc

  Request handlers. Each handler inherits from the `request_handler.h` header file. They are created per request by the `dispatcher`.

* request_parser.cc

  Takes raw byte data and attempts to convert it into an `HTTPRequest` object. Returns an error code in an out parameter on failure.

* server.cc

  When created, starts up the server and begins a `session`.

* session.cc

  Manages reads and writes to the server. `handleRead` method passes received data into `request_parser` to convert it into an `HTTPRequest` object. If this succeeds, the request's URL is matched to a handler via the `dispatcher` and the request is passed to the matching handler.

## Custom HTTP Message Types

Our server uses 2 custom types to define HTTP requests and responses. These are defined in `http_types.h`.

```cpp
struct HttpRequest {
  std::string method; // HTTP method
  std::string path; // URL path
  std::map<std::string, std::string> headers; // map header names to their values
  std::string body; //HTTP body
  std::string raw; //raw string containing the full HTTP request
};
```
```cpp
struct HttpResponse {
  int status_code; //HTTP status
  std::map<std::string, std::string> headers; // map header names to their values
  std::string body; //HTTP body
};
``` 

# Building, Testing, and Running

The server can be built by performing an [out of source build](https://www.cs130.org/guides/cmake/#out-of-source-builds).
```shell
$ mkdir build
$ cd build
$ cmake ..
$ make
```

Still in the /build directory, you can run our server locally using our gcloud config file with the following command:

```shell
$ bin/server ../config/gcloud.config
```

You can [run the tests](https://www.cs130.org/assignments/1/#run-the-existing-tests) for our server by using either the `ctest` or `make test` command in /build. 

# Adding a Request Handler

*Note: In all of the following examples, you can replace* `ExampleHandler` *with the name of your new handler.*

## Defining a New Request Handler Class

All request handlers must inherit from the `RequestHandler` class.

Request handlers should contain the following public methods and fields:
  
### Constructor

The constructor's first parameter must be the serving path of the handler defined in the config file, followed by any number of additional arguments. The additional arguments map to the same arguments defined within the `{...}` block of the config file **in the same order**.

```cpp
ExampleHandler::ExampleHandler(const std::string& path, .../*other params*/) {
  ...
}
```

### handle_request

The handle request method takes a `const HttpRequest&` as input and outputs a `std::unique_ptr<HttpResponse>`. This method is where the main handler functionality should be defined.

```cpp
std::unique_ptr<HttpResponse> ExampleHandler::handle_request(const HttpRequest& req) {
  ...
}
```

### kName Field

As part of our code style, the class should have a const string called kName. The value of this field determines the name of the handler used in the config file.

```cpp
const std::string ExampleHandler::kName = "ExampleHandler";
```

### Static Initializer

In order to register our handlers dynamically, the following static field should be included

```cpp
static const bool exampleRegistered =
  HandlerRegistry::instance()
    .registerHandler(
      ExampleHandler::kName,    
      [](auto const& args) {        
        return std::make_unique<ExampleHandler>(
          args.at(0) /*, args.at(1), args.at(2), etc*/
        );
      }
    );
```

The name of the field can be anything.

The number of args.at(#) should match the number of parameters in your constructor.

## Example Handler Templates

Here are some example templates of a full header and source file for a new handler.

### Header

```cpp
#pragma once

class ExampleHandler : public RequestHandler {
public:
  ExampleHandler(const std::string& path, .../*other params*/);

  std::unique_ptr<HttpResponse> handle_request(const HttpRequest& req) override;

  static const std::string kName;

};

```
### Source

```cpp
#include "example_handler.h"
#include "handler_registry.h"

const std::string ExampleHandler::kName = "ExampleHandler";

ExampleHandler::ExampleHandler(const std::string& path, .../*other params*/) {
  ...
}

std::unique_ptr<HttpResponse> ExampleHandler::handle_request(const HttpRequest& req) {
  auto res = std::make_unique<HttpResponse>();
  ...
  /*code to setup response*/
  ...
  return res;
}

static const bool exampleRegistered =
  HandlerRegistry::instance()
    .registerHandler(
      ExampleHandler::kName,    
      [](auto const& args) {        
        return std::make_unique<ExampleHandler>(
          args.at(0) /*, args.at(1), args.at(2), etc*/
        );
      }
    );
```

## Existing Handler Example

not_found_handler


### Header
```cpp
#pragma once

#include "request_handler.h"
#include "http_types.h"

class NotFoundHandler : public RequestHandler {
public:
  explicit NotFoundHandler(const std::string& path);
  std::unique_ptr<HttpResponse> handle_request(const HttpRequest& req);
  static const std::string kName;
protected:
std::string path_;
};
```

### Source
```cpp
#include "not_found_handler.h"
#include "handler_registry.h"

//kName, used as handler name in config file
const std::string NotFoundHandler::kName = "NotFoundHandler";

//Constructor, first arg is always serving path
NotFoundHandler::NotFoundHandler(const std::string& path) : path_(path) {}

//handle_request method, sets up and returns a unique ptr to a 404 response
std::unique_ptr<HttpResponse> NotFoundHandler::handle_request(const HttpRequest& req){
    auto res = std::make_unique<HttpResponse>();
    res->status_code = 404; //example of setting a response field
    return res;
}

//static initializer, registers handler with handler registry
static const bool notFoundRegistered =
  HandlerRegistry::instance()
    .registerHandler(
      NotFoundHandler::kName,          
      [](auto const& args) {      
        return std::make_unique<NotFoundHandler>(
          args.at(0) //this maps directly to "path" in the constructor
        );
      }
    );
```


## CMakeLists.txt

In order to properly link a new handler, you must add a **object library** for it in `CMakeLists.txt`. It is important that the `OBJECT` keyword is included, as otherwise the handler may not get registered properly.

```
add_library(example_handler_lib OBJECT src/example_handler.cc)
```

In addition, it should be linked to the handler registry as follows:

```
target_link_libraries(example_handler_lib
  PUBLIC handler_registry
)
```

## Config

The config file should follow the API discussed in class, with the name of the new handler being the same as the value of its `kName` field.

It should be noted that the only argument allowed inside the location blocks of our config files is `root [PATH];`. If additional argument types are needed, then the `config_parser` should be modified to support them in the same way as root.

