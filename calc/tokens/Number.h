#pragma once
#include "Token.h"

struct Number : Token
{
    long double n;

    Number(long double n) : Token{TokenType::Number}, n{n} {}
};
