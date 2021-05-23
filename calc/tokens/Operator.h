#pragma once

#include <limits>
#include <vector>

#include "Token.h"
#include "Grouping.h"

struct Operator : Token
{
    char symbol;
    int orderOfOp;

    Operator(char symbol) : Token{TokenType::Operator}, symbol{symbol}, orderOfOp{getOrderOfOp(symbol)} {}

    static int getOrderOfOp(char symbol)
    {
        switch (symbol)
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
        default:
            return std::numeric_limits<int>::max();
        }
    }

    // This doesn't currently handle something like '5!5'; I need to determine whether I should handle it here or if I can change the equation to be 'fact(5)5' before calling this function
    static void addMissingOperators(std::vector<Token*>& tokens)
    {
        for (int i = 1; i < tokens.size(); i++)
        {
            // MUST exclude the pattern of function then grouping, otherwise "func()" => "func*()"; I probably don't need to worry about whether it is an open grouping or not, as if it were a close grouping right after the function, it would be invalid anyway and should have already been taken care of
            if (tokens[i - 1]->tokenType == TokenType::Function && tokens[i]->tokenType == TokenType::Grouping)
                continue;
            // [num/func/)]_[num/func/(]
            if ((tokens[i - 1]->tokenType == TokenType::Number || tokens[i - 1]->tokenType == TokenType::Function || tokens[i - 1]->tokenType == TokenType::Grouping && !((Grouping*)tokens[i - 1])->isOpen) &&
               (tokens[i]->tokenType == TokenType::Number || tokens[i]->tokenType == TokenType::Function || tokens[i]->tokenType == TokenType::Grouping && ((Grouping*)tokens[i])->isOpen))
                tokens.insert(tokens.begin() + i++, new Operator{'*'});
        }
    }
};