#pragma once

#include <variant>
#include "expected.hpp"
#include "lexer.hpp"
#include "config.hpp"
#include <cassert>
namespace config {

enum class parse_error_code {
    none,
    expected_token,
    unexpected_token,
    not_allowed_top_level,
    // missing_value,
    // missing_semicolon,
    // missing_block_end,
    // invalid_value
};

inline const char* parse_error_msg(parse_error_code code) {
    switch (code) {
        case parse_error_code::none:
        case parse_error_code::expected_token:
            assert(false && "Invalid use of parse_error()");
            break;
        case parse_error_code::unexpected_token:
            return "unexpected token";
        case parse_error_code::not_allowed_top_level:
            return "configuration not allowed on top level";
        // case parse_error_code::unexpected_token:
        // case parse_error_code::unexpected_token:
        // case parse_error_code::unexpected_token:
        // case parse_error_code::unexpected_token:
    }

    return "";
}

struct parse_error {
    parse_error_code    code;
    usize               line;
    usize               column;
    token_type          expected;
    token_type          found;

    parse_error(parse_error_code c,
                            usize l,
                            usize col,
                            token_type ex,
                            token_type f)
    : code(c), line(l), column(col), expected(ex), found(f) {}
    parse_error()
        : code(parse_error_code::expected_token),
        line(0),
        column(0),
        expected(token_type::none),
        found(token_type::none) {}
};

struct directive {
    string value;
    token_type type;
};

class parser {
private:
    const std::vector<token>& tokens_;
    usize           pos_;
    main_config     conf_;
    usize           line_;
    usize           column_;
    parse_error     error_;

    base::expected<token, parse_error>  expect(token_type type);
    token                               next();
    bool                                eof() const;

    base::expected<server_config, parse_error>      parse_server();
    base::expected<location_config, parse_error>    parse_location();
    base::expected<fastcgi_config, parse_error>     parse_fastcgi();
    base::expected<directive, parse_error>          parse_directive();    
public:
    parser(const std::vector<token>& tokens);
    ~parser();

    base::expected<void, parse_error> parse();
};

}