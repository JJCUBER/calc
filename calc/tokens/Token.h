#pragma once

struct Token
{
    enum class TokenType { Number, Operator, Grouping, Function };
    TokenType tokenType;

    Token(TokenType tokenType) : tokenType{tokenType} {}
};
