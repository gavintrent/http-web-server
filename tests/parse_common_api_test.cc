#include "gtest/gtest.h"
#include "config_parser.h"
#include <fstream>

using HandlerPtr = std::shared_ptr<RequestHandler>;
class CommmonAPIConfigParserTest : public ::testing::Test {
    protected:
        std::string config_file = "test_config";

        // Helper function to write a config file
        void WriteConfigToFile(const std::string& config_text) {
            std::ofstream out(config_file);
            out << config_text;
            out.close();
        }
};

TEST_F(CommmonAPIConfigParserTest, WorkingConfigParse) {
  WriteConfigToFile(R"(
    port 8080;

    location /echo EchoHandler {
    }

    location /static StaticHandler {
      root ./files
    }
  )");

  int port;
  std::vector<std::tuple<std::string, std::string, HandlerPtr>> routes;
  bool success = parseConfig(config_file.c_str(), port, routes);

  EXPECT_TRUE(success);
  EXPECT_EQ(port, 8080);
  EXPECT_EQ(routes.size(), 2);
}

TEST_F(CommmonAPIConfigParserTest, DuplicateLocationFails) {
  WriteConfigToFile(R"(
    port 8080;

    location /echo EchoHandler {
    }

    location /echo StaticHandler {
      root ./files
    }
  )");

  int port;
  std::vector<std::tuple<std::string, std::string, HandlerPtr>> routes;
  bool success = parseConfig(config_file.c_str(), port, routes);

  EXPECT_FALSE(success);
}

TEST_F(CommmonAPIConfigParserTest, TrailingSlashFails) {
  WriteConfigToFile(R"(
    port 8080;

    location /static/ StaticHandler {
      root ./files
    }
  )");

  int port;
  std::vector<std::tuple<std::string, std::string, HandlerPtr>> routes;
  bool success = parseConfig(config_file.c_str(), port, routes);

  EXPECT_FALSE(success);
}

TEST_F(CommmonAPIConfigParserTest, UnknownHandlerFails) {
  WriteConfigToFile(R"(
    port 8080;

    location /unknown MadeUpHandler {
    }
  )");

  int port;
  std::vector<std::tuple<std::string, std::string, HandlerPtr>> routes;
  bool success = parseConfig(config_file.c_str(), port, routes);

  EXPECT_FALSE(success);
}

TEST_F(CommmonAPIConfigParserTest, MissingRootInStaticHandler) {
  WriteConfigToFile(R"(
    port 8080;

    location /static StaticHandler {
      # missing root
    }
  )");

  int port;
  std::vector<std::tuple<std::string, std::string, HandlerPtr>> routes;
  bool success = parseConfig(config_file.c_str(), port, routes);

  // If your implementation requires root in child block:
  EXPECT_FALSE(success);
}