#pragma once

#include <string>
#include <vector>
#include <cctype>

#include "tokens/Function.h"
#include "tokens/Number.h"
#include "tokens/Token.h"
#include "tokens/Grouping.h"
#include "tokens/Operator.h"

namespace Parser
{
    inline void initialParse(std::string& equation, std::vector<Token*>& tokens)
    {
        std::string token{};
        bool isNumber{};
        for (char c : equation)
        {
            if (c == '.' || std::isdigit(c))
            {
                if (!isNumber && !token.empty())
                {
                    tokens.push_back(new Function{token});
                    token.clear();
                }

                token.push_back(c);
                isNumber = true;
                continue;
            }

            if (isNumber)
            {
                tokens.push_back(new Number{std::stold(token)});
                token.clear();
            }

            isNumber = false;
            
            if (!isalpha(c))
            {
                if (!token.empty())
                {
                    tokens.push_back(new Function{token});
                    token.clear();
                }

                if (c != ' ')
                    tokens.push_back(c == '(' || c == ')' ? (Token*)new Grouping{c} : new Operator{c});
                continue;
            }

            token.push_back(c);
        }
        if (!token.empty())
            tokens.push_back(isNumber ? (Token*)new Number{std::stold(token)} : new Function{token});
    }
}
