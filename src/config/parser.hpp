#pragma once

#include "config.hpp"
#include "expected.hpp"
#include "lexer.hpp"

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
        case parse_error_code::none: return "";
        case parse_error_code::expected_token: return "expected token";
        case parse_error_code::unknown_token: return "unknown token";
        case parse_error_code::unexpected_token: return "unexpected token";
        case parse_error_code::not_allowed: return "configuration rule not allowed here";
        case parse_error_code::missing_value: return "missing value";
    }
    return "";
}

inline string unexpected_token_error_phrase(token_type expected, token_type found) {
    return "unexpected token: expected '" + string(get_token_name(expected)) +
           "' got '" + string(get_token_name(found)) + "'";
}

inline string get_line_column(usize line, usize column) {
    return "at line: " + std::to_string(line) + ", column: " +
           std::to_string(column);
}

inline string token_not_allowed(token_type token, token_type context) {
    return "token '" + string(get_token_name(token)) + "' not allowed within '" +
           string(get_token_name(context)) + "'";
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
    parse_error_code code;
    usize line;
    usize column;

    union {
        unexpected_token_error unexpected_err;
        missing_value_error missing_err;
        token_not_allowed_error not_allowed_err;
    };

    parse_error(parse_error_code error_code, usize error_line, usize error_column)
        : code(error_code), line(error_line), column(error_column) {}

    parse_error(parse_error_code error_code, usize error_line, usize error_column,
                token_type expected, token_type found)
        : code(error_code), line(error_line), column(error_column) {
        unexpected_err = {expected, found};
    }

    parse_error(parse_error_code error_code, usize error_line, usize error_column,
                token_type directive)
        : code(error_code), line(error_line), column(error_column) {
        if (error_code == parse_error_code::missing_value)
            missing_err = {directive};
        else
            not_allowed_err = {directive, token_type::none};
    }

    parse_error(parse_error_code error_code, token_type token,
                usize error_line, usize error_column)
        : parse_error(error_code, error_line, error_column, token) {}

    parse_error(parse_error_code error_code, token_type token, token_type context,
                usize error_line, usize error_column)
        : code(error_code), line(error_line), column(error_column) {
        not_allowed_err = {token, context};
    }
};

struct directive {
    string value;
    token_type type;
};

class parser {
private:
    static constexpr usize max_location_depth_ = 2;

    const std::vector<token>& tokens_;
    usize pos_;
    main_config conf_;
    usize line_;
    usize column_;

    bool eof() const;
    token next();
    base::expected<token, parse_error> expect(token_type type);
    base::expected<server_config, parse_error> parse_server();
    base::expected<location_config, parse_error> parse_location(usize depth);
    base::expected<fastcgi_config, parse_error> parse_fastcgi();
    base::expected<directive, parse_error> parse_directive(token_type directive);

public:
    parser(const std::vector<token>& tokens);
    ~parser();

    base::expected<void, parse_error> parse();
    const main_config& get_main_conf() const;
};

}
