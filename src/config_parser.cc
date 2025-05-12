// An nginx config file parser.
//
// See:
//   http://wiki.nginx.org/Configuration
//   http://blog.martinfjordvald.com/2010/07/nginx-primer/
//
// How Nginx does it:
//   http://lxr.nginx.org/source/src/core/ngx_conf_file.c

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>
#include <unordered_set>
#include <boost/log/trivial.hpp>
#include <map>

#include "config_parser.h"

// ========================================
// Namespace
// ========================================
namespace {
  // returns true if the character is a whitespace character
  inline bool IsWhiteSpace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
  }

  // returns true if the character is a block delimeter
  inline bool IsBlockDelimeter(char c) {
    return c == ';' || c == '{' || c == '}';
  }

  // returns true if the next character in the stream terminates a quoted token
  bool NextCharTerminatesQuotedToken(std::istream* input) {
    if (!input->good()) 
      return true;
    
    char c = input->peek();
    return IsWhiteSpace(c) || IsBlockDelimeter(c); 
  }

  // consume character until the matching quote is found
  NginxConfigParser::TokenType ConsumeQuotedToken(std::istream* input, std::string* value, const char quote_char) {
    while (input->good()) {
      char c = input->get();
      if (!input->good()) 
        break;
      
      *value += c;

      if (c == '\\') {
        const char escaped_char = input->get();
        if (!input->good()) {
          return NginxConfigParser::TOKEN_TYPE_ERROR;
        }
        *value += escaped_char;
        continue;
      } 
      if (c == quote_char) {
        return NextCharTerminatesQuotedToken(input) ? 
          NginxConfigParser::TOKEN_TYPE_QUOTED_STRING : 
          NginxConfigParser::TOKEN_TYPE_ERROR;
      }
    }
    return NginxConfigParser::TOKEN_TYPE_ERROR;
  }
  
} // end of namespace

// ========================================
// NginxConfig implementation
// ========================================
std::string NginxConfig::ToString(int depth) {
  std::string output;
  // iterate through all statements in the config
  for (const auto& statement : statements_) {
    output.append(statement->ToString(depth));
  }
  return output;
}

// ========================================
// NginxConfigStatement implementation
// ========================================
std::string NginxConfigStatement::ToString(int depth) {
  std::string output(depth * 2, ' '); // 2 spaces per indentation

  // iterate through all tokens in the statement
  for (size_t i = 0; i < tokens_.size(); ++i) {
    if (i != 0) {
      output.append(" ");
    }
    output.append(tokens_[i]);
  }
  // if the statement has a child block, add it to the output
  if (child_block_.get() != nullptr) {
    output.append(" {\n");
    output.append(child_block_->ToString(depth + 1));
    output.append(std::string(depth * 2, ' '));
    output.append("}");
  } else { // if the statement does not have a child block, add a semicolon to the output
    
    output.append(";");
  }
  output.append("\n");
  return output;
}

// ========================================
// NginxConfigParser helper functions
// ========================================
const char* NginxConfigParser::TokenTypeAsString(TokenType type) {
  switch (type) {
    case TOKEN_TYPE_START:         return "START";
    case TOKEN_TYPE_NORMAL:        return "NORMAL";
    case TOKEN_TYPE_START_BLOCK:   return "START_BLOCK";
    case TOKEN_TYPE_END_BLOCK:     return "END_BLOCK";
    case TOKEN_TYPE_COMMENT:       return "COMMENT";
    case TOKEN_TYPE_STATEMENT_END: return "STATEMENT_END";
    case TOKEN_TYPE_EOF:           return "EOF";
    case TOKEN_TYPE_ERROR:         return "ERROR";
    case TOKEN_TYPE_QUOTED_STRING: return "QUOTED_STRING";  // added
    default:                       return "Unknown token type";
  }
}

// ========================================
// NginxConfigParser ParseToken
// ========================================
NginxConfigParser::TokenType NginxConfigParser::ParseToken(std::istream* input,
                                                           std::string* value) {
  // clear any previous token data
  value->clear();
  TokenParserState state = TOKEN_STATE_INITIAL_WHITESPACE;
  while (input->good()) {
    const char c = input->get();
    if (!input->good()) 
      break;
    
    switch (state) {
      case TOKEN_STATE_INITIAL_WHITESPACE: // if token is a whitespace, skip, else return the proper token type
        if (IsWhiteSpace(c)) {
          continue;  // Skip.
        } else if (c == '{') {
          *value = c;
          return TOKEN_TYPE_START_BLOCK;
        } else if (c == '}') {
          *value = c;
          return TOKEN_TYPE_END_BLOCK;
        } else if (c == ';') {
          *value = c;
          return TOKEN_TYPE_STATEMENT_END;
        } else if (c == '#') {
          state = TOKEN_STATE_TOKEN_TYPE_COMMENT;
        } else if (c == '\"' || c == '\'') {
          *value = c;
          return ConsumeQuotedToken(input, value, c);
        } else {  // Start a regular token.
          *value += c;
          state = TOKEN_STATE_TOKEN_TYPE_NORMAL;
        }
        break;

      case TOKEN_STATE_TOKEN_TYPE_NORMAL: // if token is a normal, return the normal
        if (IsWhiteSpace(c) || IsBlockDelimeter(c)) {
          input->unget();
          return TOKEN_TYPE_NORMAL;
        } 
        *value += c;
        break;
      
      case TOKEN_STATE_TOKEN_TYPE_COMMENT: // if token is a comment, return the comment
        if (c == '\n' || c == '\r') {
          return TOKEN_TYPE_COMMENT;
        }
        break;

      default:
        break;
    }
  }

  if (!value->empty()) {
    return TOKEN_TYPE_NORMAL;
  }
  return TOKEN_TYPE_EOF;
}

// ========================================
// NginxConfigParser Parse
// ========================================
bool NginxConfigParser::Parse(std::istream* config_file, NginxConfig* config) {
  std::stack<NginxConfig*> config_stack;
  config_stack.push(config);

  TokenType last = TOKEN_TYPE_START;
  TokenType token_type;
  while (true) {
    std::string token;
    token_type = ParseToken(config_file, &token);
    //printf ("%s: %s\n", TokenTypeAsString(token_type), token.c_str());
    if (token_type == TOKEN_TYPE_ERROR) {
      return false;
    }

    if (token_type == TOKEN_TYPE_COMMENT) {
      continue; // Skip comments.
    }

    switch (token_type) {// if token is a normal or quoted string, add it to current statement
      case TOKEN_TYPE_NORMAL:
      case TOKEN_TYPE_QUOTED_STRING: { 
        if (last == TOKEN_TYPE_START || last == TOKEN_TYPE_STATEMENT_END || last == TOKEN_TYPE_START_BLOCK || last == TOKEN_TYPE_END_BLOCK) {
          config_stack.top()->statements_.emplace_back(new NginxConfigStatement); 
          }
          config_stack.top()->statements_.back().get()->tokens_.push_back(token);
        break;
      }
      case TOKEN_TYPE_STATEMENT_END: // if token is a statement end, check if previous token was normal or quoted string
        if (last != TOKEN_TYPE_NORMAL && last != TOKEN_TYPE_QUOTED_STRING) {
          return false;
        }
        break;

      case TOKEN_TYPE_START_BLOCK: // if token is a start block, check if previous token was a normal
        if (last != TOKEN_TYPE_NORMAL) {
          return false;
        }
        // add a child block to the current statement
        config_stack.top()->statements_.back().get()->child_block_.reset(new NginxConfig);
        config_stack.push(config_stack.top()->statements_.back().get()->child_block_.get());
        break;

      case TOKEN_TYPE_END_BLOCK: // if token is a end block, check if config stack size is 1, and if last token was a statement end, end block, or start block
        if (config_stack.size() == 1) {
          return false;
        }
        if (last != TOKEN_TYPE_STATEMENT_END && last != TOKEN_TYPE_END_BLOCK && last != TOKEN_TYPE_START_BLOCK) {
          return false;
        }
        // pop the current statement from the stack
        config_stack.pop();
        break;

      case TOKEN_TYPE_EOF: // if token is a end of file, check if config stack size is 1, and if last token was a statement end, end block, or start block
        if (config_stack.size() != 1) {
          return false;
        }
        return last == TOKEN_TYPE_STATEMENT_END || last == TOKEN_TYPE_END_BLOCK ||
               (last == TOKEN_TYPE_START && config_stack.top()->statements_.empty());

      default:
        return false;
    }
    last = token_type;
  }

  printf ("Bad transition from %s to %s\n",
          TokenTypeAsString(last),
          TokenTypeAsString(token_type));
  return false;
}

bool NginxConfigParser::Parse(const char* file_name, NginxConfig* config) {
  std::ifstream config_file;
  config_file.open(file_name);
  if (!config_file.good()) {
    printf ("Failed to open config file: %s\n", file_name);
    return false;
  }

  const bool return_value =
      Parse(dynamic_cast<std::istream*>(&config_file), config);
  config_file.close();
  return return_value;
}

using HandlerPtr = std::shared_ptr<RequestHandler>;
bool parseConfig(const char* config_file, int& port, std::vector<std::tuple<std::string, std::string, HandlerPtr>>& routes) {
    NginxConfigParser parser;
    NginxConfig config;

    if (!parser.Parse(config_file, &config)) {
        BOOST_LOG_TRIVIAL(error) << "Error parsing config file";
        return false;
    }

    std::unordered_set<std::string> seen_paths; // Track to prevent duplicate paths

    for (const auto& stmt : config.statements_) {
        if (stmt->tokens_.size() == 2 && stmt->tokens_[0] == "port") {
            port = std::stoi(stmt->tokens_[1]);
            BOOST_LOG_TRIVIAL(info) << "Parsed port: " << port;
        } else if (stmt->tokens_.size() >= 3 && stmt->tokens_[0] == "location") {
            std::string path = stmt->tokens_[1];
            std::string handler_type = stmt->tokens_[2];

            // Check for trailing slash
            if (path.back() == '/') {
                BOOST_LOG_TRIVIAL(error) << "Trailing slashes not allowed: " << path;
                return false;
            }

            // Check for duplicates
            if (!seen_paths.insert(path).second) {
                BOOST_LOG_TRIVIAL(fatal) << "Duplicate location: " << path; // Server shutdown on duplicate
                return false;
            }

            // Parse handler arguments from the child block
            std::map<std::string, std::string> handler_args;
            if (stmt->child_block_) {
                for (const auto& arg_stmt : stmt->child_block_->statements_) {
                    if (arg_stmt->tokens_.size() >= 2) {
                        handler_args[arg_stmt->tokens_[0]] = arg_stmt->tokens_[1];
                    }
                }
            }

            // Get handler instance
            std::shared_ptr<RequestHandler> handler;
            if (handler_type == "EchoHandler") {
                handler = std::make_shared<EchoHandler>();
            } else if (handler_type == "StaticHandler") {
                handler = std::make_shared<StaticHandler>();
            }

            // Get root dir from child { ... } block
            std::string root_dir = "";
            for (const auto& arg_stmt : stmt->child_block_->statements_) {
              if (arg_stmt->tokens_.size() >= 2 && arg_stmt->tokens_[0] == "root") {
                root_dir = arg_stmt->tokens_[1];
              }
            }

            routes.emplace_back(path, root_dir, handler);
        }
        else{
          routes.emplace_back("/", "/", std::make_shared<NotFoundHandler>());
        }
    }

    return true;
}