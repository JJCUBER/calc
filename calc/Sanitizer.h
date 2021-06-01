#pragma once

#include <vector>
#include <iostream>

#include "tokens/Token.h"
#include "tokens/Operator.h"
#include "tokens/Number.h"
#include "tokens/Grouping.h"
#include "tokens/Function.h"

#include "Printer.h"

namespace Sanitizer
{
    inline int checkForEarlyMalformedInput(const std::vector<Token*>& tokens)
    {
        // Starting with any operator other than '+' or '-'
        if (tokens[0]->tokenType == Token::TokenType::Operator)
        {
            Operator* op{(Operator*)tokens[0]};
            if (op->orderOfOp != 2)
            {
                std::cout << "Error: Malformed Input - input starts with an operator (or unknown symbol) other than '+' or '-': '" << op->symbol << "'\n";
                return -1;
            }
        }

        // Ending with any operator other than '!'
        if (tokens.back()->tokenType == Token::TokenType::Operator)
        {
            Operator* op{(Operator*)tokens.back()};
            if (op->symbol != '!')
            {
                std::cout << "Error: Malformed Input - input ends with an operator (or unknown symbol) other than '!': '" << op->symbol << "'\n";
                return -1;
            }
        }

        // Might also want to invert ifs involving a continue and return path/branch inside of them to be more consistent
        for (int i = 1; i < tokens.size(); i++)
        {
            // Consecutive numbers
            if (tokens[i]->tokenType == Token::TokenType::Number && tokens[i - 1]->tokenType == Token::TokenType::Number)
            {
                std::cout << "Error: Malformed Input - input has consecutive numbers: '" << ((Number*)tokens[i - 1])->n << "' '" << ((Number*)tokens[i])->n << "'\n";
                return -1;
            }

            // Might want to add handling of consecutive operators here for stuff like '5*-4,' '5++4,' '5--4,' '5---4,' etc. (might want to warn about extra +'s and -'s)
            // Will also need to handle functions after taking into account ^, as in 'floor(+5),' 'floor(-5),' and 'floor(---5)' should all be valid, but not 'floor(*5)'

            // Consecutive operators other than ![_] or [_][+-]
            if (tokens[i - 1]->tokenType == Token::TokenType::Operator && tokens[i]->tokenType == Token::TokenType::Operator)
            {
                Operator *prevOp{(Operator*)tokens[i - 1]}, *currOp{(Operator*)tokens[i]};
                // if (prevOp->orderOfOp == 2 && currOp->orderOfOp != 2)
                if (prevOp->symbol != '!' && currOp->orderOfOp != 2)
                {
                    std::cout << "Error: Malformed Input - input has consecutive operators other than '![_]' or '[_][+-]': '" << prevOp->symbol << currOp->symbol << "'\n";
                    return -1;
                }
                continue;
            }

            if (tokens[i - 1]->tokenType == Token::TokenType::Grouping)
            {
                Grouping* grouping{(Grouping*)tokens[i - 1]};
                if (grouping->isOpen)
                {
                    // Inside of group starts with any operator
                    if (tokens[i]->tokenType == Token::TokenType::Operator)
                    {
                        std::cout << "Error: Malformed Input - input has an operator directly after a grouping: '" << grouping->symbol << ((Operator*)tokens[i])->symbol << "'\n";
                        return -1;
                    }

                    // Empty set of groupings "()"
                    if (tokens[i]->tokenType == Token::TokenType::Grouping && !((Grouping*)tokens[i])->isOpen)
                    {
                        std::cout << "Error: Malformed Input - input has a set of groupings with nothing inside: '()'\n";
                        return -1;
                    }
                }
            }
            // Inside of group ends with any operator other than '!'
            else if (tokens[i]->tokenType == Token::TokenType::Grouping && !((Grouping*)tokens[i])->isOpen &&
                     tokens[i - 1]->tokenType == Token::TokenType::Operator && ((Operator*)tokens[i - 1])->symbol != '!')
            {
                std::cout << "Error: Malformed Input - input has an operator other than '!' directly before a grouping: '" << ((Operator*)tokens[i - 1])->symbol << ((Grouping*)tokens[i])->symbol << "'\n";
                return -1;
            }
        }

        return 0;
    }

    // Might want to move this to Number:: or Operator::?
    inline void substituteConstantsAndSymbols(std::vector<Token*>& tokens)
    {
        for (int i = 0; i < tokens.size(); i++)
        {
            if (tokens[i]->tokenType != Token::TokenType::Function)
                continue;

            Function* currFunc{(Function*)tokens[i]};

            // Replace "mod" function with "%" operator, as it is formatted with a l and r side, just like operators
            if (currFunc->name == "mod")
            {
                delete tokens[i];
                tokens[i] = new Operator{'%'};
                continue;
            }

            // Replace functions that should be constants with their respective values
            long double val{Number::getConstant(currFunc->name)};
            if (!val)
                continue;
            delete tokens[i];
            tokens[i] = new Number{val};
        }
    }

    inline int checkForLaterMalformedInput(const std::vector<Token*>& tokens)
    {
        for (int i = 1; i < tokens.size(); i++)
        {
            // Function followed by a symbol other than '^'
            if (tokens[i - 1]->tokenType == Token::TokenType::Function && tokens[i]->tokenType == Token::TokenType::Operator)
            {
                Operator* currOp{(Operator*)tokens[i]};
                if (currOp->symbol != '^')
                {
                    std::cout << "Error: Malformed Input - input has an operator other than '^' directly after a function: '" << ((Function*)tokens[i - 1])->name << currOp->symbol << "'\n";
                    return -1;
                }
                // continue;
            }
        }
        return 0;
    }

    inline int sanitizeEquation(std::vector<Token*>& tokens)
    {
        // 1) parse function names and constants (if they aren't recognized, give the user an error and return)
        //   - this includes seeing if arc comes right before a trig function or h comes right after, should probably also handle ^-1 for trig functions and convert ^n or ^(n...) to some sort of function if it is being applied to a function
        //   - will also need to throw an error if the equation ends with a function, though I might eventually support functions with no parameters (along with more than one parameter), so handling functions on a case by case basis might eventually be necessary
        for (int i = 0; i < tokens.size(); i++)
        {
            if (tokens[i]->tokenType != Token::TokenType::Function)
                continue;
            if (Function::splitFunctions(tokens, i))
                return -1;
        }

        // Ensure functions were split properly
        Printer::printParsed(tokens, "Functions Split");


        // Should probably handle trig functions around here, like combining arc and ^-1 with the trig function

        substituteConstantsAndSymbols(tokens);

        // Ensure constants were substituted in properly
        Printer::printParsed(tokens, "Constants and Symbols Substituted In");

        // TODO: combine and apply +-'s
        // combineSigns(); (maybe Operator::)
        // applySigns();
        // OR:
        // handleSigns(); (collective of both)
        // TODO: handle ^# and arc, a, and h
        // handleFuncExponents(); (maybe Function::)
        // TODO: apply user constants
        // applyUserConstants(); (maybe Number::)
        // TODO: convert ! to fact()
        // applyFact(); (maybe Function:: or Operator::)


        // This check might be useless at this point due to me already handling ^'s after functions, unless I were to change this (also might want to move ! handling to after this)

        // HAVE to do this AFTER splitting functions and replacing functions with constants where relevant
        if (checkForLaterMalformedInput(tokens))
            return -1;



        // TODO: I need to be handling the syntax '[func]^[n]([m])' either before this function gets called, or alongside/inside of it!
        if (Grouping::addMissingFunctionGroupings(tokens))
            return -1;

        // Ensure function groupings were added properly
        Printer::printParsed(tokens, "Function Groupings Added");

        // Have to do this after inserting omitted function groupings, otherwise I would end up putting extra *'s where I don't intend them to be
        // My original reason for trying to do it before replacing functions with constants is because I was worried about consecutive numbers, but I actually check for those implicitly in the following function (it is easier to include it in the checks as a "by-product" of sorts)
        Operator::addMissingOperators(tokens);

        // Ensure *'s were added properly
        Printer::printParsed(tokens, "*'s Added");

        // Remove consectuive +'s and -'s
        // removeExtraOperators(tokens);


        // [DONE] 2) add groupings following functions if they aren't there
        // [DONE] 3) insert omitted *'s between numbers, groupings, and factorials ('[n!f](', ')[n!f]', ')(', 'ff', and '[n!f][n!f]' (except can't have '[n][n]'; also, if I decide to do the step about inserting groupings after functions before this, then I wouldn't need to include that in th checks, just groupings)
        // 4) convert consecutive +---+--... to one symbol
        // 5) if there is anything of the nature [*/][+-], make the [+-] apply to the following number, or if it is a function/grouping, makes some sort of sub() function in the event of a '-' and do nothing in the event of a '+'
        //     - Also want to handle "[(][+-]," as it is currently counted as an error
        // [DONE] 6) prepend and append missing groupings (not sure when the optimal time would be to do this; could also decouple when I find the amount to be added from when I actually do so so that they aren't in the way of the other "sanitizers")
        // 7) convert ! to fact()

        // I've been thinking; I might want each function to have its own member vector of Token*'s, though it might take around the same amount of code and complexity to do it either way (with or without functions having their own member vector)


        // Prepend and Append missing groupings
        Grouping::addMissingOuterGroupings(tokens);

        // Ensure outer Groupings were added properly
        Printer::printParsed(tokens, "Outer Groupings Added");

        return 0;
    }
}
