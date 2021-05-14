#pragma once

#include "Token.h"

struct Grouping : Token
{
    char symbol;
    bool isOpen;

    Grouping(char symbol) : Token{TokenType::Grouping}, symbol{symbol}, isOpen{ symbol == '(' } {}
};
