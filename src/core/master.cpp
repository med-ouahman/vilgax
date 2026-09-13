#include "master.hpp"
#include "worker.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include "parser.hpp"
#include "lexer.hpp"

namespace core {

master::master(const std::string& conf) {
    
    if (!load_config(conf)) exit(1);
    
}

master::~master() {

}

int master::start() {
    return 0;
}

bool master::load_config(const std::string& conf) {
    std::ifstream file(conf);

    if (file.fail()) {
        std::cout << "Error: Cannot open configuration file\n";
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
        std::cout << "Lexer error: " << config::lexer_error_code_phrase(e.code_) << " line: " << e.line_ << " column: " << e.column_ << std::endl;
        return false;
    }

    auto tokens = lexer_.tokens();
    for ( auto const& t: tokens) { print_token(t); }
    config::parser parser(tokens);
    auto result = parser.parse();
    if (!result) {
        
        const auto& err = result.error();

        if (err.code == config::parse_error_code::unexpected_token) {
            std::cout << "Parse error: " << unexpected_token_error(err.expected, err.found)
            << " "
            << config::get_line_column(err.line, err.column) << std::endl;
        } else if (err.code == config::parse_error::not_allowed) {
            std::cout << "Parse error: " << token_not_allowed(err.not_allowed, err.context) << config::get_line_column(err.line, err.column) << "\n";
        }
        
        return false;
    }

    return true;
}

}