#include "lexer.hpp"

namespace config {

lexer::lexer(std::string_view& s)
    : source_(s),
    pos_(0),
    line_(0),
    column_(0) {}

lexer::~lexer() {}

bool lexer::eof() const {
    return pos_ >= source_.size();
}

char lexer::consume() {
    if (pos_ == source_.size())
        return '\0';
    return source_[pos_++];
}


std::expected<token, lexer_error> lexer::next() {
    string word;
    while (!eof()) {
        char c = consume();
        if (c == '\n') {
            ++line_;
            column_ = 0;
        } else {
            ++column_;
        }
        auto type = type_of(c);
        switch (type) {
            case char_type::symbol:
                return token(token_type_of(c), c);
            case 
        }
    }
}

std::expected<void, lexer_error> lexer::lex() {
    while (true) {
        auto token = next();
        if (!token)
            return std::unexpected(token.error());
        tokens_.push_back(token.value());
    }
    return {};
}

}
