#pragma once

#include <utility>

#if __cplusplus >= 202302L
    #include <expected>
#endif

namespace base {

template<typename E>
class unexpected_internal
{
private:
    E _error;

public:
    explicit unexpected_internal(const E& error)
        : _error(error)
    {}

    explicit unexpected_internal(E&& error)
        : _error(std::move(error))
    {}

    E& error()
    {
        return _error;
    }

    const E& error() const
    {
        return _error;
    }
};


template<typename T, typename E>
class expected_internal
{
private:
    bool _has_value;

    union {
        T _value;
        E _error;
    };

public:
    expected_internal(const T& value)
        : _has_value(true), _value(value)
    {}

    expected_internal(T&& value)
        : _has_value(true), _value(std::move(value))
    {}

    expected_internal(const unexpected_internal<E>& unexpected)
        : _has_value(false), _error(unexpected.error())
    {}

    expected_internal(unexpected_internal<E>&& unexpected)
        : _has_value(false), _error(std::move(unexpected.error()))
    {}

    ~expected_internal()
    {
        if (_has_value)
            _value.~T();
        else
            _error.~E();
    }

    bool has_value() const
    {
        return _has_value;
    }

    explicit operator bool() const
    {
        return _has_value;
    }

    T& value()
    {
        return _value;
    }

    const T& value() const
    {
        return _value;
    }

    E& error()
    {
        return _error;
    }

    const E& error() const
    {
        return _error;
    }
};


template<typename E>
class expected_internal<void, E>
{
private:
    bool _has_value;
    E _error;

public:
    expected_internal()
        : _has_value(true)
    {}

    expected_internal(const unexpected_internal<E>& unexpected)
        : _has_value(false), _error(unexpected.error())
    {}

    expected_internal(unexpected_internal<E>&& unexpected)
        : _has_value(false), _error(std::move(unexpected.error()))
    {}

    bool has_value() const
    {
        return _has_value;
    }

    explicit operator bool() const
    {
        return _has_value;
    }

    E& error()
    {
        return _error;
    }

    const E& error() const
    {
        return _error;
    }
};


#if __cplusplus >= 202302L

template<typename T, typename E>
using expected = std::expected<T, E>;

template<typename E>
using unexpected = std::unexpected<E>;

#else

template<typename T, typename E>
using expected = expected_internal<T, E>;

template<typename E>
using unexpected = unexpected_internal<E>;

#endif

}