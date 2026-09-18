

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include "baselib/expected.hpp"
#include "types.hpp"
#include "baselib/string.hpp"
#include "baselib/stoi.hpp"

struct parse_error {
    int err;

    parse_error(): err(1) {

    };

};

static base::expected<ipv6, parse_error>
parse_ipv6(const string& ip_str) {

    auto first = ip_str.find("::");
    auto last = ip_str.rfind("::");

    if (first != last) {
        return base::unexpected(parse_error());
    }

    if (first == string::npos) {
        ipv6 address;
        auto hextets = base::split_string(ip_str, ":");
        
        if (hextets.size() != 8) {
            return base::unexpected(parse_error());
        }

        for (const auto& hextet: hextets) {
            auto group = base::parse_number<usize>(hextet, 16);
            if (!group || group.value() > 0xffff) return base::unexpected(parse_error());
            address = (address << 16) | group.value();
        }
        return address;
    }
    
}


int main(int argc, char** argv) {
    
}
