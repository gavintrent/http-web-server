#include "gtest/gtest.h"
#include "config_parser.h"

// test fixture for config parser tests
class NginxConfigParserTest : public ::testing::Test {
  protected:
    NginxConfigParser parser;
  
    // helper function that uses an istringstream to simulate file input
    bool ParseString(const std::string &config_str, NginxConfig *config) {
      std::istringstream config_stream(config_str);
      return parser.Parse(&config_stream, config);
    }
  };

// test empty configuration
TEST_F(NginxConfigParserTest, EmptyConfig) {
  std::string config_str = "";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  // check that no statements were parsed
  EXPECT_EQ(config.statements_.size(), 0);
}
  
// test valid configuration
TEST_F(NginxConfigParserTest, ValidConfig) {
  std::string config_str = "hello;";
  NginxConfig config;
  EXPECT_TRUE(ParseString(config_str, &config));
  // check that one statement was parsed
  EXPECT_EQ(config.statements_.size(), 1);
}

// test invalid configuration with no semicolon
TEST_F(NginxConfigParserTest, InvalidConfigMissingSemicolon) {
  std::string config_str = "hello";
  NginxConfig config;
  // check that parsing fails
  EXPECT_FALSE(ParseString(config_str, &config));
}

// test multiple statements
TEST_F(NginxConfigParserTest, MultipleStatements) {
  std::string config_str = "hello;\nworld;";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  // check that two statements were parsed
  EXPECT_EQ(config.statements_.size(), 2);
}

// test a simple configuration with a block
TEST_F(NginxConfigParserTest, SimpleBlock) {
  std::string config_str = "server {\nlisten 80;\n}\n";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  // check that one statement was parsed
  EXPECT_EQ(config.statements_.size(), 1);
  // check that the block contains one statement
  EXPECT_EQ(config.statements_[0]->child_block_->statements_.size(), 1);
}

// test nested blocks
TEST_F(NginxConfigParserTest, NestedBlocks) {
  std::string config_str = "server {\nlocation / {\ntry_files $uri $uri/ =404;\n}\n}\n";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  // check that one statement was parsed
  EXPECT_EQ(config.statements_.size(), 1);
  // check that the block contains one statement
  EXPECT_EQ(config.statements_[0]->child_block_->statements_.size(), 1);
  // check that the nested block contains one statement
  EXPECT_EQ(config.statements_[0]->child_block_->statements_[0]->child_block_->statements_.size(), 1);
}

// test single quoted string
TEST_F(NginxConfigParserTest, SingleQuotedString) {
  std::string config_str = "foo 'bar';";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  EXPECT_EQ(config.statements_.size(), 1);
  EXPECT_EQ(config.statements_[0]->tokens_.size(), 2);
  EXPECT_EQ(config.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config.statements_[0]->tokens_[1], "'bar'");
}

// test double quoted string
TEST_F(NginxConfigParserTest, DoubleQuotedString) {
  std::string config_str = "foo \"bar\";";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  EXPECT_EQ(config.statements_.size(), 1);
  EXPECT_EQ(config.statements_[0]->tokens_.size(), 2);
  EXPECT_EQ(config.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config.statements_[0]->tokens_[1], "\"bar\"");
}

// test comments
TEST_F(NginxConfigParserTest, Comments) {
  std::string config_str = "foo; # this is a comment\nbar; # another comment\n";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  // check that two statements were parsed
  EXPECT_EQ(config.statements_.size(), 2);
  EXPECT_EQ(config.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config.statements_[1]->tokens_[0], "bar");
}

// test escape sequences in strings
TEST_F(NginxConfigParserTest, EscapeSequences) {
  std::string config_str = "foo \"bar\\n\";";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  EXPECT_EQ(config.statements_.size(), 1);
  EXPECT_EQ(config.statements_[0]->tokens_.size(), 2);
  EXPECT_EQ(config.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config.statements_[0]->tokens_[1], "\"bar\\n\"");
}

// test invalid quoted string
TEST_F(NginxConfigParserTest, InvalidQuotedString) {
  std::string config_str = "foo \"bar;";
  NginxConfig config;
  
  // check that parsing fails
  EXPECT_FALSE(ParseString(config_str, &config));
}

// test invalid block structure
TEST_F(NginxConfigParserTest, UnmatchedOpenBrace) {
  std::string config_str = "server {";
  NginxConfig config;
  
  EXPECT_FALSE(ParseString(config_str, &config));
}

TEST_F(NginxConfigParserTest, UnmatchedCloseBrace) {
  std::string config_str = "server {}}\n";
  NginxConfig config;
  
  EXPECT_FALSE(ParseString(config_str, &config));
}

// test empty block
TEST_F(NginxConfigParserTest, EmptyBlock) {
  std::string config_str = "server { }";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  // check that one statement was parsed
  EXPECT_EQ(config.statements_.size(), 1);
  // check that the block contains no statements
  EXPECT_EQ(config.statements_[0]->child_block_->statements_.size(), 0);
}

// add more tests if necessary

/*
TEST(NginxConfigParserTest, SimpleConfig) {
  NginxConfigParser parser;
  NginxConfig out_config;

  bool success = parser.Parse("example_config", &out_config);

  EXPECT_TRUE(success);
}*/
