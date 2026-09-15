
#include <string>
#include <stdexcept>

#include "expected.hpp"

namespace base {

enum class stoi_error {
    invalid_argument,
    out_of_range
};

expected<int, stoi_error> stoi(const std::string& str) {
    try {
        return std::stoi(str);
    }
    catch (const std::invalid_argument&) {
        return unexpected(stoi_error::invalid_argument);
    }
    catch (const std::out_of_range&) {
        return unexpected(stoi_error::out_of_range);
    }
}

}
