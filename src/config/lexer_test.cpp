#include <gtest/gtest.h>
#include <string_view>
#include "config/lexer.hpp"

using namespace config;

class LexerTest : public ::testing::Test {
protected:
    std::expected<std::vector<token>, lexer_error> lex_string(const std::string& input) {
        std::string_view sv(input);
        lexer lex(sv);
        auto result = lex.lex();
        if (!result) {
            return std::unexpected(result.error());
        }
        return lex.tokens();
    }
};

// Test basic keywords
TEST_F(LexerTest, BasicKeywords) {
    auto result = lex_string("workers; user; group;");
    ASSERT_TRUE(result);
    EXPECT_EQ(result->size(), 6);
    EXPECT_EQ(result->at(0).type, token_type::workers);
    EXPECT_EQ(result->at(1).type, token_type::semicolon);
    EXPECT_EQ(result->at(2).type, token_type::user);
}

// Test symbols
TEST_F(LexerTest, Symbols) {
    auto result = lex_string("{ } ;");
    ASSERT_TRUE(result);
    EXPECT_EQ(result->size(), 3);
    EXPECT_EQ(result->at(0).type, token_type::lbrace);
    EXPECT_EQ(result->at(1).type, token_type::rbrace);
    EXPECT_EQ(result->at(2).type, token_type::semicolon);
}

// Test numbers and identifiers
TEST_F(LexerTest, Numbers) {
    auto result = lex_string("max_connections 1024;");
    ASSERT_TRUE(result);
    EXPECT_GE(result->size(), 2);
    EXPECT_EQ(result->at(0).type, token_type::max_connections);
    EXPECT_EQ(result->at(1).type, token_type::identifier);
    EXPECT_EQ(result->at(1).value, "1024");
}

// Test identifiers with underscores and dashes
TEST_F(LexerTest, Identifiers) {
    auto result = lex_string("my_custom_var; localhost-server;");
    ASSERT_TRUE(result);
    EXPECT_GE(result->size(), 4);
    EXPECT_EQ(result->at(0).type, token_type::identifier);
    EXPECT_EQ(result->at(0).value, "my_custom_var");
    EXPECT_EQ(result->at(1).type, token_type::semicolon);
}

// Test comments
TEST_F(LexerTest, Comments) {
    auto result = lex_string("workers auto; # Comment here\nuser www-data;");
    ASSERT_TRUE(result);
    EXPECT_GE(result->size(), 4);
    EXPECT_EQ(result->at(0).type, token_type::workers);
}

// Test complex configuration structure
TEST_F(LexerTest, ConfigurationStructure) {
    std::string config = R"(
        workers auto;
        user www-data;
        group www-data;
        pid_file /var/run/vilgax.pid;
    )";
    auto result = lex_string(config);
    ASSERT_TRUE(result);
    EXPECT_GE(result->size(), 8);
    EXPECT_EQ(result->at(0).type, token_type::workers);
    EXPECT_EQ(result->at(2).type, token_type::semicolon);
}

// Test nested server configuration
TEST_F(LexerTest, NestedServerConfig) {
    std::string config = R"(
        server {
            server_name localhost;
            listen 8080;
            location / {
                root /var/www;
            }
        }
    )";
    auto result = lex_string(config);
    ASSERT_TRUE(result);
    EXPECT_EQ(result->at(0).type, token_type::server);
    EXPECT_EQ(result->at(1).type, token_type::lbrace);
}

// Test path and file names
TEST_F(LexerTest, Paths) {
    auto result = lex_string("access_log /var/log/vilgax.log;");
    ASSERT_TRUE(result);
    EXPECT_GE(result->size(), 2);
    EXPECT_EQ(result->at(0).type, token_type::access_log);
    EXPECT_EQ(result->at(1).type, token_type::identifier);
    EXPECT_EQ(result->at(1).value, "/var/log/vilgax.log");
}

// Test mixed case keywords and identifiers
TEST_F(LexerTest, MixedCase) {
    auto result = lex_string("MyServer; my-server; my_server;");
    ASSERT_TRUE(result);
    EXPECT_GE(result->size(), 6);
    EXPECT_EQ(result->at(0).type, token_type::identifier);
    EXPECT_EQ(result->at(1).type, token_type::semicolon);
}

// Test all config keywords
TEST_F(LexerTest, AllConfigKeywords) {
    std::string config = R"(
        workers 4;
        user www-data;
        group www-data;
        pid_file /var/run/vilgax.pid;
        access_log /var/log/vilgax/access.log;
        error_log /var/log/vilgax/error.log;
        max_connections 1024;
        max_connections_per_worker 256;
        max_request_line_size 8192;
        max_header_size 16384;
        client_body_max_size 1048576;
        timeout_request_line 30000;
        timeout_headers 30000;
        timeout_body 30000;
        timeout_write 30000;
    )";
    auto result = lex_string(config);
    ASSERT_TRUE(result);
    EXPECT_GT(result->size(), 20);
}

// Test multi-line configuration
TEST_F(LexerTest, MultilineConfiguration) {
    std::string config = R"(
workers auto;

user www-data;
group www-data;

server {
    server_name example.com;
    listen 80;
}
    )";
    auto result = lex_string(config);
    ASSERT_TRUE(result);
    EXPECT_GT(result->size(), 10);
}

// Test multiple comments and whitespace
TEST_F(LexerTest, WhitespaceAndComments) {
    std::string config = R"(
# Configuration file for Vilgax
workers auto;  # Number of workers

# User and group settings
user www-data;
    )";
    auto result = lex_string(config);
    ASSERT_TRUE(result);
    EXPECT_GE(result->size(), 4);
}

// Test various identifier formats
TEST_F(LexerTest, VariousIdentifiers) {
    auto result = lex_string("test_var; test-var; test123; _test;");
    ASSERT_TRUE(result);
    EXPECT_GE(result->size(), 8);
    EXPECT_EQ(result->at(0).type, token_type::identifier);
    EXPECT_EQ(result->at(0).value, "test_var");
    EXPECT_EQ(result->at(2).type, token_type::identifier);
    EXPECT_EQ(result->at(2).value, "test-var");
}

// Test special characters in identifiers
TEST_F(LexerTest, SpecialCharsInIdentifiers) {
    auto result = lex_string("file.conf; config~backup; path/to/file;");
    ASSERT_TRUE(result);
    EXPECT_GE(result->size(), 6);
    EXPECT_EQ(result->at(0).type, token_type::identifier);
    EXPECT_EQ(result->at(0).value, "file.conf");
    EXPECT_EQ(result->at(2).type, token_type::identifier);
    EXPECT_EQ(result->at(2).value, "config~backup");
}

// Test sequential symbols
TEST_F(LexerTest, SequentialSymbols) {
    auto result = lex_string("{}{};;");
    ASSERT_TRUE(result);
    EXPECT_EQ(result->size(), 6);
}

// Test empty configuration
TEST_F(LexerTest, EmptyConfiguration) {
    auto result = lex_string("");
    ASSERT_TRUE(result);
    EXPECT_EQ(result->size(), 0);
}

// Test tabs and multiple spaces
TEST_F(LexerTest, TabsAndSpaces) {
    auto result = lex_string("workers\t\tauto;    user\t\twww-data;");
    ASSERT_TRUE(result);
    EXPECT_EQ(result->at(0).type, token_type::workers);
    EXPECT_EQ(result->at(1).type, token_type::identifier);
    EXPECT_EQ(result->at(2).type, token_type::semicolon);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
