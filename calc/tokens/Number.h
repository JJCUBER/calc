#pragma once

#include <unordered_map>
#include <string>
#include <utility>

#include "Token.h"

struct Number : Token
{
    static const std::unordered_map<std::string, long double> constants;
    static std::unordered_map<std::string, long double> customConstants;

    long double n;

    Number(long double n) : Token{TokenType::Number}, n{n} {}

    static std::pair<bool, long double> getConstant(const std::string& s);
    static int tryAddCustomConstant(const std::string& s, long double n);
    static int tryRemoveCustomConstant(const std::string& s);
    static std::string handleCustomConstantCreation(std::vector<Token*>& tokens);
};
