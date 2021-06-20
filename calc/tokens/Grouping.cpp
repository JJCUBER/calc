#include <iostream>

#include "Grouping.h"

#include "Function.h"
#include "Operator.h"

bool Grouping::addMissingFunctionGroupings(std::vector<Token*>& tokens)
{
    // TODO: It would be more optimal to go from left to right since this is doing insertions, but the code currently relies on functions to the right already having their groupings so as to make it easier to wrap around it

    // 1) Find each function position
    // 2) If it is followed directly by an open grouping, then continue on to look for another function
    // 3) Otherwise, wrap the item directly after it with groupings (if the item directly after it is a function, will have to wrap around the function and the grouping after it: "lnln(5)" => "ln(ln(5))")
    // Note: Due to step 3, I should go right to left (this is to handle nested functions like "lnlnlnlnln100")
    for (int i = tokens.size() - 2; i >= 0; i--)
    {
        if (tokens[i]->tokenType != TokenType::Function)
            continue;

        switch (tokens[i + 1]->tokenType)
        {
        case TokenType::Grouping:
            if (((Grouping*)tokens[i + 1])->isOpen)
                break;
            std::cout << "Error: Malformed Input - input has a closing grouping directly after a function: '" << ((Function*)tokens[i])->name << "'\n";
            return false;
        case TokenType::Function:
            tokens.insert(tokens.begin() + ++i, new Grouping{'('});
            for (int j = i + 1; j < tokens.size(); j++)
            {
                if (tokens[j]->tokenType != TokenType::Grouping || ((Grouping*)tokens[j])->isOpen)
                    continue;
                tokens.insert(tokens.begin() + j, new Grouping{')'});
                break;
            }
            break;
        case TokenType::Number:
            tokens.insert(tokens.begin() + ++i, new Grouping{'('});
            i++;
            tokens.insert(tokens.begin() + ++i, new Grouping{')'});
            i++;
            break;
        case TokenType::Operator:
            // I could also check for this in 'checkForMalformedInput(),' making it get caught earlier (as in having '[func]_[op]'), but I would need to exclude the case of '[func]_^' and either handle it here or earlier
            // Handle default (its an operator [which is invalid]; cout error)
            std::cout << "Error: UNCAUGHT Malformed Input - input has an operator other than '^' directly after a function: '" << ((Function*)tokens[i])->name << ((Operator*)tokens[i + 1])->symbol << "'\n";
            return false;
        }
    }


    return true;
}

void Grouping::addMissingOuterGroupings(std::vector<Token*>& tokens)
{
    // Should be done after initial checks; I can't see any cases currently which would pass the checks when they shouldn't by doing this after (something like '(1+' would still be invalid without inserting the ')', since the equation would end with an operator other than '!')


    // Get number of groupings, aka parenthesis to prepend and append
    int min{}, ct{};
    for (Token* t : tokens)
    {
        if (t->tokenType != TokenType::Grouping)
            continue;
        ct += (((Grouping*)t)->isOpen << 1) - 1;
        min = std::min(ct, min);
    }

    // Prepend and append proper groupings if needed
    if (min < 0)
    {
        for (int i = 0; i < -min; i++)
            tokens.insert(tokens.begin(), new Grouping('('));
        std::cout << "Warning: prepended " << -min << " '('\n";
    }
    if (ct - min > 0)
    {
        for (int i = 0; i < ct - min; i++)
            tokens.push_back(new Grouping(')'));
        std::cout << "Warning: appended " << ct - min << " ')'\n";
    }
}
