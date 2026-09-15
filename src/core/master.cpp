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

    config::parser parser(lexer_.tokens());
    // auto tokens = lexer_.tokens();
    // for (auto a: tokens) {
    //     config::print_token(a);
    // }
    auto result = parser.parse();
    if (!result) {
        config::print_parse_error(result.error(), conf);
        return false;
    }

    return true;
}

}