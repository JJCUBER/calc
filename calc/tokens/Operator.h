#pragma once

// #include <unordered_map>

#include "Token.h"

struct Operator : Token
{
    char symbol;
    int orderOfOp;

    Operator(char symbol) : Token{TokenType::Operator}, symbol{symbol}, orderOfOp{getOrderOfOp(symbol)} {}

    static int getOrderOfOp(char symbol)
    {
        switch(symbol)
        {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
            return 2;
        case '^':
            return 3;
        case '!':
            return 4;
        case '%':
            return 5;
        // Not entirely sure how I want to handle default as of yet, maybe return -1 instead?
        default: return INT_MAX;
        }
    }
};