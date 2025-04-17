#pragma once

/**
 * @file tribool.hpp
 * @author karurochari
 * @brief Implementation of a tree state flag.
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <string_view>
class tribool {
public:
    enum unknown_t { unknown = 2 };

    constexpr tribool() : data(unknown){}
    constexpr tribool(bool v) : data(v){}
    constexpr tribool(unknown_t) : data(unknown){}

    constexpr tribool operator!() const{
        static const tribool lookup[3] = { true, false, unknown };
        return lookup[data];
    }

    constexpr tribool operator&&(bool t) const { return *this && tribool(t); }
    constexpr tribool operator&&(tribool t) const {
        static const tribool lookup[3][3] = { { false, false, false },
                                              { false, true, unknown },
                                              { false, unknown, unknown } };
        return lookup[data][t.data];
    }

    constexpr tribool operator||(bool t) const { return *this || tribool(t); }
    constexpr tribool operator||(tribool t) const {
        static const tribool lookup[3][3] = { { false, true, unknown },
                                              { true, true, true },
                                              { unknown, true, unknown } };
        return lookup[data][t.data];
    }

    constexpr tribool operator==(tribool t) const {
        static const tribool lookup[3][3] = { { true, false, unknown },
                                              { false, true, unknown },
                                              { unknown, unknown, unknown } };
        return lookup[data][t.data];
    }

    constexpr tribool operator==(bool t) const {
        return data!=unknown ? unknown
                         : static_cast<tribool>(static_cast<bool>(data) == t);
    }

    constexpr tribool operator!=(tribool t) const {
        static const tribool lookup[3][3] = { { false, true, unknown },
                                              { true, false, unknown },
                                              { unknown, unknown, unknown } };
        return lookup[data][t.data];
    }

    constexpr tribool operator!=(bool t) const{
        return data!=unknown ? unknown
                         : static_cast<tribool>(static_cast<bool>(data) != t);
    }

    constexpr operator bool() const { return true == static_cast<bool>(data); }

    constexpr std::string_view to_chars() const{
        static char const* lookup[3] = { "false", "true", "unknown" };
        return lookup[data];
    }

    constexpr inline tribool operator==(tribool::unknown_t){
        return tribool::unknown;
    }

    constexpr inline tribool operator!=(tribool::unknown_t){
        return tribool::unknown;
    }

public:
    int data;
};

