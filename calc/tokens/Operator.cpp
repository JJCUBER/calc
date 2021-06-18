#include <iostream>
#include <cmath>

#include "Operator.h"

#include "Grouping.h"
#include "Function.h"

Operator::OrderOfOpType Operator::getOrderOfOp(char symbol)
{
    // I could potentially just make the cases fall through and go from highest to lowest, with each on incrementing a number so that I don't have hardcoded numbers (in case the order ever needs to change/get corrected again).  Even then, the way that I am relying on hardcoded numbers (including in some checks in "calc.cpp") is not good and I should find another solution at some point.
    switch (symbol)
    {
    case '|':
        return OrderOfOpType::Or;
    case '&':
        return OrderOfOpType::And;
    case '=':
        return OrderOfOpType::Equal;
    case '%':
        return OrderOfOpType::Modulus;
    case '+':
    case '-':
        return OrderOfOpType::AddSubtract;
    case '*':
    case '/':
        return OrderOfOpType::MultiplyDivide;
    case '^':
        return OrderOfOpType::Power;
    // Assuming that I do end up converting factorials to a function like "fact()", this case probably won't be needed
    case '!':
        return OrderOfOpType::Factorial;
    default:
        return OrderOfOpType::Unknown;
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

void Operator::combineSigns(std::vector<Token*>& tokens)
{
    // TODO: It would be optimal to go backwards instead, as inserting is optimal left to right and erasing is optimal right to left (for insertion, you want to insert before the next one so as to minimize the amount of elements moved; for erasure, you want to erase after the next one so as to minimize the amount of elements moved; you want to add as little and remove as many elements as possible that are after the current element ahead of time to minimize the amount of elements moved overall)
    int start{}, len{};
    bool isNegative{};
    for (int i = 0; i < tokens.size(); i++)
    {
        if (tokens[i]->tokenType != TokenType::Operator || ((Operator*)tokens[i])->orderOfOp != OrderOfOpType::AddSubtract)
        {
            if (!len)
                continue;

            // remove at start with len-1, replace [i] with proper value
            for (int j = start; j < start + len - 1; j++)
                delete tokens[j];
            tokens.erase(tokens.begin() + start, tokens.begin() + start + len - 1);
            i -= len - 1;
            ((Operator*)tokens[i - 1])->changeOperator(isNegative ? '-' : '+', OrderOfOpType::AddSubtract);
            len = 0;
            continue;
        }

        Operator* op{(Operator*)tokens[i]};
        if (!len++)
        {
            start = i;
            isNegative = op->symbol == '-';
            continue;
        }

        // If the signs are different, then it should be negative, and if they are the same, it should be positive (+-|-+ => -, ++|-- => +)
        isNegative ^= op->symbol == '-';
    }

    /* I don't think that I'll ever need to handle this, as this would require the equation ending with a symbol, which is invalid
    if (!len)
        return;

    // Handle unfinished part
    for (int j = start; j < start + len - 1; j++)
        delete tokens[j];
    tokens.erase(tokens.begin() + start, tokens.begin() + start + len - 1);
    ((Operator*)tokens.back())->changeOperator(isNegative ? '-' : '+');
    */
}

void Operator::applySigns(std::vector<Token*>& tokens)
{
    // TODO: It would be optimal to go backwards over the for loop so as to erase from right to left, along with handling the special case for the first element last
    int i{};
    auto apply = [&]()
    {
        bool isPositive{((Operator*)tokens[i])->symbol == '+'};
        delete tokens[i];
        if (isPositive)
            tokens.erase(tokens.begin() + i);
        else
            tokens[i++] = new Function{"neg"};
        // This is to skip forward, as the current i has to be a function/number, otherwise the input would be malformed, i after incrementing by 1 would be preceded by a non-operator, making it irrelevant in the loop; the for loop would increment one more time to be in the first spot with an actual chance of being relevant
        i++;
    };

    // Handle the equation starting with a -
    if (!tokens.empty() && tokens[0]->tokenType == TokenType::Operator)
    {
        // I don't take into account the equation starting with an operator other than +/-, as it shouldn't have passed malformed input checks
        apply();
        i++;
    }
    else
        // Earliest point that there can be a sign after an operator if the equation doesn't start with a sign: "(-1..."
        i = 1;


    // if a sign is directly after an operator (since signs have been combined, I don't need to check for them being after a specific operator, any will work), then I should remove it if it is +, or wrap the proceeding number/function/grouping with the function neg()

    for (; i < tokens.size(); i++)
    {
        // have to also ensure that I don't have consecutive factorials, but it is better to just ensure that I have the right orderOfOps instead to allow for potential future edge cases

        if (tokens[i]->tokenType != TokenType::Operator || ((Operator*)tokens[i])->orderOfOp != OrderOfOpType::AddSubtract)
            continue;

        if (tokens[i - 1]->tokenType == TokenType::Operator ||
            tokens[i - 1]->tokenType == TokenType::Grouping && ((Grouping*)tokens[i - 1])->isOpen)
            apply();

        // don't need to handle wrapping here, as my implicit grouping wrapper for all functions should take care of it after the fact
    }
}

int Operator::ensureProperDoubleOperators(std::vector<Token*>& tokens)
{
    // scan for any lone equals/ands/ors
    for (int i = 1; i < tokens.size() - 1; i++)
    {
        // This currently only handles Equal, need to add support for | and &, also should probably compare using the symbol, as order of operations doesn't necessarily have a hard value for every type (example being AddSubtract and MultiplyDivide)
        /*
        if (tokens[i]->tokenType != Token::TokenType::Operator || ((Operator*)tokens[i])->orderOfOp != Operator::OrderOfOpType::Equal ||
            tokens[i - 1]->tokenType == Token::TokenType::Operator && ((Operator*)tokens[i - 1])->orderOfOp == Operator::OrderOfOpType::Equal ||
            tokens[i + 1]->tokenType == Token::TokenType::Operator && ((Operator*)tokens[i + 1])->orderOfOp == Operator::OrderOfOpType::Equal)
            continue;
        */
        if (tokens[i]->tokenType != TokenType::Operator)
            continue;
        Operator* currOp{(Operator*)tokens[i]};
        if (currOp->orderOfOp > OrderOfOpType::Equal ||
            tokens[i - 1]->tokenType == TokenType::Operator && ((Operator*)tokens[i - 1])->orderOfOp == currOp->orderOfOp ||
            tokens[i + 1]->tokenType == TokenType::Operator && ((Operator*)tokens[i + 1])->orderOfOp == currOp->orderOfOp)
            continue;

        std::cout << "Error: Malformed Input - input contains only one of a symbol that requires itself to be repeated twice: '" << currOp->symbol << "' (expected something of the format '" << std::string(2, currOp->symbol) << "')\n";
        return -1;
    }
    return 0;
}

// void Operator::replaceEquals(std::vector<Token*>& tokens)
void Operator::replaceDoubleOperators(std::vector<Token*>& tokens)
{
    // TODO: Currently, chained =='s each individually return a 1 or 0, making 2==2==2 evaluate to 2==1 which is 0/false.  Determine whether this is how I want to keep the functionality as, or if I want to change it to support chained =='s
    // handle ||'s, &&'s, and =='s (converts them to |, &, and = respectively)
    for (int i = tokens.size() - 2; i >= 0; i--)
    {
        if (tokens[i]->tokenType != TokenType::Operator || tokens[i + 1]->tokenType != TokenType::Operator)
            continue;

        /*
        Operator* currOp{(Operator*)tokens[i]};
        if (currOp->symbol != ((Operator*)tokens[i + 1])->symbol ||
            currOp->symbol != '|' && currOp->symbol != '&' && currOp->symbol != '=')
            continue;
        */
        Operator* currOp{(Operator*)tokens[i]};
        if (currOp->symbol != ((Operator*)tokens[i + 1])->symbol || currOp->orderOfOp > OrderOfOpType::Equal)
            continue;

        delete tokens[i + 1];
        // Extra decrement to i so as to prevent === from being considered valid; === and ==== would both convert to == (which is invalid syntax and will be caught as expected)
        tokens.erase(tokens.begin() + i-- + 1);
    }
}


void Operator::changeOperator(char symbol, OrderOfOpType orderOfOp)
{
    this->symbol = symbol;
    // Allows the option to specify the orderOfOp if already known instead of calling getOrderOfOp; the function should never be explicitly called with the value "Unknown," so calling the getOrderOfOp function when it is set to unknown should never be unnecessary
    this->orderOfOp = orderOfOp == OrderOfOpType::Unknown ? getOrderOfOp(symbol) : orderOfOp;
}

long double Operator::runOp(long double l, long double r) const
{
    switch (symbol)
    {
    case '|':
        return l || r;
    case '&':
        return l && r;
    // At this point, this is actually == sign; might want to do std::abs(l - r) <= epsilon instead of l == r?
    case '=':
        return l == r;
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
        // TODO: Cout warning uncaught unrecognized symbol, returning 0
        return 0;
    }
}
