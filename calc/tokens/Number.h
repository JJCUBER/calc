#pragma once

#include <string>

#include "Token.h"

struct Number : Token
{
    long double n;

    Number(long double n) : Token{TokenType::Number}, n{n} {}

    static long double getConstant(const std::string& s);
};
