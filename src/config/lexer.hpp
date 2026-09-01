#pragma once

#include <string_view>
#include <string>
#include <vector>
#include <expected>
#include "types.hpp"

namespace config {

enum class char_type {
    whitespace,
    digit,
    alpha,
    quote,
    symbol,
    end,
    invalid
};

bool is_space(char c) {
    return c == ' ' ||
           c == '\t' ||
           c == '\n' ||
           c == '\r';
}

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z');
}

char_type type_of(char c) {

    if (c == '\0') return char_type::end;
    if (is_space(c)) return char_type::whitespace;
    if (is_digit(c)) return char_type::digit;
    if (is_alpha(c)) return char_type::alpha;
    if (string("\"'`").find(c) != string::npos) return char_type::quote;
    if (string("{};#").find(c) != string::npos) return char_type::symbol;

    return char_type::invalid;
}

enum class token_type {
    lbrace,
    rbrace,
    semicolon,
    server,
    listen,
    root,
    location,
    server_name,
    workers,
    fastcgi,
    autoindex,
    index,
    identifier,
    string,
    number,
    end
};

enum class lexer_error_code {
    none,
    invalid_character,
    malformed_token,
    unterminated_string,
    invalid_escape
};

const char* lexer_error_code_phrase(lexer_error_code code) {
    switch (code) {
        case lexer_error_code::none:
            return "";
        case lexer_error_code::invalid_character:
            return "Invalid character";
        case lexer_error_code::malformed_token:
            return "Malformed token";
        case lexer_error_code::unterminated_string:
            return "Unterminated string";
        case lexer_error_code::invalid_escape:
            return "Invalid escape";
    }
    return "";
}

token_type_of(char c) {
    
}

struct lexer_error {
    lexer_error_code    code_;
    usize               line_;
    usize               column_;
    lexer_error(): code_(lexer_error_code::none), line_(0), column_(0) {}
};

struct token {
    token_type  type;
    string      value;

    token(token_type t, string v): type(t), value(v) {}
};

class lexer {
private:
    string_view& source_;
    std::vector<token> tokens_;
    usize pos_;
    usize line_;
    usize column_;
private:
    std::expected<token, lexer_error> next();
    bool        eof() const;
    char_type   skip(char_type t);
    char        consume();
public:
    lexer(std::string_view& s);
    ~lexer();
    std::expected<void, lexer_error> lex();
};

}