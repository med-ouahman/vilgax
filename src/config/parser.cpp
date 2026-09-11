#include "parser.hpp"
#include "config.hpp"


namespace config {

#define PARSE_ERROR() std::unexpected(parse_error())
parser::parser(const std::vector<token>& tokens): tokens_(tokens) {}

parser::~parser() {}

bool parser::eof() const {
    return tokens_[pos_].type_ == token_type::end;
}


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
    server_config serv_conf{};
    while (true) {
        auto token = next();
        switch (token.type_) {
            case token_type::server_name:
            case token_type::autoindex:
            case token_type::root:
            case token_type::index:
            case token_type::listen:
            case token_type::error_pages:
            case token_type::redirect:
                parse_directive();
                break;
            case token_type::location:
                parse_location();
                break;
            case token_type::fastcgi:
                parse_fastcgi();
                break;
            case token_type::end:
                break;
            default:
                return PARSE_ERROR();
        }
    }
    if (!expect(token_type::rbrace)) return PARSE_ERROR();
    return serv_conf;
}

std::expected<location_config, parse_error> parser::parse_location() {
    if (!expect(token_type::lbrace)) return PARSE_ERROR();
    location_config loc_conf{};
    while (true) {
        auto token = next();
        switch (token.type_) {
            case token_type::root:
            case token_type::index:
            case token_type::autoindex:
            case token_type::client_body_max_size:
            case token_type::redirect:
                parse_directive();
                break;
            case token_type::end:
                break;
            default:
                return PARSE_ERROR();
        }
    }
    if (!expect(token_type::rbrace)) return PARSE_ERROR();
    return loc_conf;

}

std::expected<fastcgi_config, parse_error> parser::parse_fastcgi() {
    if (!expect(token_type::lbrace)) return PARSE_ERROR();
    fastcgi_config fastcgi_conf{};
    while (!eof()) {
        auto token = next();
        switch (token.type_) {
            case token_type::listen:
                parse_directive();
                break;
            default:
                return PARSE_ERROR();
        }
    }
    if (!expect(token_type::lbrace)) return PARSE_ERROR();
    return fastcgi_conf;
}

std::expected<directive, parse_error> parser::parse_directive() {
    directive dirc{};
    auto token = expect(token_type::identifier);
    if (!token) return PARSE_ERROR();
    if (!expect(token_type::semicolon)) return PARSE_ERROR();
    dirc.value = token.value().value_;
    dirc.type = token.value().type_;
    return dirc;
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