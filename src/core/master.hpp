#pragma once

#include <vector>
#include <string>

namespace core {

class master {
private:

    bool load_config(const std::string& conf);
public:
    master(const std::string& conf);
    ~master();
    int start();
};

}