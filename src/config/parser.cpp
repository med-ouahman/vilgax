#include "parser.hpp"
#include "baselib/stoi.hpp"
#include "baselib/string.hpp"
#include <arpa/inet.h>

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

static base::expected<ipv4, listen_error>
parse_ipv4(const string& ip_str) {
    in_addr addr;
    if (0 == inet_aton(ip_str.c_str(), std::addressof(addr)))
        return base::unexpected(listen_error(listen_error_code::invalid_ipv4));
    return static_cast<ipv4>(addr.s_addr);
}

static base::expected<ipv6, listen_error>
parse_ipv6(const string& ip_str) {

    auto first = ip_str.find("::");
    auto last = ip_str.rfind("::");

    if (first != last) {
        return base::unexpected(listen_error(listen_error_code::invalid_ipv6));
    }

    if (first == string::npos) {
        ipv6 address = 0;
        auto hextets = base::split_string(ip_str, ":");
        if (hextets.size() != 8) {
            return base::unexpected(listen_error(listen_error_code::invalid_ipv6));
        }

        for (const auto& hextet : hextets) {
            auto group = base::parse_number<usize>(hextet, 16);
            if (!group || group.value() > 0xffff) return base::unexpected(listen_error(listen_error_code::invalid_ipv6));
            address = (address << 16) | group.value();
        }
        return address;
    }

    string left_str  = ip_str.substr(0, first);
    string right_str = ip_str.substr(first + 2);

    std::vector<string> left_hextets;
    std::vector<string> right_hextets;

    if (!left_str.empty())  left_hextets  = base::split_string(left_str, ":");
    if (!right_str.empty()) right_hextets = base::split_string(right_str, ":");

    for (const auto& h : left_hextets)  if (h.empty()) {
        return base::unexpected(listen_error_code(listen_error_code::invalid_ipv4));
    }

    for (const auto& h : right_hextets) if (h.empty()) {
        return base::unexpected(listen_error_code(listen_error_code::invalid_ipv4));
    }

    auto parse_hextets = [](const std::vector<string>& hextets, std::vector<u16>& out) {
        for (const auto& hextet : hextets) {
            auto group = base::parse_number<usize>(hextet, 16);
            if (!group || group.value() > 0xffff) return false;
            out.push_back(static_cast<u16>(group.value()));
        }
        return true;
    };

    std::vector<u16> left_groups;
    if (!parse_hextets(left_hextets, left_groups)) {
        return base::unexpected(listen_error(listen_error_code::invalid_ipv6));
    }

    std::vector<u16> right_groups;

    if (!right_hextets.empty() &&
        right_hextets.back().find('.') != string::npos) {

        auto v4 = parse_ipv4(right_hextets.back());
        if (!v4) return base::unexpected(listen_error(listen_error_code::invalid_ipv6));

        right_hextets.pop_back();
        if (!parse_hextets(right_hextets, right_groups)) {
            return base::unexpected(listen_error(listen_error_code::invalid_ipv6));
        }

        u32 v4_addr = v4.value();
        right_groups.push_back(static_cast<u16>(v4_addr >> 16));
        right_groups.push_back(static_cast<u16>(v4_addr & 0xffff));
    } else {
        if (!parse_hextets(right_hextets, right_groups)) {
            return base::unexpected(listen_error(listen_error_code::invalid_ipv6));
        }
    }

    usize total = left_groups.size() + right_groups.size();
    if (total >= 8) {
        return base::unexpected(listen_error(listen_error_code::invalid_ipv6));
    }

    usize zero_groups = 8 - total;

    ipv6 address = 0;
    for (auto g : left_groups)  address = (address << 16) | g;
    for (usize i = 0; i < zero_groups; ++i) address = address << 16;
    for (auto g : right_groups) address = (address << 16) | g;

    return address;
}

static base::expected<ip_address, listen_error> parse_ip_address(const string& ip_str) {
    if (ip_str.find(':') == string::npos) {
        auto v4 = parse_ipv4(ip_str);
        if (!v4) {
            return base::unexpected(v4.error());
        }

        return ip_address(v4.value());
    }
    auto v6 = parse_ipv6(ip_str);
    if (!v6) {
        return base::unexpected(v6.error());
    }

    return ip_address(v6.value());
}

static base::expected<port_number, listen_error> parse_port_number(const string& port_str) {
    auto result = base::parse_number<usize>(port_str);
    if (!result) {
        return base::unexpected(listen_error(listen_error_code::invalid_port));
    }

    auto number = result.value();

    if (number > parser::max_port_number_) {
        return base::unexpected(listen_error(listen_error_code::port_out_of_range));
    }

    return number;
}

static base::expected<listen_endpoint, listen_error>
parse_listen(const std::vector<string>& values) {
   
    if (values.size() < 1 || values.size() > 2) {
        return base::unexpected(listen_error(listen_error_code::invalid_format));
    }

    const auto& ip_part = values[0];
    auto colon = ip_part.find_last_of(':');
    
    if (colon == string::npos) {
        return base::unexpected(listen_error(listen_error_code::invalid_format));
    }

    auto ip_str   = ip_part.substr(0, colon);
    auto port_str = ip_part.substr(colon + 1);
    
    auto address = parse_ip_address(ip_str);

    if (!address) return base::unexpected(address.error());
    
    auto port = parse_port_number(port_str);
    
    if (!port) {
        return base::unexpected(port.error());
    }

    auto backlog = 0uz;

    if (values.size() == 2) {
        auto result = base::parse_number<usize>(values[1]);
        if (!result)  return base::unexpected(listen_error_code::invalid_backlog);
        backlog = result.value();
    }

    return listen_endpoint{address.value(), port.value(), backlog};
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
            auto result = base::parse_number<usize>(directive.values[1]);
            if (!result) {
                return base::unexpected(parse_error());
            }
            server.redirect.code = result.value();
            break;
        }

        case token_type::listen: {
            auto listen = parse_listen(directive.values);
            if (!listen) return base::unexpected(parse_error());
            server.listens.push_back(listen.value());
            break;
        }

        default: return base::unexpected(parse_error());
    }
    return {};
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
        case parse_error_code::listen_error:
            std::cout << error.listen_err.message() << "\n";
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
