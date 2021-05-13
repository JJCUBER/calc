#pragma once

#include "Token.h"

struct Operator : Token
{
    char symbol;

    Operator(char symbol) : Token{TokenType::Operator}, symbol{symbol} {}
};