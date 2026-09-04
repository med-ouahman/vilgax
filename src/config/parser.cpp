#include "parser.hpp"
#include "config.hpp"


namespace config {

#define UNEXPECTED_TOKEN_ERROR(EXPECTED, FOUND) \
    base::unexpected(parse_error(parse_error_code::unexpected_token, \
        line_, \
        column_, \
        EXPECTED, \
        FOUND))
        
#define PARSE_ERROR(CODE) base::unexpected(parse_error(CODE, line_, column_))

#define VALUE_ERROR() PARSE_ERROR(parse_error_code::missing_value)

#define NOT_ALLOWED(TOKEN_TYPE) \
    base::unexpected(parse_error(parse_error_code::not_allowed, TOKEN_TYPE, line_, column_))

parser::parser(const std::vector<token>& tokens): tokens_(tokens), pos_(0), conf_(), line_(0), column_(0) {}

parser::~parser() {}

token parser::next() {
    if (pos_ >= tokens_.size()) return token(token_type::end, "");
    return tokens_[pos_++];
}

bool parser::eof() const {
    return pos_ >= tokens_.size();
}

base::expected<token, parse_error> parser::expect(token_type type) {
    auto next_token = next();

    if (next_token.type_ != type)
        return UNEXPECTED_TOKEN_ERROR(type, next_token.type_);
    return next_token;
}

base::expected<server_config, parse_error> parser::parse_server() {

    server_config serv_conf{};

    auto token = expect(token_type::lbrace);

    if (!token) return base::unexpected(token.error());

    while (!eof()) {
       auto token = next();
       switch (token.type_) {
            case token_type::location:
                parse_location();
                break;
            case token_type::root:
            case token_type::server_name:
            case token_type::index:
            case token_type::error_pages:
            case token_type::client_body_max_size:
            case token_type::redirect:
            case token_type::listen:
                parse_directive();
                break;
            case token_type::fastcgi:
                parse_fastcgi();
                break;
            default: return NOT_ALLOWED(token.type_);
       }
    }

    {

        
        auto token = \
            expect(token_type::rbrace);
 parse_error_code::not_allowed, TOKEN_TYPE, line_, column_       if (!token) return base::unexpected(token.error());
    }
    return serv_conf;
}

base::expected<location_config, parse_error> parser::parse_location() {
    location_config loc_conf{};
    
    auto token = expect(token_type::lbrace);
    
    if (!token) return base::unexpected(token.error());

    while (!eof()) {
    
    }

    {
        auto token = expect(token_type::rbrace);
        if (!token) return base::unexpected(token.error());
    }
    
    
    return loc_conf;
}

base::expected<fastcgi_config, parse_error> parser::parse_fastcgi() {
    fastcgi_config fastcgi_conf{};

    auto token = expect(token_type::lbrace);
    if (!token) return base::unexpected(token.error());
    
    while (!eof()) {
        std::cout << "hehe\n";
    }

  { auto token = expect(token_type::rbrace);
    
    if (!token) return base::unexpected(token.error());}
    
    return fastcgi_conf;
}

base::expected<directive, parse_error> parser::parse_directive() {
    
    directive dirc{};

    auto token = next();
    
    if (!token) return VALUE_ERROR();
        
    auto expected_ = expect(token_type::semicolon);
    
    if (!expected_) return base::unexpected(expected_.error());

    dirc.value = expected_.value().value_;
    dirc.type = expected_.value().type_;
    
    return dirc;
}

base::expected<void, parse_error> parser::parse() {
    line_ = 0;
    column_ = 0;
    pos_ = 0;
    while (!eof()) {
        auto token = next();
        auto type = token.type_;
        line_ = token.line_;
        column_ = token.column_;
        switch (type) {
            case token_type::server: {
                auto result = parse_server();
                if (!result) return base::unexpected(result.error());
                conf_.servers.push_back(result.value());
                break;
            }
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
                return PARSE_ERROR(parse_error_code::not_allowed_top_level);
        }

    }

    return {};
}

}