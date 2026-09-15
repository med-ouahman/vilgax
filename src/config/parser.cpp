#include "parser.hpp"
#include "baselib/stoi.hpp"

namespace config {

static bool accpets_multiple_values(token_type token) {

    switch (token) {
        case token_type::error_pages:
        case token_type::server_name:
        case token_type::index:
        case token_type::autoindex:
            return true;
        default: return false;
    }

    return false;
}

static base::expected<listen_endpoint, parse_error> parse_listen(const string& listen_str) {
    auto colon = listen_str.find_last_of(':');
    if (colon == string::npos)
        return std::unexpected(parse_error());

}


base::expected<void, parse_error>
parser::add_server_directive(server_config& server, const directive_conf& directive) {
    switch (directive.type) {
        case token_type::server_name:
            server.server_names = std::move(directive.values);
            break;
        case token_type::root:
            server.root = std::move(directive.values[0]);
            break;
        case token_type::error_pages:
            server.erorr_pages = std::move(directive.values);
            break;
        case token_type::index:
            server.index = std::move(directive.values);
            break;
        case token_type::autoindex: {
            const auto& value = directive.values[0];
            server.autoindex = value == "on";
            break;
        }
        case token_type::redirect: {
            if (directive.values.size() > 2)
                return base::unexpected(parse_error());
            server.redirect.location = directive.values[0];
            auto result = base::stoi(directive.values[1]);
            if (!result) {
                return std::unexpected(parse_error());
            }
            server.redirect.code = result.value();
            break;
        }
        case token_type::listen: {
            for (const auto value: directive.values) {
                auto listen = parse_listen(value);
                if (!listen) {
                    return std::unexpected(parse_error());
                }
                server.listens.push_back(listen.value());
            }
        }

    }
}

void print_parse_error(const parse_error& error, const string& filename) {
    std::cout << "Error: Parser: " << parse_error_msg(error.code) << "\n";

    switch (error.code) {
        case parse_error_code::unexpected_token:
            std::cout << "  Expected: '"
                      << get_token_name(error.unexpected_err.expected)
                      << "'\n"
                      << "  Found: '"
                      << get_token_name(error.unexpected_err.found)
                      << "'\n";
            break;
        case parse_error_code::missing_value:
            std::cout << "  Directive: '"
                      << get_token_name(error.missing_err.directive)
                      << "'\n";
            break;
        case parse_error_code::not_allowed:
            std::cout << "  Token: '"
                      << get_token_name(error.not_allowed_err.not_allowed)
                      << "'\n"
                      << "  Context: '"
                      << get_token_name(error.not_allowed_err.context)
                      << "'\n";
            break;
        case parse_error_code::none:
        case parse_error_code::expected_token:
        case parse_error_code::unknown_token:
            break;
    }

    std::cout << "  Location: " << filename << ":" << error.line << ":"
              << error.column << "\n";
}

#define UNEXPECTED_TOKEN_ERROR(EXPECTED, FOUND) \
    base::unexpected(parse_error(parse_error_code::unexpected_token, \
                                 line_, column_, EXPECTED, FOUND))

#define VALUE_ERROR(DIRECTIVE) \
    base::unexpected(parse_error(parse_error_code::missing_value, \
                                 line_, column_, DIRECTIVE))

#define NOT_ALLOWED(TOKEN_TYPE, CONTEXT) \
    base::unexpected(parse_error(parse_error_code::not_allowed, \
                                 TOKEN_TYPE, CONTEXT, line_, column_))

parser::parser(const std::vector<token>& tokens)
    : tokens_(tokens), pos_(0), conf_(), line_(0), column_(0) {}

parser::~parser() = default;

bool parser::eof() const {
    return pos_ >= tokens_.size();
}

token parser::next() {
    if (eof())
        return token(token_type::end, "", line_, column_);
    return tokens_[pos_++];
}

base::expected<token, parse_error>
parser::expect(token_type type) {
    const auto found = next();
    line_ = found.line_;
    column_ = found.column_;
    if (found.type_ != type)
        return UNEXPECTED_TOKEN_ERROR(type, found.type_);
    return found;
}

base::expected<directive_conf, parse_error>
parser::parse_directive(token_type directive_type) {
    auto value = next();
    if (value.type_ == token_type::end)
        return VALUE_ERROR(directive_type);
    directive_conf dirc{};
    size_t count = 0;
    bool accepts_mult = accpets_multiple_values(directive_type);
    while (value && value.type_ == token_type::identifier) {
        ++count;
        if (!accepts_mult && count > 1)
            return UNEXPECTED_TOKEN_ERROR(token_type::semicolon, value.type_);
        dirc.values.push_back(value.value_);
        value = next();
    }
    if (value.type_ != token_type::semicolon) {
        return UNEXPECTED_TOKEN_ERROR(token_type::semicolon, value.type_);
    }
    if (count == 0) {
        return UNEXPECTED_TOKEN_ERROR(token_type::identifier, token_type::semicolon);
    }
    return dirc;
}

base::expected<location_config, parse_error>
parser::parse_location(usize depth) {
    const auto path = expect(token_type::identifier);
    if (!path)
        return base::unexpected(path.error());
    const auto brace = expect(token_type::lbrace);
    if (!brace)
        return base::unexpected(brace.error());
    location_config location{};
    location.path = path.value().value_;
    while (!eof()) {
        const auto current = next();
        line_ = current.line_;
        column_ = current.column_;
        if (current.type_ == token_type::rbrace)
            return location;

        switch (current.type_) {
            case token_type::root:
            case token_type::index:
            case token_type::autoindex:
            case token_type::client_body_max_size:
            case token_type::redirect: {
                auto result = parse_directive(current.type_);
                if (!result)
                    return base::unexpected(result.error());
                break;
            }
            case token_type::location: {
                if (depth >= max_location_depth_)
                    return NOT_ALLOWED(current.type_, token_type::location);
                auto result = parse_location(depth + 1);
                if (!result)
                    return base::unexpected(result.error());
                location.locations.push_back(result.value());
                break;
            }
            default: return NOT_ALLOWED(current.type_, token_type::location);
        }
    }
    return UNEXPECTED_TOKEN_ERROR(token_type::rbrace, token_type::end);
}

base::expected<fastcgi_config, parse_error>
parser::parse_fastcgi() {

    const auto brace = expect(token_type::lbrace);
    if (!brace)
        return base::unexpected(brace.error());

    fastcgi_config fastcgi{};
    while (!eof()) {
        const auto current = next();
        line_ = current.line_;
        column_ = current.column_;
        if (current.type_ == token_type::rbrace)
            return fastcgi;
        if (current.type_ != token_type::listen)
            return NOT_ALLOWED(current.type_, token_type::fastcgi);

        auto result = parse_directive(current.type_);
        if (!result)
            return base::unexpected(result.error());
    }
    return UNEXPECTED_TOKEN_ERROR(token_type::rbrace, token_type::end);
}

base::expected<server_config, parse_error> parser::parse_server() {
    const auto brace = expect(token_type::lbrace);
    if (!brace)
        return base::unexpected(brace.error());

    server_config server{};
    while (!eof()) {
        const auto current = next();
        line_ = current.line_;
        column_ = current.column_;
        if (current.type_ == token_type::rbrace)
            return server;
        switch (current.type_) {
            case token_type::server_name:
            case token_type::autoindex:
            case token_type::root:
            case token_type::index:
            case token_type::listen:
            case token_type::error_pages:
            case token_type::redirect: {
                auto result = parse_directive(current.type_);
                if (!result)
                    return base::unexpected(result.error());
                add_server_directive(server, result.value());
                break;
            }
            case token_type::location: {
                auto result = parse_location(1);
                if (!result)
                    return base::unexpected(result.error());
                server.locations.push_back(result.value());
                break;
            }
            case token_type::fastcgi: {
                auto result = parse_fastcgi();
                if (!result)
                    return base::unexpected(result.error());
                break;
            }
            default: return NOT_ALLOWED(current.type_, token_type::server);
        }
    }
    return UNEXPECTED_TOKEN_ERROR(token_type::rbrace, token_type::end);
}

base::expected<void, parse_error> parser::parse() {
    pos_ = 0;
    conf_ = main_config{};
    while (!eof()) {
        const auto current = next();
        line_ = current.line_;
        column_ = current.column_;
        if (current.type_ != token_type::server)
            return NOT_ALLOWED(current.type_, token_type::none);
        auto result = parse_server();
        if (!result)
            return base::unexpected(result.error());
        conf_.servers.push_back(result.value());
    }

    return {};
}

const main_config& parser::get_main_conf() const {
    return conf_;
}

}
