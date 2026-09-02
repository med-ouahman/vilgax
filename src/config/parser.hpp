#pragma once

#include <variant>
#include <expected>
#include "lexer.hpp"

namespace config {

struct parse_error {

};

struct eof {

};

struct directive {
    string value;
    token_type type;
};

class parser {
private:
    const std::vector<token>& tokens_;
    usize pos_;
    std::expected<token, parse_error> expect(token_type type);
    token next();
    
    std::expected<server_config, parse_error> parse_server();
    std::expected<location_config, parse_error> parse_location();
    std::expected<fastcgi_config, parse_error> parse_fastcgi();
    std::expected<directive, parse_error> parse_directive();    
public:
    parser(const std::vector<token>& tokens);
    ~parser();
    std::expected<void, parse_error> parse();
};

}