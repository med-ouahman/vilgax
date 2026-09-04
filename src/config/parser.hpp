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
    unknown_token,
    unexpected_token,
    not_allowed,
    missing_value
};

inline const char* parse_error_msg(parse_error_code code) {
    switch (code) {
        case parse_error_code::none:
        case parse_error_code::expected_token:
            assert(false && "Invalid use of parse_error()");
            break;
        case parse_error_code::unexpected_token:
            return "unexpected token";
        case parse_error_code::not_allowed:
            return "configuration not allowed on top level";
        case parse_error_code::missing_value:
            return "missing value";
    }

    return "";
}

inline string unexpected_token_error(token_type expected, token_type found) {

    string a(get_token_name(expected));
    string b(get_token_name(found));
    
    return "unexpected token: expected '" + a + "' got '"+b+"'";
}

inline string get_line_column(usize line, usize col) {
    return "at line: " + std::to_string(line) + ", column: " + std::to_string(col);
}

inline string token_not_allowed(token_type t, token_type context) {
    string a(get_token_name(t));
    string b(get_token_name(context))

    return "token '" + a + "' not allowed within '" + b + "'";
}

struct unexpected_token_error {
    token_type expected;
    token_type found;
};

struct missing_value_error {
    token_type directive;
};

struct token_not_allowed_error {
    token_type not_allowed;
    token_type context;
};

struct parse_error {
    parse_error_code    code;
    usize               line;
    usize               column;

    union type {
        unexpected_token_error  unexpected_err;
        missing_value_error     missing_err;
        token_not_allowed_error not_allowed_err;
    };

    parse_error(parse_error_code c,
                            usize l,
                            usize col,
                            token_type ex,
                            token_type f)
    : code(c), line(l), column(col), expected(ex), found(f) {}

    parse_error() {}

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
