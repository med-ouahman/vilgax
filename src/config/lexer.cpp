#include "lexer.hpp"
#include <cassert>
#include <map>

namespace config {

#define TOKEN(TYPE, VALUE) token(TYPE, VALUE, line_, column_)
#define WORD(W) token(token_type_of_word(W), W, line_, column_)
#define SYMBOL(S) token(token_type_of_symbol(S), string(&S, 1), line_, column_)
#define LEXER_ERROR(CODE) std::unexpected(lexer_error(CODE, line_, column_))
 

static std::map<std::string, token_type> known_words = {
    { "workers", token_type::workers},
    { "workers_auto", token_type::workers_auto},
    { "user", token_type::user},
    { "group", token_type::group},
    { "pid_file", token_type::pid_file},
    { "access_log", token_type::access_log},
    { "error_log", token_type::error_log},
    { "max_connections", token_type::max_connections},
    { "max_connections_per_worker", token_type::max_connections_per_worker},
    { "server", token_type::server},
    { "server_name", token_type::server_name},

    { "root", token_type::root},
    { "listen", token_type::listen},
    { "location", token_type::location},
    { "index", token_type::index},
    { "max_request_line_size", token_type::max_request_line_size},
    { "max_headers_size", token_type::max_header_size},
    { "max_headers", token_type::max_headers},
    { "client_max_body_size", token_type::client_body_max_size},
    { "max_request_per_connection", token_type::max_requests_per_connection},

    { "keepalive", token_type::keepalive},
    { "sendfile", token_type::sendfile},
    { "sendfile_min_size", token_type::sendfile_min_size},
    { "timeout_headers", token_type::timeout_headers},
    { "timeout_body", token_type::timeout_body},
    { "timeout_request_line", token_type::timeout_request_line},
    { "timeout_write", token_type::timeout_write},
    { "backlog", token_type::backlog},
    { "autoindex", token_type::autoindex},
    { "error_pages", token_type::error_pages},
    { "fastcgi", token_type::fastcgi},
    { "connection_timeout", token_type::connection_timeout},
    { "read_timeout", token_type::read_timeout},
    { "redirect", token_type::redirect}
};


static bool is_space(char c) {
    return c == ' ' ||
           c == '\t' ||
           c == '\n' ||
           c == '\r';
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z');
}

static bool is_special(char c) {
    return c == '-' ||
        c == '_' ||
        c == '.' ||
        c == '/' ||
        c == '\\' ||
        c == '$' ||
        c == ':' ||
        c == '~';
}

static char_type classify(char c) {
    if (c == '#') return char_type::comment;
    if (c == '\n') return char_type::newline;
    if (c == '\0') return char_type::end;
    if (is_space(c)) return char_type::whitespace;
    if (is_digit(c)) return char_type::digit;
    if (is_alpha(c)) return char_type::alpha;
    
    if (is_special(c)) return char_type::special;

    if (string("\"'`").find(c) != string::npos) return char_type::quote;
    if (string("{};").find(c) != string::npos) return char_type::symbol;
    return char_type::invalid;
}

static const char* get_token_name(token_type type) {
    switch (type) {
        case token_type::lbrace:                       return "lbrace";
        case token_type::rbrace:                       return "rbrace";
        case token_type::semicolon:                   return "semicolon";
        case token_type::workers:                     return "workers";
        case token_type::workers_auto:                return "workers_auto";
        case token_type::user:                        return "user";
        case token_type::group:                       return "group";
        case token_type::access_log:                  return "access_log";
        case token_type::error_log:                   return "error_log";
        case token_type::pid_file:                     return "pid_file";
        case token_type::max_connections:             return "max_connections";
        case token_type::max_connections_per_worker:  return "max_connections_per_worker";
        case token_type::server:                       return "server";
        case token_type::server_name:                  return "server_name";
        case token_type::listen:                       return "listen";
        case token_type::root:                         return "root";
        case token_type::max_request_line_size:        return "max_request_line_size";
        case token_type::max_headers:                  return "max_headers";
        case token_type::max_header_size:              return "max_header_size";
        case token_type::client_body_max_size:         return "client_body_max_size";
        case token_type::max_requests_per_connection:  return "max_requests_per_connection";
        case token_type::keepalive:                    return "keepalive";
        case token_type::sendfile:                     return "sendfile";
        case token_type::sendfile_min_size:            return "sendfile_min_size";
        case token_type::timeout_headers:              return "timeout_headers";
        case token_type::timeout_body:                 return "timeout_body";
        case token_type::timeout_request_line:         return "timeout_request_line";
        case token_type::timeout_write:                return "timeout_write";
        case token_type::backlog:                      return "backlog";
        case token_type::error_pages:                  return "error_pages";
        case token_type::connection_timeout:           return "connection_timeout";
        case token_type::read_timeout:                 return "read_timeout";
        case token_type::redirect:                     return "redirect";
        case token_type::location:                     return "location";
        case token_type::fastcgi:                      return "fastcgi";
        case token_type::autoindex:                    return "autoindex";
        case token_type::index:                        return "index";
        case token_type::identifier:                   return "identifier";
        case token_type::string:                       return "string";
        case token_type::number:                       return "number";
        case token_type::end:                          return "end";
    }
    return "unknown";
}

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

static token_type token_type_of_symbol(char c) {
    switch (c) {
        case lbrace: return token_type::lbrace;
        case rbrace: return token_type::rbrace;
        case semicolon: return token_type::semicolon;
    }
    return token_type::end;
}

static token_type token_type_of_word(const std::string& word) {
    auto it = known_words.find(word);
    if (it == known_words.end()) return token_type::identifier;
    return it->second;
}

void print_token(const token& token) {
    std::cout << "type: " << get_token_name(token.type_) << " | value: " << token.value_ << std::endl; 
}

lexer::lexer(std::string_view& s)
    : lexed_(false),
    source_(s),
    pos_(0),
    line_(0),
    column_(0) {}

lexer::~lexer() {}

bool lexer::eof() const {
    return pos_ >= source_.size();
}

void lexer::skip_line() {
    char c = '\0';
    while (!eof() && ((c = consume()) != '\n')) {}

    if (c == '\n') unconsume();
}

char lexer::consume() {
    if (pos_ >= source_.size())
        return '\0';
    return source_[pos_++];
}

void lexer::unconsume() {
    if (pos_ == 0) return;
    --pos_;
}

std::expected<token, lexer_error> lexer::next() {
    string word;

    while (!eof()) {
        auto c = consume();
        auto type = classify(c);
        switch (type) {
            case char_type::newline:
                ++line_;
                column_ = 0;
                break;
            case char_type::comment:
                skip_line();
                if (!word.empty()) return WORD(word);
                break;
            case char_type::symbol:
                if (!word.empty()) {
                    unconsume();
                    return WORD(word);
                }
                return SYMBOL(c);
            case char_type::digit: case char_type::alpha: case char_type::special:
                word += c;
                break;
            case char_type::whitespace:
                if (!word.empty()) return WORD(word);
                break;
            case char_type::invalid:
                std::cout << "C: '" << c << "'\n";
                return LEXER_ERROR(lexer_error_code::invalid_character);
            default: break;
        }
        ++column_;
    }
    return TOKEN(token_type::end, "");
}

std::expected<void, lexer_error> lexer::lex() {
    while (true) {
        auto token = next();
        if (!token)
            return std::unexpected(token.error());
        if (token.value().type_ == token_type::end) break;
        tokens_.push_back(token.value());
    }
    lexed_ = true;
    return {};
}

const std::vector<token>& lexer::tokens() const {
    assert(lexed_ && "tokens() called before lexing");
    return tokens_;
}

}
