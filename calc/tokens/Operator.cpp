#include <limits>
#include <cmath>

#include "Operator.h"
#include "Grouping.h"

int Operator::getOrderOfOp(char symbol)
{
    // I could potentially just make the cases fall through and go from highest to lowest, with each on incrementing a number so that I don't have hardcoded numbers (in case the order ever needs to change/get corrected again).  Even then, the way that I am relying on hardcoded numbers (including in some checks in "calc.cpp") is not good and I should find another solution at some point.
    switch (symbol)
    {
    case '%':
        return 1;
    case '+':
    case '-':
        return 2;
    case '*':
    case '/':
        return 3;
    case '^':
        return 4;
    // Assuming that I do end up converting factorials to a function like "fact()", this case probably won't be needed
    case '!':
        return 5;
    // Not entirely sure how I want to handle default as of yet, maybe return -1 instead?
    default:
        return std::numeric_limits<int>::max();
    }
}

// This doesn't currently handle something like '5!5'; I need to determine whether I should handle it here or if I can change the equation to be 'fact(5)5' before calling this function
void Operator::addMissingOperators(std::vector<Token*>& tokens)
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

long double Operator::runOp(long double l, long double r) const
{
    switch (symbol)
    {
    case '%':
        // slow
        // return std::fmod((r + std::fmod(l, r)), r);
        return l - r * std::floor(l / r);
    case '+':
        return l + r;
    case '-':
        return l - r;
    case '*':
        return l * r;
    case '/':
        return l / r;
    case '^':
        return std::pow(l, r);
    default:
        // Might want to handle this differently
        return 0;
    }
}
