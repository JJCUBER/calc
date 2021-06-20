#pragma once

#include <vector>

#include "Token.h"

struct Grouping : Token
{
    char symbol;
    bool isOpen;

    Grouping(char symbol) : Token{TokenType::Grouping}, symbol{symbol}, isOpen{symbol == '('} {}

    static bool addMissingFunctionGroupings(std::vector<Token*>& tokens);
    static void addMissingOuterGroupings(std::vector<Token*>& tokens);
};
