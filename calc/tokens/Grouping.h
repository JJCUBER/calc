#pragma once

#include "Token.h"

struct Grouping : Token
{
    bool isOpen;

    Grouping(char symbol) : Token{TokenType::Grouping}, isOpen{symbol == '('} {}
};
