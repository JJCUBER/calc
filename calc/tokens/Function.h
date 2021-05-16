#pragma once

#include <string>
#include <utility>

#include "Token.h"

struct Function : Token
{
    std::string name;

    Function(std::string name) : Token{TokenType::Function}, name{std::move(name)} {}
};
