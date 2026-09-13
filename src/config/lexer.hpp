#pragma once

#include <string_view>
#include <string>
#include <vector>
#include "expected.hpp"
#include "types.hpp"
#include <iostream>
#include <unordered_map>

namespace config {

enum class char_type {
    whitespace,
    comment,
    newline,
    digit,
    alpha,
    special,
    quote,
    symbol,
    end,
    invalid
};

enum class token_type {
    none,
    lbrace,
    rbrace,
    semicolon,
    workers,
    workers_auto,
    user,
    group,
    access_log,
    error_log,
    pid_file,
    max_connections,
    max_connections_per_worker,
    server,
    server_name,
    listen,
    root,
    max_request_line_size,
    max_headers,
    max_header_size,
    client_body_max_size,
    max_requests_per_connection,
    keepalive,
    sendfile,
    sendfile_min_size,
    timeout_headers,
    timeout_body,
    timeout_request_line,
    timeout_write,
    backlog,
    error_pages,
    connection_timeout,
    read_timeout,
    redirect,
    location,
    fastcgi,
    autoindex,
    index,
    identifier,
    string,
    number,
    end
};

enum known_symbols: char {
    semicolon = ';',
    lbrace = '{',
    rbrace = '}',
    none = '\0',
};

enum class lexer_error_code {
    none,
    invalid_character,
    malformed_token,
    unterminated_string,
    invalid_escape
};

struct lexer_error {
    lexer_error_code    code_;
    usize               line_;
    usize               column_;
    
    lexer_error(): code_(lexer_error_code::none), line_(0), column_(0) {}
    lexer_error(lexer_error_code code, usize line, usize col): code_(code), line_(line), column_(col) {}  
};

struct token {
    token_type  type_;
    string      value_;
    usize       line_;
    usize       column_;
    token(token_type t, string v, usize l=0, usize c=0)
        : type_(t), value_(v), line_(l), column_(c) {}
    explicit operator bool() { return type_ == token_type::end; }
};

const char* get_token_name(token_type type);
void print_token(const token& token);
const char* lexer_error_code_phrase(lexer_error_code code);

class lexer {
private:
    bool lexed_;
    string_view& source_;
    std::vector<token> tokens_;
    usize pos_;
    usize line_;
    usize column_;

private:
    base::expected<token, lexer_error> next();

    bool    eof() const;
    void    skip_line();
    char    consume();
    void    unconsume();

public:
    lexer(std::string_view& s);
    ~lexer();
    base::expected<void, lexer_error> lex();
    const std::vector<token>& tokens() const;
};

}