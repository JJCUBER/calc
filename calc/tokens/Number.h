#pragma once

#include <string>
#include <numbers>

#include "Token.h"

struct Number : Token
{
    long double n;

    Number(long double n) : Token{TokenType::Number}, n{n} {}

    /*
    static bool isConstant(const std::string& s)
    {
        return s == "e" || s == "pi" || s == "phi";
    }

    static long double getConstant(const std::string& s)
    {
        // It would probably be a good idea to do this in a less hardcoded way; this is meant to be temporary
        return s == "e" ? std::numbers::e : s == "pi" ? std::numbers::pi : s == "phi" ? std::numbers::phi : std::numeric_limits<long double>::max();
    }
    */
    static long double getConstant(const std::string& s)
    {
        // It would probably be a good idea to do this in a less hardcoded way; this is meant to be temporary
        return s == "e" ? std::numbers::e : s == "pi" ? std::numbers::pi : s == "phi" ? std::numbers::phi : 0.0L;
    }
};
