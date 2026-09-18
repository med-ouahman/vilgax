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
    missing_value,
    listen_error
};

inline const char* parse_error_msg(parse_error_code code) {
    switch (code) {
        case parse_error_code::none: return "";
        case parse_error_code::expected_token: return "expected token";
        case parse_error_code::unknown_token: return "unknown token";
        case parse_error_code::unexpected_token: return "unexpected token";
        case parse_error_code::not_allowed: return "configuration rule not allowed here";
        case parse_error_code::missing_value: return "missing value";
        case parse_error_code::listen_error: return "listen error";
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

enum class listen_error_code {
    empty_directive,       // The config string was completely empty
    invalid_format,        // General syntax error (e.g., unmatched '[' for IPv6)
    
    missing_port,          // Expected a port but found none (e.g., "127.0.0.1:")
    invalid_port,          // Port is not a number or contains invalid chars (e.g., "127.0.0.1:abc")
    port_out_of_range,     // Port is < 1 or > 65535
    
    missing_ip,            // Expected an IP address but found none
    invalid_ipv4,          // Malformed IPv4 (e.g., "256.1.2.3" or "192.168.1")
    invalid_ipv6,          // Malformed IPv6 (e.g., ":::1" or invalid hex)
    
    invalid_backlog,
    unsupported_protocol   // If your config parses prefixes like "tcp://" or "udp://"
};

struct listen_error {
    listen_error_code   code;
    std::string         context;

    listen_error(listen_error_code c) : code(c) {}
    listen_error(listen_error_code c, std::string ctx)
        : code(c), context(std::move(ctx)) {}

    std::string message() const {
        std::string base_msg;
        switch (code) {
            case listen_error_code::empty_directive:      base_msg = "Listen directive is empty"; break;
            case listen_error_code::invalid_format:       base_msg = "Invalid listen address format"; break;
            case listen_error_code::missing_port:         base_msg = "Port number is missing"; break;
            case listen_error_code::invalid_port:         base_msg = "Port number contains invalid characters"; break;
            case listen_error_code::port_out_of_range:    base_msg = "Port number must be between 1 and 65535"; break;
            case listen_error_code::missing_ip:           base_msg = "IP address is missing"; break;
            case listen_error_code::invalid_ipv4:         base_msg = "Invalid IPv4 address format"; break;
            case listen_error_code::invalid_ipv6:         base_msg = "Invalid IPv6 address format"; break;
            case listen_error_code::unsupported_protocol: base_msg = "Unsupported protocol specified"; break;
            case listen_error_code::invalid_backlog:      base_msg = "Invalid backlog"; break;
            default:                                      base_msg = "Unknown parsing error"; break;
        }

        if (!context.empty()) {
            base_msg += ": '" + context + "'";
        }
        
        return base_msg;
    }
};

struct parse_error {
    parse_error_code code;
    usize line;
    usize column;

    union {
        unexpected_token_error  unexpected_err;
        missing_value_error     missing_err;
        token_not_allowed_error not_allowed_err;
        listen_error            listen_err;
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

    parse_error(const listen_error& err)
        : code(parse_error_code::listen_error),
        line(0),
        column(0),
        listen_err(std::move(err)) {}

    parse_error()
        :  code(parse_error_code::missing_value),
        line(0),
        column(0),
            missing_err() {}

    ~parse_error() {}
};

void print_parse_error(const parse_error& error, const string& filename);

struct directive_conf {
    std::vector<string> values;
    token_type type;
};

class parser {
public:
    static constexpr usize max_port_number_ = (256 << 8) - 1;
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
    base::expected<directive_conf, parse_error> parse_directive(token_type directive);

    base::expected<void, parse_error> add_server_directive(server_config& server, const directive_conf& directive);

public:
    parser(const std::vector<token>& tokens);
    ~parser();

    base::expected<void, parse_error> parse();
    const main_config& get_main_conf() const;
};

}
