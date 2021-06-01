#pragma once

#include <vector>
#include <iostream>
#include <limits>

#include "tokens/Token.h"
#include "tokens/Number.h"
#include "tokens/Grouping.h"
#include "tokens/Function.h"
#include "tokens/Operator.h"

namespace Evaluator
{
    inline long double evaluate(const std::vector<Token*>& tokens, int pos, int len)
    {
        if (len == 1)
        {
            if (tokens[pos]->tokenType != Token::TokenType::Number)
            {
                // Might want to output this same warning if len <= 0?  I don't think that this function can ever be called with len <= 0 though
                std::cout << "Warning: Uncaught malformed input, answer might be wrong\n";
                return 0;
            }
            return ((Number*)tokens[pos])->n;
        }

        // int opPos{std::numeric_limits<int>::max()}, opPrecendence{std::numeric_limits<int>::max()}, depth{};
        int depth{};
        bool hasOffset{tokens[pos]->tokenType == Token::TokenType::Function};
        // This checks to make sure that the sector passed to this function is of the form "(...)"/"func(...)" and not something like "(...)...(...)"/"func(...)...(...)"
        // Don't need to check that the first is an open grouping and the second is a close grouping, as this should already be ensured by checks and the way that "sectors" are passed to this recursive function; an unmatched set of groupings can never make it here
        if (tokens[pos + hasOffset]->tokenType == Token::TokenType::Grouping && tokens[pos + len - 1]->tokenType == Token::TokenType::Grouping)
        {
            // Empty groupings "()"; doesn't take into account "[func]()," but I don't think that a function with no parameter can get to here (should log an error earlier on)
            // In fact, I should actually make it so that this isn't valid input (having groupings directly after each other like this "()")
            // if (len == 2)
                // return 0;

            for (int i = pos + len - 1; i >= pos + 1 + hasOffset; i--)
            {
                if (tokens[i]->tokenType != Token::TokenType::Grouping)
                    continue;
                depth += (((Grouping*)tokens[i])->isOpen << 1) - 1;
                if (!depth)
                    break;
            }

            if (depth)
            {
                if (hasOffset)
                    // std::cout << "evaluate(tokens, " << pos + 2 << ", " << len - 3 << ")\n";
                    return ((Function*)tokens[pos])->runFunc(evaluate(tokens, pos + 2, len - 3));
                // std::cout << "evaluate(tokens, " << pos + 1 << ", " << len - 2 << ")\n";
                return evaluate(tokens, pos + 1, len - 2);
            }
        }

        int opPos{std::numeric_limits<int>::max()}, minOrderOfOp{std::numeric_limits<int>::max()};
        depth = 0;
        // Very first and last shouldn't be operators, so no need to check them
        // for (int i = pos + len - 2; i >= pos + 1; i--)
        // However, I do have to go from the very end to the very beginning because one side can have a grouping which I need to account for
        for (int i = pos + len - 1; i >= pos; i--)
        {
            // Keep track of grouping depth
            if (tokens[i]->tokenType == Token::TokenType::Grouping)
            {
                depth += (((Grouping*)tokens[i])->isOpen << 1) - 1;
                continue;
            }

            // Continue if at the wrong depth or not on an operator
            if (depth || tokens[i]->tokenType != Token::TokenType::Operator)
                continue;

            // Update stored operator information if it has lower precedence (order of operations) than the current
            Operator* op{(Operator*)tokens[i]};
            if (op->orderOfOp >= minOrderOfOp)
                continue;
            minOrderOfOp = op->orderOfOp;
            opPos = i;
        }




        // std::cout << "evaluate(tokens, " << pos << ", " << opPos - pos << ") | evaluate(tokens, " << opPos + 1 << ", " << len + pos - opPos - 1 << ")\n";
        return ((Operator*)tokens[opPos])->runOp(evaluate(tokens, pos, opPos - pos), evaluate(tokens, opPos + 1, len + pos - opPos - 1));
    }
}
