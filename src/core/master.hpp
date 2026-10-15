#pragma once

#include <string>
#include "config.hpp"

namespace core {

class master {
private:
    config::main_config conf_;
    bool load_config(const std::string& conf);
    bool test_config(const config::main_config& conf);
public:
    master(const std::string& conf);
    ~master();
    int start();
};

} // namespace config