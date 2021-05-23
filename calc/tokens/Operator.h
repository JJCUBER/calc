#pragma once

#include <vector>

#include "Token.h"

struct Operator : Token
{
    char symbol;
    int orderOfOp;

    Operator(char symbol) : Token{TokenType::Operator}, symbol{symbol}, orderOfOp{getOrderOfOp(symbol)} {}

    static int getOrderOfOp(char symbol);
    static void addMissingOperators(std::vector<Token*>& tokens);
};