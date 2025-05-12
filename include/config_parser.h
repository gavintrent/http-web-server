// An nginx config file parser.

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "request_handler.h"
#include "echo_handler.h"
#include "static_handler.h"
#include "not_found_handler.h"

bool parseConfig(const char* config_file, int& port,
    std::vector<std::tuple<std::string, std::string, std::shared_ptr<RequestHandler>>>& routes);

class NginxConfig;

// The parsed representation of a single config statement.
class NginxConfigStatement {
 public:
  std::string ToString(int depth);
  std::vector<std::string> tokens_;
  std::unique_ptr<NginxConfig> child_block_;
};

// The parsed representation of the entire config.
class NginxConfig {
 public:
  std::string ToString(int depth = 0);
  std::vector<std::shared_ptr<NginxConfigStatement>> statements_;
};

// The driver that parses a config file and generates an NginxConfig.
class NginxConfigParser {
 public:
  NginxConfigParser() = default;

  // Take a opened config file or file name (respectively) and store the
  // parsed config in the provided NginxConfig out-param.  Returns true
  // iff the input config file is valid.
  bool Parse(std::istream* config_file, NginxConfig* config);
  bool Parse(const char* file_name, NginxConfig* config);

 public:
  // Token kinds produced by the lexer (made public for helper usage)
  enum TokenType {
    TOKEN_TYPE_START,
    TOKEN_TYPE_NORMAL,
    TOKEN_TYPE_START_BLOCK,
    TOKEN_TYPE_END_BLOCK,
    TOKEN_TYPE_COMMENT,
    TOKEN_TYPE_STATEMENT_END,
    TOKEN_TYPE_EOF,
    TOKEN_TYPE_ERROR,
    TOKEN_TYPE_QUOTED_STRING,
  };

 private:
  const char* TokenTypeAsString(TokenType type);

  enum TokenParserState {
    TOKEN_STATE_INITIAL_WHITESPACE,
    TOKEN_STATE_SINGLE_QUOTE,
    TOKEN_STATE_DOUBLE_QUOTE,
    TOKEN_STATE_TOKEN_TYPE_COMMENT,
    TOKEN_STATE_TOKEN_TYPE_NORMAL
  };

  TokenType ParseToken(std::istream* input, std::string* value);
};
