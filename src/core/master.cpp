#include "master.hpp"
#include "worker.hpp"
#include <cassert>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <set>
#include <string>
#include "parser.hpp"
#include "lexer.hpp"
#include "config.hpp"

namespace {

bool is_valid_host_name(const std::string& value) {
    if (value.empty()) {
        return false;
    }

    for (char c : value) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '.' || c == '_') {
            continue;
        }
        return false;
    }

    return true;
}

bool is_valid_path(const std::string& path) {
    return !path.empty() && path.front() == '/';
}

bool validate_location_tree(const config::location_config& location, std::set<std::string>& seen_paths) {
    if (!is_valid_path(location.path)) {
        std::cerr << "Invalid location path: " << location.path << "\n";
        return false;
    }

    if (!seen_paths.insert(location.path).second) {
        std::cerr << "Duplicate location path: " << location.path << "\n";
        return false;
    }

    if (!location.root.empty() && !is_valid_path(location.root)) {
        std::cerr << "Invalid root path for location " << location.path << ": " << location.root << "\n";
        return false;
    }

    for (const auto& child : location.locations) {
        if (!validate_location_tree(child, seen_paths)) {
            return false;
        }
    }

    return true;
}

} // namespace

namespace core {

master::master(const std::string& conf_filename) {
    
    if (!load_config(conf_filename)) {
        exit(1);
    }

    config::display_conf(conf_);
}

master::~master() {

}

int master::start() {
    return 0;
}

bool master::test_config(const config::main_config& conf) {
    const auto& runtime = conf.runtime_conf;
    const auto& http = conf.http_conf;
    const auto& timeouts = conf.http_timeout_conf;

    if (runtime.workers_auto) {
        // auto resolves to a system CPU count later in startup. It is valid only when
        // the resolved count is positive and fits within runtime limits.
    } else if (runtime.workers == 0) {
        std::cerr << "Invalid config: workers must be auto or a positive integer.\n";
        return false;
    }

    if (runtime.max_connections == 0 || runtime.max_connections_per_worker == 0) {
        std::cerr << "Invalid config: max_connections and max_connections_per_worker must be positive.\n";
        return false;
    }

    if (runtime.max_connections < runtime.max_connections_per_worker) {
        std::cerr << "Invalid config: max_connections must be >= max_connections_per_worker.\n";
        return false;
    }

    if (runtime.user.empty() || runtime.group.empty()) {
        std::cerr << "Invalid config: user and group must be non-empty.\n";
        return false;
    }

    if (http.max_request_line_size == 0 || http.max_header_size == 0 || http.max_headers == 0 ||
        http.max_requests_per_connection == 0 || http.sendfile_min_size == 0) {
        std::cerr << "Invalid config: all HTTP size and request limits must be positive.\n";
        return false;
    }

    if (timeouts.request_line == 0 || timeouts.headers == 0 || timeouts.body == 0 ||
        timeouts.keepalive == 0 || timeouts.write == 0) {
        std::cerr << "Invalid config: all timeout values must be positive.\n";
        return false;
    }

    for (const auto& server : conf.servers) {
        if (server.listens.empty()) {
            std::cerr << "Invalid config: server requires at least one listen endpoint.\n";
            return false;
        }

        if (server.root.empty() || !is_valid_path(server.root)) {
            std::cerr << "Invalid config: server root must be a non-empty absolute path.\n";
            return false;
        }

        for (const auto& name : server.server_names) {
            if (!is_valid_host_name(name)) {
                std::cerr << "Invalid config: server_name is empty or contains invalid characters: "
                          << name << "\n";
                return false;
            }
        }

        for (const auto& listen : server.listens) {
            const auto port = listen.port;
            if (port == 0) {
                std::cerr << "Invalid config: listen port must be non-zero.\n";
                return false;
            }
            if (listen.backlog == 0) {
                std::cerr << "Invalid config: listen backlog must be positive.\n";
                return false;
            }
        }

        if (server.redirect.code != 0 && server.redirect.location.empty()) {
            std::cerr << "Invalid config: redirect code requires a non-empty destination.\n";
            return false;
        }

        if (server.redirect.code != 0 && server.redirect.code != 301 && server.redirect.code != 302 &&
            server.redirect.code != 307 && server.redirect.code != 308) {
            std::cerr << "Invalid config: unsupported redirect code: " << server.redirect.code << "\n";
            return false;
        }

        std::set<std::string> seen_paths;
        for (const auto& location : server.locations) {
            if (!validate_location_tree(location, seen_paths)) {
                return false;
            }
        }
    }

    return true;
}

bool master::load_config(const std::string& conf_filename) {
    std::ifstream file(conf_filename);

    if (file.fail()) {
        std::cerr << "Error: Cannot open configuration file\n";
        return false;
    }

    file.seekg(0, std::ios::end);
    auto size = file.tellg();

    file.seekg(0, std::ios::beg);
    
    std::string buf(size, '\0');
    
    file.read(buf.data(), size);
    
    string_view v = buf;
    
    config::lexer lexer_(v);
    auto err = lexer_.lex();

    if (!err) {
        auto e = err.error();
        std::cerr << "Lexer error: " << config::lexer_error_code_phrase(e.code_) << " line: " << e.line_ << " column: " << e.column_ << std::endl;
        return false;
    }

    config::parser parser(lexer_.tokens());
    auto result = parser.parse();
    if (!result) {
        config::print_parse_error(result.error(), conf_filename);
        return false;
    }

    const auto& conf = parser.get_main_conf();
    if (!test_config(conf)) {
        std::cerr << "Configuration validation failed.\n";
        return false;
    }
    conf_ = std::move(conf);
    return true;
}

}