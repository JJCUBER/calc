#pragma once

#include "Token.h"
#include <string>
#include <utility>

struct Function : Token
{
    std::string name;

    Function(std::string name) : Token{TokenType::Function}, name{std::move(name)} {}
};
