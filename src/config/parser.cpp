#include "parser.hpp"
#include "config.hpp"


namespace config {

#define PARSE_ERROR() std::unexpected(parse_error())
parser::parser(const std::vector<token>& tokens): tokens_(tokens) {}

parser::~parser() {}


token parser::next() {
    if (pos_ >= tokens_.size()) return token(token_type::end, "");
    return tokens_[pos_++];
}

std::expected<token, parse_error> parser::expect(token_type type) {
    auto next_token = next();
    if (next_token.type_ != type)
        return std::unexpected(parse_error());
    return next_token;
}

std::expected<server_config, parse_error> parser::parse_server() {

    if (!expect(token_type::lbrace)) return PARSE_ERROR();

    while (true) {

    }

    if (!expect(token_type::rbrace)) return PARSE_ERROR();

}

std::expected<location_config, parse_error> parser::parse_location() {
    
}

std::expected<fastcgi_config, parse_error> parser::parse_fastcgi() {
    
}

std::expected<directive, parse_error> parser::parse_directive() {
    
}

std::expected<void, parse_error> parser::parse() {

    while (true) {

        auto token = next();

        auto type = token.type_;
        switch (type) {
            /* parse top level blocks*/
            case token_type::server:
                parse_server();
                break;
            case token_type::workers:
            case token_type::workers_auto:
            case token_type::max_connections:
            case token_type::max_connections_per_worker:
            case token_type::user:
            case token_type::group:
            case token_type::access_log:
            case token_type::error_log:
            case token_type::pid_file:
            case token_type::max_request_line_size:
            case token_type::max_header_size:
            case token_type::max_headers:
            case token_type::max_requests_per_connection:
            case token_type::timeout_headers:
            case token_type::timeout_body:
            case token_type::timeout_write:
            case token_type::keepalive:
            case token_type::sendfile:
            case token_type::sendfile_min_size:
                parse_directive();
                break;
            default:
                /* error: configuration rule not allowed on top level*/
                return std::unexpected(parse_error());
        }

    }

    return {};
}

}