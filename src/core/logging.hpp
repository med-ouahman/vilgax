
#pragma once

#include <string>
#include <ostream>
#include <iostream>

class logging {

public:
    void log(std::ostream& out = std::cout, const std::string& message) {
        out << message << std::endl;
    }
    
    void log(std::ostream& out = std::cout, const char* message, size_t size) {
        out.write(message, size);
        out << std::endl;
    }

};