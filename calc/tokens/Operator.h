#pragma once

#include <vector>

#include "Token.h"

struct Operator : Token
{
    char symbol;
    int orderOfOp;

    Operator(char symbol) : Token{TokenType::Operator}, symbol{symbol}, orderOfOp{getOrderOfOp(symbol)}/*, opFunc{getOpFunc(symbol)}*/ {}

    static int getOrderOfOp(char symbol);
    static void addMissingOperators(std::vector<Token*>& tokens);
    long double runOp(long double l, long double r) const;
};