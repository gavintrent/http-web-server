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

// ========================================
// Valid‑Configuration Tests
// ========================================

// test empty configuration
TEST_F(NginxConfigParserTest, EmptyConfig) {
  std::string config_str = "";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  // check that no statements were parsed
  EXPECT_EQ(config.statements_.size(), 0);
}

// test single statement
TEST_F(NginxConfigParserTest, ValidConfig) {
  std::string config_str = "hello;";
  NginxConfig config;
  EXPECT_TRUE(ParseString(config_str, &config));
  // check that one statement was parsed
  EXPECT_EQ(config.statements_.size(), 1);
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
TEST_F(NginxConfigParserTest, SimpleNonEmptyBlock) {
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

// test deep nesting
TEST_F(NginxConfigParserTest, DeepNestingToString) {
  std::string config_str = "a { b { c 1; } }";
  NginxConfig config;
  EXPECT_TRUE(ParseString(config_str, &config));
  // round‑trip ToString
  EXPECT_EQ(config.ToString(), "a {\n  b {\n    c 1;\n  }\n}\n");
}

// test recursive empty blocks
TEST_F(NginxConfigParserTest, RecursiveEmptyBlocks) {
  std::string config_str = "server {\nlocation / {\n}\n}\n";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  // check that one statement was parsed
  EXPECT_EQ(config.statements_.size(), 1);
  // check that the block contains one statement
  EXPECT_EQ(config.statements_[0]->child_block_->statements_.size(), 1);
  // check that the nested block contains no statements
  EXPECT_EQ(config.statements_[0]->child_block_->statements_[0]->child_block_->statements_.size(), 0);
}

// test multiple blocks
TEST_F(NginxConfigParserTest, MultipleBlocks) {
  std::string config_str = "server { listen 80; }\nserver { listen 443; }\n";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  // check that two statements were parsed
  EXPECT_EQ(config.statements_.size(), 2);
  // check that each block contains one statement
  EXPECT_EQ(config.statements_[0]->child_block_->statements_.size(), 1);
  EXPECT_EQ(config.statements_[1]->child_block_->statements_.size(), 1);
}

// test mixing blocks and statements
TEST_F(NginxConfigParserTest, MixedBlocksAndStatements) {
  std::string config_str = "server { listen 80; location / { try_files $uri $uri/ =404; } }\n";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  // check that one statement was parsed
  EXPECT_EQ(config.statements_.size(), 1);
  // check that the block contains two statements
  EXPECT_EQ(config.statements_[0]->child_block_->statements_.size(), 2);
}

// ========================================
// Invalid‑Configuration Tests
// ========================================

// test invalid configuration with no semicolon
TEST_F(NginxConfigParserTest, MissingSemicolon) {
  std::string config_str = "hello";
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

// test unexpected block token
TEST_F(NginxConfigParserTest, UnexpectedBlockToken) {
  std::string config_str = "server { listen 80; } extra_token";
  NginxConfig config;
  
  EXPECT_FALSE(ParseString(config_str, &config));
}

// test no whitespace after quoted string
TEST_F(NginxConfigParserTest, NoWhitespaceAfterQuotedString) {
  std::string config_str = "foo 'bar'baz;";
  NginxConfig config;
  
  EXPECT_FALSE(ParseString(config_str, &config));
}

// test undetermined backslash at end of input
TEST_F(NginxConfigParserTest, UndeterminedBackslash) {
  std::string config_str = "foo bar\\";
  NginxConfig config;
  
  EXPECT_FALSE(ParseString(config_str, &config));
}

// test unexpected start token
TEST_F(NginxConfigParserTest, UnexpectedStartToken) {
  std::string config_str = "{ foo; }";
  NginxConfig config;
  
  EXPECT_FALSE(ParseString(config_str, &config));
}

// ========================================
// Quoted-String Tests
// ========================================

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

// test escaped single quote inside single quoted string
TEST_F(NginxConfigParserTest, EscapedSingleQuote) {
  std::string config_str = "foo 'bar\\'baz';";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  EXPECT_EQ(config.statements_.size(), 1);
  EXPECT_EQ(config.statements_[0]->tokens_.size(), 2);
  EXPECT_EQ(config.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config.statements_[0]->tokens_[1], "'bar\\'baz'");
}

// test escaped double quote inside double quoted string
TEST_F(NginxConfigParserTest, EscapedDoubleQuote) {
  std::string config_str = "foo \"bar\\\"baz\";";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  EXPECT_EQ(config.statements_.size(), 1);
  EXPECT_EQ(config.statements_[0]->tokens_.size(), 2);
  EXPECT_EQ(config.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config.statements_[0]->tokens_[1], "\"bar\\\"baz\"");
}

// test unmatched single quote
TEST_F(NginxConfigParserTest, UnmatchedSingleQuote) {
  std::string config_str = "foo 'bar;";
  NginxConfig config;
  
  EXPECT_FALSE(ParseString(config_str, &config));
}

// test unmatched double quote
TEST_F(NginxConfigParserTest, UnmatchedDoubleQuote) {
  std::string config_str = "foo \"bar;";
  NginxConfig config;
  
  EXPECT_FALSE(ParseString(config_str, &config));
}

// test cross style embedded quotes
TEST_F(NginxConfigParserTest, CrossStyleEmbeddedQuotes) {
  std::string config_str = "foo \"bar 'baz' qux\";";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  EXPECT_EQ(config.statements_.size(), 1);
  EXPECT_EQ(config.statements_[0]->tokens_.size(), 2);
  EXPECT_EQ(config.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config.statements_[0]->tokens_[1], "\"bar 'baz' qux\"");
}

// ========================================
// Comment‑Handling Tests
// ========================================

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

// test comment only
TEST_F(NginxConfigParserTest, CommentOnly) {
  std::string config_str = "# this is a comment\n";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  // check that no statements were parsed
  EXPECT_EQ(config.statements_.size(), 0);
}


// ========================================
// Serialization (ToString) Tests
// ========================================

// test empty configuration serialization
TEST_F(NginxConfigParserTest, EmptyConfigToString) {
  std::string config_str = "";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  std::string serialized = config.ToString();
  
  // check that the serialized string matches the original
  EXPECT_EQ(serialized, config_str);
}

// test to string serialization
TEST_F(NginxConfigParserTest, ToString) {
  std::string config_str = "server {\n" "  listen 80;\n" "}\n";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  std::string serialized = config.ToString();
  
  // check that the serialized string matches the original
  EXPECT_EQ(serialized, config_str);
}

// test nested blocks to string serialization
TEST_F(NginxConfigParserTest, NestedBlocksToString) {
  std::string config_str = "server {\n" "  location / {\n" "    try_files $uri $uri/ =404;\n" "  }\n" "}\n";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  std::string serialized = config.ToString();
  
  // check that the serialized string matches the original
  EXPECT_EQ(serialized, config_str);
}

// test statement only serialization
TEST_F(NginxConfigParserTest, StatementOnlyToString) {
  std::string config_str = "foo bar;\n";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  std::string serialized = config.ToString();
  
  // check that the serialized string matches the original
  EXPECT_EQ(serialized, config_str);
}

// ========================================
// Escaped‑Sequence Tests
// ========================================

// test escaped sequences
TEST_F(NginxConfigParserTest, EscapedSequences) {
  std::string config_str = "foo \"bar\\n\";";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  EXPECT_EQ(config.statements_.size(), 1);
  EXPECT_EQ(config.statements_[0]->tokens_.size(), 2);
  EXPECT_EQ(config.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config.statements_[0]->tokens_[1], "\"bar\\n\"");
}

// test escaped backslash
TEST_F(NginxConfigParserTest, BackslashEscapesInQuotes) {
  std::string config_str = "foo \"bar\\n\\t\\\\baz\";";
  NginxConfig config;
  EXPECT_TRUE(ParseString(config_str, &config));
  EXPECT_EQ(config.statements_.size(), 1);
  EXPECT_EQ(config.statements_[0]->tokens_.size(), 2);
  EXPECT_EQ(config.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config.statements_[0]->tokens_[1], "\"bar\\n\\t\\\\baz\"");
}

// ========================================
// Whitespace‑Variation Tests
// ========================================

// test whitespace variations
TEST_F(NginxConfigParserTest, WhitespaceVariations) {
  std::string config_str = "foo   \t\n bar ;";
  NginxConfig config;
  
  EXPECT_TRUE(ParseString(config_str, &config));
  EXPECT_EQ(config.statements_.size(), 1);
  EXPECT_EQ(config.statements_[0]->tokens_.size(), 2);
  EXPECT_EQ(config.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config.statements_[0]->tokens_[1], "bar");
}

// add more tests if necessary

/*
TEST(NginxConfigParserTest, SimpleConfig) {
  NginxConfigParser parser;
  NginxConfig out_config;

  bool success = parser.Parse("example_config", &out_config);

  EXPECT_TRUE(success);
}*/
