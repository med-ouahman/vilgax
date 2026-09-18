#pragma once

#include <string>
#include <stdexcept>
#include "expected.hpp"

namespace base {

enum class number_error {
    invalid_argument,
    out_of_range
};

template <typename T>
expected<T, number_error> parse_number(const std::string& str, int base = 10);


template <>
inline expected<int, number_error> parse_number<int>(const std::string& str, int base) {
    try {
        return expected<int, number_error>(std::stoi(str, 0, base));
    } catch (const std::invalid_argument&) {
        return unexpected(number_error::invalid_argument);
    } catch (const std::out_of_range&) {
        return unexpected(number_error::out_of_range);
    }
}

template <>
inline expected<long, number_error> parse_number<long>(const std::string& str, int base) {
    try {
        return expected<long, number_error>(std::stol(str, 0, base));
    } catch (const std::invalid_argument&) {
        return unexpected(number_error::invalid_argument);
    } catch (const std::out_of_range&) {
        return unexpected(number_error::out_of_range);
    }
}

template <>
inline expected<unsigned long, number_error> parse_number<unsigned long>(const std::string& str, int base) {
    try {
        return expected<unsigned long, number_error>(std::stoul(str, 0, base));
    } catch (const std::invalid_argument&) {
        return unexpected(number_error::invalid_argument);
    } catch (const std::out_of_range&) {
        return unexpected(number_error::out_of_range);
    }
}

template <>
inline expected<long long, number_error> parse_number<long long>(const std::string& str, int base) {
    try {
        return expected<long long, number_error>(std::stoll(str, 0, base));
    } catch (const std::invalid_argument&) {
        return unexpected(number_error::invalid_argument);
    } catch (const std::out_of_range&) {
        return unexpected(number_error::out_of_range);
    }
}

template <>
inline expected<float, number_error> parse_number<float>(const std::string& str, int base) {
    (void)base;
    try {
        return expected<float, number_error>(std::stof(str));
    } catch (const std::invalid_argument&) {
        return unexpected(number_error::invalid_argument);
    } catch (const std::out_of_range&) {
        return unexpected(number_error::out_of_range);
    }
}

template <>
inline expected<double, number_error> parse_number<double>(const std::string& str, int base) {
    (void)base;
    try {
        return expected<double, number_error>(std::stod(str));
    } catch (const std::invalid_argument&) {
        return unexpected(number_error::invalid_argument);
    } catch (const std::out_of_range&) {
        return unexpected(number_error::out_of_range);
    }
}

} // namespace base