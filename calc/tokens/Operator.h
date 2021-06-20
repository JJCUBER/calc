#pragma once

#include <vector>

#include "Token.h"

struct Operator : Token
{
    // Unknown should come last so that it has the highest operator precedence
    enum class OrderOfOpType {Or, And, Equal, Modulus, AddSubtract, MultiplyDivide, Power, Factorial, Unknown};

    char symbol;
    OrderOfOpType orderOfOp;

    Operator(char symbol) : Token{TokenType::Operator}, symbol{symbol}, orderOfOp{getOrderOfOp(symbol)} {}

    static OrderOfOpType getOrderOfOp(char symbol);
    static void addMissingOperators(std::vector<Token*>& tokens);
    static void combineSigns(std::vector<Token*>& tokens);
    static void applySigns(std::vector<Token*>& tokens);
    // static void replaceEquals(std::vector<Token*>& tokens);
    static bool ensureProperDoubleOperators(std::vector<Token*>& tokens);
    static void replaceDoubleOperators(std::vector<Token*>& tokens);

    void changeOperator(char symbol, OrderOfOpType orderOfOp = OrderOfOpType::Unknown);
    long double runOp(long double l, long double r) const;
};