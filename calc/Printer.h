#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iostream>

#include "tokens/Token.h"
#include "tokens/Number.h"
#include "tokens/Operator.h"
#include "tokens/Grouping.h"
#include "tokens/Function.h"

namespace Printer
{
    // There is probably a better word to use here than "reason"
    inline void printParsed(const std::vector<Token*>& tokens, const std::string& reason)
    // static void printParsed(const std::vector<Token*>& tokens, const std::string& reason)
    {
    #ifdef DONT_PRINT_PARSED
        return;
    #endif

        // Using ostringstream since I can specify the proper precision and it is much faster than cout'ing every part along the way
        std::ostringstream output{(reason.empty() ? "" : '[' + reason + "]: "), std::ios_base::ate};
        output.precision(12);
        for (Token* t : tokens)
        {
            switch (t->tokenType)
            {
            case Token::TokenType::Number:
                output << ((Number*)t)->n;
                break;
            case Token::TokenType::Operator:
                output << ((Operator*)t)->symbol;
                break;
            case Token::TokenType::Grouping:
                output << ((Grouping*)t)->symbol;
                break;
            case Token::TokenType::Function:
                output << ((Function*)t)->name;
                break;
            }
            output << '_';
        }
        output << '\n';
        std::cout << output.str();
    }
}
