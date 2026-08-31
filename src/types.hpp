#pragma once

/*
    Types

    make sure to always include this file last
*/

#include <stdint.h>
#include <cstddef>

using u16 = uint16_t;
using timer = uint32_t;
using usize = size_t;
using string = std::string;
using string_view = std::string_view;
using vector = std::vector

using ipv4 = uint32_t;

using ipv6 = __uint128_t;

using port_number = u16;