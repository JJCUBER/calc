#pragma once

#include <unordered_map>
#include <string>

#include "Token.h"

struct Number : Token
{
    static std::unordered_map<std::string, long double> customConstants;

    long double n;

    Number(long double n) : Token{TokenType::Number}, n{n} {}

    static long double getConstant(const std::string& s);
};
